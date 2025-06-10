#include "wifi_sniffer.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "esp_netif.h"
#include <string.h>
#include <stdio.h>

static const char *TAG = "WIFI_SNIFFER";

// Estruturas para análise de pacotes WiFi
typedef struct {
    uint8_t type:2;
    uint8_t subtype:4;
    uint8_t version:2;
    uint8_t order:1;
    uint8_t wep:1;
    uint8_t moredata:1;
    uint8_t powermgt:1;
    uint8_t retry:1;
    uint8_t morefrag:1;
    uint8_t fromds:1;
    uint8_t tods:1;
} __attribute__((packed)) wifi_ieee80211_frame_ctrl_t;

typedef struct {
    wifi_ieee80211_frame_ctrl_t frame_ctrl;
    uint16_t duration_id;
    uint8_t addr1[6];
    uint8_t addr2[6];
    uint8_t addr3[6];
    uint16_t sequence_ctrl;
    uint8_t addr4[6];
} __attribute__((packed)) wifi_ieee80211_mac_hdr_t;

// Variáveis globais
static wifi_traffic_stats_t g_traffic_stats = {0};
static packet_callback_t g_user_callback = NULL;
static uint8_t g_monitor_channel = 1;
static bool g_image_transmission_active = false;
static uint32_t g_image_start_time = 0;
static wifi_traffic_stats_t g_image_stats = {0};

// MAC address do próprio ESP32 (para filtrar pacotes próprios)
static uint8_t g_esp32_mac[6] = {0};

/**
 * Callback principal para captura de pacotes WiFi
 */
static void wifi_sniffer_packet_handler(void *recv_buf, wifi_promiscuous_pkt_type_t type)
{
    if (type != WIFI_PKT_DATA) {
        return; // Só processar pacotes de dados
    }
    
    wifi_promiscuous_pkt_t *pkt = (wifi_promiscuous_pkt_t *)recv_buf;
    const uint8_t *payload = pkt->payload;
    uint32_t len = pkt->rx_ctrl.sig_len;
    
    if (len < sizeof(wifi_ieee80211_mac_hdr_t)) {
        return; // Pacote muito pequeno
    }
    
    wifi_ieee80211_mac_hdr_t *hdr = (wifi_ieee80211_mac_hdr_t *)payload;
    
    // Verificar se é um pacote do nosso ESP32 (origem ou destino)
    bool is_our_packet = false;
    if (memcmp(hdr->addr2, g_esp32_mac, 6) == 0 || // Transmitido por nós
        memcmp(hdr->addr1, g_esp32_mac, 6) == 0) {  // Destinado a nós
        is_our_packet = true;
    }
    
    // Atualizar estatísticas gerais
    g_traffic_stats.total_packets++;
    g_traffic_stats.total_bytes += len;
    
    // Log de debug a cada 100 pacotes
    if (g_traffic_stats.total_packets % 100 == 0) {
        ESP_LOGI(TAG, "📊 Capturados %lu pacotes (%llu bytes total)", 
                 g_traffic_stats.total_packets, g_traffic_stats.total_bytes);
    }
    
    if (is_our_packet) {
        // Assumir que todo tráfego do nosso ESP32 é relevante para MQTT
        // (já que só usamos MQTT para comunicação)
        bool is_mqtt = false;
        
        // Heurísticas melhoradas para detecção MQTT
        if (len > 30) { // Pacotes TCP/IP mínimos
            const uint8_t *data = payload + sizeof(wifi_ieee80211_mac_hdr_t);
            uint32_t data_len = len - sizeof(wifi_ieee80211_mac_hdr_t);
            
            // Verificar padrões de porta MQTT (1883) e dados conhecidos
            if (data_len > 20) {
                // Buscar por porta 1883 (MQTT padrão) nos headers
                for (int i = 0; i < data_len - 4; i++) {
                    // Porta 1883 em bytes (0x075B)
                    if ((data[i] == 0x07 && data[i+1] == 0x5B) ||
                        (data[i] == 0x5B && data[i+1] == 0x07)) {
                        is_mqtt = true;
                        break;
                    }
                }
                
                // Se não encontrou porta, assumir MQTT se for pacote TCP do nosso ESP
                // (heurística: pacotes > 100 bytes provavelmente são dados)
                if (!is_mqtt && len > 100) {
                    is_mqtt = true;
                }
                
                // Buscar por strings conhecidas (fallback)
                if (!is_mqtt) {
                    for (int i = 0; i < data_len - 10; i++) {
                        if (memcmp(&data[i], "monitor", 7) == 0 ||
                            memcmp(&data[i], "MQTT", 4) == 0 ||
                            memcmp(&data[i], "alert", 5) == 0) {
                            is_mqtt = true;
                            break;
                        }
                    }
                }
            }
        }
        
        if (is_mqtt) {
            g_traffic_stats.mqtt_packets++;
            g_traffic_stats.mqtt_bytes += len;
            
            // Se estamos rastreando transmissão de imagem
            if (g_image_transmission_active) {
                g_image_stats.image_packets++;
                g_image_stats.image_bytes += len;
            }
            
            // Log de debug para verificar captura
            ESP_LOGD(TAG, "📡 Pacote MQTT capturado: %lu bytes", len);
        }
    }
    
    // Chamar callback do usuário se definido
    if (g_user_callback) {
        g_user_callback(payload, len, pkt);
    }
}

esp_err_t wifi_sniffer_init(uint8_t channel)
{
    ESP_LOGI(TAG, "Inicializando WiFi sniffer...");
    
    // Tentar detectar canal atual automaticamente
    uint8_t current_channel = 0;
    wifi_second_chan_t second = WIFI_SECOND_CHAN_NONE;
    esp_err_t ret = esp_wifi_get_channel(&current_channel, &second);
    
    if (ret == ESP_OK && current_channel > 0 && current_channel <= 13) {
        g_monitor_channel = current_channel;
        ESP_LOGI(TAG, "🔍 Canal detectado automaticamente: %d", current_channel);
    } else {
        if (channel < 1 || channel > 13) {
            ESP_LOGE(TAG, "Canal inválido: %d", channel);
            return ESP_ERR_INVALID_ARG;
        }
        g_monitor_channel = channel;
        ESP_LOGI(TAG, "📡 Usando canal configurado: %d", channel);
    }
    
    // Obter MAC address do ESP32
    ret = esp_wifi_get_mac(WIFI_IF_STA, g_esp32_mac);
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "Não foi possível obter MAC address: %s", esp_err_to_name(ret));
        memset(g_esp32_mac, 0, 6);
    } else {
        ESP_LOGI(TAG, "MAC ESP32: %02x:%02x:%02x:%02x:%02x:%02x",
                 g_esp32_mac[0], g_esp32_mac[1], g_esp32_mac[2],
                 g_esp32_mac[3], g_esp32_mac[4], g_esp32_mac[5]);
    }
    
    // Resetar estatísticas
    wifi_sniffer_reset_stats();
    
    ESP_LOGI(TAG, "WiFi sniffer inicializado com sucesso");
    return ESP_OK;
}

esp_err_t wifi_sniffer_start(void)
{
    ESP_LOGI(TAG, "Iniciando captura de pacotes WiFi...");
    
    esp_err_t ret;
    
    // Configurar modo promíscuo
    ret = esp_wifi_set_promiscuous(true);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Falha ao ativar modo promíscuo: %s", esp_err_to_name(ret));
        return ret;
    }
    
    // Definir callback para captura
    ret = esp_wifi_set_promiscuous_rx_cb(wifi_sniffer_packet_handler);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Falha ao definir callback: %s", esp_err_to_name(ret));
        esp_wifi_set_promiscuous(false);
        return ret;
    }
    
    // Verificar canal atual antes de tentar mudar
    uint8_t current_channel = 0;
    wifi_second_chan_t second = WIFI_SECOND_CHAN_NONE;
    esp_err_t channel_ret = esp_wifi_get_channel(&current_channel, &second);
    
    if (channel_ret == ESP_OK && current_channel == g_monitor_channel) {
        ESP_LOGI(TAG, "✅ Já estamos no canal correto: %d", current_channel);
    } else {
        // Tentar configurar canal de monitoramento
        ret = esp_wifi_set_channel(g_monitor_channel, WIFI_SECOND_CHAN_NONE);
        if (ret != ESP_OK) {
            ESP_LOGW(TAG, "⚠️  Não foi possível definir canal %d: %s", 
                     g_monitor_channel, esp_err_to_name(ret));
            ESP_LOGI(TAG, "📡 Continuando com canal atual (%d)", current_channel);
            if (current_channel > 0) {
                g_monitor_channel = current_channel;
            }
        } else {
            ESP_LOGI(TAG, "✅ Canal definido para: %d", g_monitor_channel);
        }
    }
    
    // Configurar filtro para capturar apenas pacotes de dados
    wifi_promiscuous_filter_t filter = {
        .filter_mask = WIFI_PROMIS_FILTER_MASK_DATA
    };
    ret = esp_wifi_set_promiscuous_filter(&filter);
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "Aviso: não foi possível definir filtro: %s", esp_err_to_name(ret));
    }
    
    g_traffic_stats.active = true;
    g_traffic_stats.start_time = esp_timer_get_time() / 1000000LL;
    
    ESP_LOGI(TAG, "✅ Captura de pacotes iniciada no canal %d", g_monitor_channel);
    return ESP_OK;
}

esp_err_t wifi_sniffer_stop(void)
{
    ESP_LOGI(TAG, "Parando captura de pacotes WiFi...");
    
    esp_err_t ret = esp_wifi_set_promiscuous(false);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Falha ao desativar modo promíscuo: %s", esp_err_to_name(ret));
        return ret;
    }
    
    g_traffic_stats.active = false;
    
    ESP_LOGI(TAG, "✅ Captura de pacotes parada");
    return ESP_OK;
}

esp_err_t wifi_sniffer_deinit(void)
{
    ESP_LOGI(TAG, "Deinicializando WiFi sniffer...");
    
    if (g_traffic_stats.active) {
        wifi_sniffer_stop();
    }
    
    g_user_callback = NULL;
    wifi_sniffer_reset_stats();
    
    ESP_LOGI(TAG, "WiFi sniffer deinicializado");
    return ESP_OK;
}

void wifi_sniffer_mark_image_start(void)
{
    g_image_transmission_active = true;
    g_image_start_time = esp_timer_get_time() / 1000000LL;
    
    // Resetar estatísticas específicas da imagem
    g_image_stats.image_packets = 0;
    g_image_stats.image_bytes = 0;
    g_image_stats.start_time = g_image_start_time;
    
    ESP_LOGD(TAG, "📸 Início de transmissão de imagem marcado");
}

void wifi_sniffer_mark_image_end(void)
{
    if (g_image_transmission_active) {
        g_image_transmission_active = false;
        uint32_t duration = (esp_timer_get_time() / 1000000LL) - g_image_start_time;
        
        ESP_LOGI(TAG, "📈 Transmissão de imagem concluída:");
        ESP_LOGI(TAG, "   - Duração: %lu segundos", duration);
        ESP_LOGI(TAG, "   - Pacotes: %lu", g_image_stats.image_packets);
        ESP_LOGI(TAG, "   - Bytes: %llu (%.2f KB)", 
                 g_image_stats.image_bytes, 
                 g_image_stats.image_bytes / 1024.0);
        
        if (duration > 0) {
            double throughput = (double)g_image_stats.image_bytes / duration;
            ESP_LOGI(TAG, "   - Throughput: %.2f bytes/s (%.2f KB/s)", 
                     throughput, throughput / 1024.0);
        }
    }
}

void wifi_sniffer_get_stats(wifi_traffic_stats_t *stats)
{
    if (stats) {
        memcpy(stats, &g_traffic_stats, sizeof(wifi_traffic_stats_t));
    }
}

void wifi_sniffer_reset_stats(void)
{
    memset(&g_traffic_stats, 0, sizeof(wifi_traffic_stats_t));
    memset(&g_image_stats, 0, sizeof(wifi_traffic_stats_t));
    g_image_transmission_active = false;
    g_image_start_time = 0;
    
    ESP_LOGD(TAG, "Estatísticas resetadas");
}

void wifi_sniffer_print_stats(void)
{
    uint32_t uptime = 0;
    if (g_traffic_stats.start_time > 0) {
        uptime = (esp_timer_get_time() / 1000000LL) - g_traffic_stats.start_time;
    }
    
    ESP_LOGI(TAG, "📊 === ESTATÍSTICAS DE TRÁFEGO WiFi ===");
    ESP_LOGI(TAG, "⏱️  Tempo ativo: %lu segundos", uptime);
    ESP_LOGI(TAG, "📦 Total de pacotes: %lu", g_traffic_stats.total_packets);
    ESP_LOGI(TAG, "📊 Total de bytes: %llu (%.2f KB)", 
             g_traffic_stats.total_bytes,
             g_traffic_stats.total_bytes / 1024.0);
    ESP_LOGI(TAG, "📡 Pacotes MQTT: %lu", g_traffic_stats.mqtt_packets);
    ESP_LOGI(TAG, "📡 Bytes MQTT: %llu (%.2f KB)", 
             g_traffic_stats.mqtt_bytes,
             g_traffic_stats.mqtt_bytes / 1024.0);
    
    if (g_traffic_stats.total_packets > 0) {
        double mqtt_ratio = (double)g_traffic_stats.mqtt_packets / g_traffic_stats.total_packets * 100;
        ESP_LOGI(TAG, "📈 MQTT/Total: %.1f%% dos pacotes", mqtt_ratio);
    }
    
    if (uptime > 0 && g_traffic_stats.mqtt_bytes > 0) {
        double throughput = (double)g_traffic_stats.mqtt_bytes / uptime;
        ESP_LOGI(TAG, "🚀 Throughput MQTT: %.2f bytes/s (%.2f KB/s)", 
                 throughput, throughput / 1024.0);
    }
    
    ESP_LOGI(TAG, "📍 Canal: %d | Status: %s", 
             g_monitor_channel,
             g_traffic_stats.active ? "ATIVO" : "INATIVO");
    ESP_LOGI(TAG, "=====================================");
}

void wifi_sniffer_set_callback(packet_callback_t callback)
{
    g_user_callback = callback;
    ESP_LOGD(TAG, "Callback personalizado definido");
}

bool wifi_sniffer_is_active(void)
{
    return g_traffic_stats.active;
}

esp_err_t wifi_sniffer_send_mqtt_stats(esp_mqtt_client_handle_t mqtt_client, const char* device_id)
{
    if (!mqtt_client || !g_traffic_stats.active) {
        return ESP_ERR_INVALID_STATE;
    }
    
    uint32_t current_time = esp_timer_get_time() / 1000000LL;
    uint32_t uptime = current_time - g_traffic_stats.start_time;
    
    char payload[400];
    int ret = snprintf(payload, sizeof(payload),
        "{"
        "\"timestamp\":%lu,"
        "\"device\":\"%s\","
        "\"total_packets\":%lu,"
        "\"mqtt_packets\":%lu,"
        "\"total_bytes\":%llu,"
        "\"mqtt_bytes\":%llu,"
        "\"image_packets\":%lu,"
        "\"image_bytes\":%llu,"
        "\"uptime\":%lu,"
        "\"channel\":%d,"
        "\"active\":%s"
        "}",
        current_time, device_id,
        g_traffic_stats.total_packets,
        g_traffic_stats.mqtt_packets,
        g_traffic_stats.total_bytes,
        g_traffic_stats.mqtt_bytes,
        g_image_stats.image_packets,
        g_image_stats.image_bytes,
        uptime,
        g_monitor_channel,
        g_traffic_stats.active ? "true" : "false");
    
    if (ret < 0 || ret >= sizeof(payload)) {
        ESP_LOGE(TAG, "Erro ao formatar payload de estatísticas");
        return ESP_ERR_INVALID_SIZE;
    }
    
    int msg_id = esp_mqtt_client_publish(mqtt_client, "monitoring/sniffer/stats",
                                       payload, 0, 1, 0);
    
    if (msg_id < 0) {
        ESP_LOGE(TAG, "Falha ao enviar estatísticas do sniffer via MQTT");
        return ESP_FAIL;
    }
    
    ESP_LOGD(TAG, "📡 Estatísticas do sniffer enviadas via MQTT");
    return ESP_OK;
} 