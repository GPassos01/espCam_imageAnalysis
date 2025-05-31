/*
 * Projeto de Monitoramento de Enchentes - ESP32 (Versão Teste de Rede)
 * Desenvolvido para Iniciação Científica - Gabriel Passos de Oliveira
 * IGCE/UNESP - 2024
 * 
 * Sistema de monitoramento inteligente com simulação de análise de imagens 
 * para teste de rede e MQTT (SEM CÂMERA)
 */

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <stdlib.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"
#include "lwip/sys.h"

#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "nvs_flash.h"
#include "esp_timer.h"
#include "mqtt_client.h"
#include "driver/gpio.h"
#include "esp_random.h"

// Configurações de rede
#define WIFI_SSID        "Steps 2.4G"
#define WIFI_PASS        "h%8Ka4D&"
#define MQTT_BROKER_URI  "mqtt://192.168.1.2:1883"
#define MQTT_USERNAME    ""
#define MQTT_PASSWORD    ""

// Tópicos MQTT
#define TOPIC_IMAGE_DATA    "enchentes/imagem/dados"
#define TOPIC_SENSOR_DATA   "enchentes/sensores"
#define TOPIC_NETWORK_STATS "enchentes/rede/estatisticas"
#define TOPIC_ALERT         "enchentes/alertas"

// Configurações do sistema
#define IMAGE_CAPTURE_INTERVAL  30000   // 30 segundos
#define NETWORK_MONITOR_INTERVAL 5000   // 5 segundos
#define CHANGE_THRESHOLD        0.15    // 15% de diferença para considerar mudança
#define MAX_IMAGE_SIZE          (50 * 1024) // 50KB máximo por imagem

static const char *TAG = "ENCHENTES_MONITOR_TESTE";

// Event groups para sincronização
static EventGroupHandle_t wifi_event_group;
const int WIFI_CONNECTED_BIT = BIT0;

// Handles globais
static esp_mqtt_client_handle_t mqtt_client;

// Estrutura para simular dados de imagem
typedef struct {
    uint8_t *buf;
    size_t len;
    uint32_t timestamp;
} simulated_frame_t;

static simulated_frame_t *last_frame = NULL;

// Estatísticas de rede
typedef struct {
    uint32_t bytes_enviados;
    uint32_t bytes_recebidos;
    uint32_t pacotes_enviados;
    uint32_t pacotes_recebidos;
    uint32_t imagens_enviadas;
    uint32_t imagens_descartadas;
    float taxa_compressao;
} network_stats_t;

static network_stats_t net_stats = {0};

// Função para gerar dados simulados de imagem
static simulated_frame_t* generate_simulated_image(void) {
    simulated_frame_t *frame = malloc(sizeof(simulated_frame_t));
    if (!frame) {
        return NULL;
    }
    
    // Simular tamanho de imagem variável (10KB a 50KB)
    size_t base_size = 10 * 1024 + (esp_random() % (40 * 1024));
    frame->len = base_size;
    frame->buf = malloc(frame->len);
    frame->timestamp = esp_timer_get_time() / 1000000LL;
    
    if (!frame->buf) {
        free(frame);
        return NULL;
    }
    
    // Preencher com dados simulados
    // Simular padrões que mudam com o tempo para testar detecção de mudanças
    uint32_t pattern_seed = frame->timestamp / 60; // Muda a cada minuto
    
    for (size_t i = 0; i < frame->len; i++) {
        // Criar padrão que varia baseado no tempo e posição
        uint8_t pattern_value = (pattern_seed + i + (esp_random() % 50)) % 256;
        frame->buf[i] = pattern_value;
    }
    
    ESP_LOGI(TAG, "Imagem simulada gerada: %zu bytes, timestamp: %lu", frame->len, frame->timestamp);
    return frame;
}

// Função para calcular diferença entre imagens simuladas
static float calculate_image_difference(simulated_frame_t *img1, simulated_frame_t *img2) {
    if (!img1 || !img2 || img1->len != img2->len) {
        return 1.0f; // Máxima diferença se imagens inválidas
    }
    
    uint32_t diff_pixels = 0;
    uint32_t total_pixels = img1->len;
    
    // Comparação simples pixel a pixel
    for (uint32_t i = 0; i < total_pixels; i += 4) { // Amostragem para otimização
        if (abs(img1->buf[i] - img2->buf[i]) > 30) { // Threshold de diferença
            diff_pixels++;
        }
    }
    
    return (float)diff_pixels / (total_pixels / 4);
}

// Função para comprimir dados da imagem (simples)
static size_t compress_image_data(simulated_frame_t *frame, uint8_t *output, size_t max_output_size) {
    if (!frame || !output || frame->len > max_output_size) {
        return 0;
    }
    
    // Compressão simples - reduzir resolução/qualidade
    size_t compressed_size = frame->len / 2; // Simulação de compressão 50%
    if (compressed_size > max_output_size) {
        compressed_size = max_output_size;
    }
    
    // Copiar dados reduzindo amostragem
    for (size_t i = 0, j = 0; i < frame->len && j < compressed_size; i += 2, j++) {
        output[j] = frame->buf[i];
    }
    
    net_stats.taxa_compressao = (float)compressed_size / frame->len;
    return compressed_size;
}

// Event handler para WiFi
static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                              int32_t event_id, void* event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        esp_wifi_connect();
        xEventGroupClearBits(wifi_event_group, WIFI_CONNECTED_BIT);
        ESP_LOGI(TAG, "Tentando reconectar ao WiFi...");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "IP obtido:" IPSTR, IP2STR(&event->ip_info.ip));
        xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

// Event handler para MQTT
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;
    
    switch ((esp_mqtt_event_id_t)event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT Conectado");
            // Subscrever a tópicos de controle se necessário
            esp_mqtt_client_subscribe(client, "enchentes/controle/+", 0);
            break;
            
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "MQTT Desconectado");
            break;
            
        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGI(TAG, "MQTT Subscrito, msg_id=%d", event->msg_id);
            break;
            
        case MQTT_EVENT_PUBLISHED:
            ESP_LOGI(TAG, "MQTT Publicado, msg_id=%d", event->msg_id);
            net_stats.pacotes_enviados++;
            break;
            
        case MQTT_EVENT_DATA:
            ESP_LOGI(TAG, "MQTT Dados recebidos: %.*s", event->data_len, event->data);
            // Processar comandos recebidos se necessário
            break;
            
        case MQTT_EVENT_ERROR:
            ESP_LOGI(TAG, "MQTT Erro");
            break;
            
        default:
            break;
    }
}

// Inicializar WiFi
static void wifi_init_sta(void) {
    wifi_event_group = xEventGroupCreate();
    
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();
    
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    
    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        &instance_got_ip));
    
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
        },
    };
    
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    
    ESP_LOGI(TAG, "Inicialização WiFi finalizada.");
    
    // Aguardar conexão
    EventBits_t bits = xEventGroupWaitBits(wifi_event_group,
                                          WIFI_CONNECTED_BIT,
                                          pdFALSE,
                                          pdFALSE,
                                          portMAX_DELAY);
    
    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "Conectado ao WiFi SSID:%s", WIFI_SSID);
    }
}

// Inicializar MQTT
static void mqtt_init(void) {
    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = MQTT_BROKER_URI,
        .credentials.username = MQTT_USERNAME,
        .credentials.authentication.password = MQTT_PASSWORD,
    };
    
    mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(mqtt_client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(mqtt_client);
}

// Task para simulação de captura e análise de imagens
static void image_simulation_task(void *pvParameters) {
    uint8_t *compressed_data = malloc(MAX_IMAGE_SIZE);
    
    if (!compressed_data) {
        ESP_LOGE(TAG, "Falha ao alocar memória para compressão");
        vTaskDelete(NULL);
        return;
    }
    
    ESP_LOGI(TAG, "🔄 Iniciando simulação de captura de imagens...");
    
    while (1) {
        // Aguardar conexão WiFi
        xEventGroupWaitBits(wifi_event_group, WIFI_CONNECTED_BIT, pdFALSE, pdFALSE, portMAX_DELAY);
        
        // Gerar imagem simulada
        simulated_frame_t *current_frame = generate_simulated_image();
        
        if (!current_frame) {
            ESP_LOGE(TAG, "Falha na geração da imagem simulada");
            vTaskDelay(pdMS_TO_TICKS(5000));
            continue;
        }
        
        ESP_LOGI(TAG, "📸 Imagem simulada capturada: %zu bytes", current_frame->len);
        
        bool should_send = true;
        float difference = 0.0;
        
        // Comparar com última imagem se existir
        if (last_frame) {
            difference = calculate_image_difference(current_frame, last_frame);
            ESP_LOGI(TAG, "📊 Diferença calculada: %.2f%%", difference * 100);
            
            if (difference < CHANGE_THRESHOLD) {
                should_send = false;
                net_stats.imagens_descartadas++;
                ESP_LOGI(TAG, "🚫 Imagem descartada - mudança insuficiente (%.1f%% < %.1f%%)", 
                        difference * 100, CHANGE_THRESHOLD * 100);
            }
        }
        
        if (should_send) {
            // Comprimir dados da imagem
            size_t compressed_size = compress_image_data(current_frame, compressed_data, MAX_IMAGE_SIZE);
            
            if (compressed_size > 0) {
                // Preparar dados do sensor para envio
                char sensor_data[256];
                snprintf(sensor_data, sizeof(sensor_data),
                    "{\"timestamp\":%lu,\"image_size\":%zu,\"compressed_size\":%zu,\"difference\":%.3f,\"location\":\"rio_monitoring_simulacao\",\"modo\":\"teste_sem_camera\"}",
                    current_frame->timestamp, current_frame->len, compressed_size, difference);
                
                // Enviar dados do sensor
                int msg_id = esp_mqtt_client_publish(mqtt_client, TOPIC_SENSOR_DATA, 
                                                   sensor_data, 0, 1, 0);
                if (msg_id >= 0) {
                    ESP_LOGI(TAG, "📤 Dados do sensor enviados, msg_id=%d", msg_id);
                    net_stats.bytes_enviados += strlen(sensor_data);
                }
                
                // Enviar imagem comprimida (em chunks se necessário)
                const size_t chunk_size = 1024; // 1KB por chunk
                for (size_t offset = 0; offset < compressed_size; offset += chunk_size) {
                    size_t current_chunk = (offset + chunk_size > compressed_size) ? 
                                          (compressed_size - offset) : chunk_size;
                    
                    char topic[128];
                    snprintf(topic, sizeof(topic), "%s/%zu/%zu", TOPIC_IMAGE_DATA, offset, compressed_size);
                    
                    msg_id = esp_mqtt_client_publish(mqtt_client, topic,
                                                   (char*)(compressed_data + offset), current_chunk, 1, 0);
                    if (msg_id >= 0) {
                        net_stats.bytes_enviados += current_chunk;
                    }
                    
                    vTaskDelay(pdMS_TO_TICKS(100)); // Pequeno delay entre chunks
                }
                
                net_stats.imagens_enviadas++;
                ESP_LOGI(TAG, "✅ Imagem simulada enviada com sucesso - %zu bytes comprimidos (taxa: %.1f%%)", 
                        compressed_size, net_stats.taxa_compressao * 100);
                
                // Verificar se é uma mudança significativa (possível enchente)
                if (difference > 0.5) { // 50% de mudança - possível alerta
                    char alert_msg[200];
                    snprintf(alert_msg, sizeof(alert_msg),
                        "{\"alert\":\"significant_change\",\"difference\":%.3f,\"timestamp\":%lu,\"modo\":\"simulacao\"}",
                        difference, current_frame->timestamp);
                    
                    esp_mqtt_client_publish(mqtt_client, TOPIC_ALERT, alert_msg, 0, 1, 0);
                    ESP_LOGW(TAG, "🚨 ALERTA: Mudança significativa detectada na simulação (%.1f%%)", difference * 100);
                }
            }
        }
        
        // Liberar frame anterior e armazenar atual
        if (last_frame) {
            if (last_frame->buf) free(last_frame->buf);
            free(last_frame);
        }
        last_frame = current_frame;
        
        // Aguardar próxima captura
        vTaskDelay(pdMS_TO_TICKS(IMAGE_CAPTURE_INTERVAL));
    }
    
    free(compressed_data);
    vTaskDelete(NULL);
}

// Task para monitoramento de rede
static void network_monitor_task(void *pvParameters) {
    ESP_LOGI(TAG, "📡 Iniciando monitoramento de rede...");
    
    while (1) {
        // Aguardar conexão WiFi
        xEventGroupWaitBits(wifi_event_group, WIFI_CONNECTED_BIT, pdFALSE, pdFALSE, portMAX_DELAY);
        
        // Obter estatísticas de WiFi
        wifi_sta_list_t wifi_sta_list;
        esp_wifi_ap_get_sta_list(&wifi_sta_list);
        
        // Preparar relatório de estatísticas
        char stats_report[512];
        snprintf(stats_report, sizeof(stats_report),
            "{"
            "\"timestamp\":%lld,"
            "\"bytes_enviados\":%lu,"
            "\"bytes_recebidos\":%lu,"
            "\"pacotes_enviados\":%lu,"
            "\"pacotes_recebidos\":%lu,"
            "\"imagens_enviadas\":%lu,"
            "\"imagens_descartadas\":%lu,"
            "\"taxa_compressao\":%.3f,"
            "\"memoria_livre\":%lu,"
            "\"uptime\":%lld,"
            "\"modo\":\"teste_sem_camera\""
            "}",
            esp_timer_get_time() / 1000000LL,
            net_stats.bytes_enviados,
            net_stats.bytes_recebidos,
            net_stats.pacotes_enviados,
            net_stats.pacotes_recebidos,
            net_stats.imagens_enviadas,
            net_stats.imagens_descartadas,
            net_stats.taxa_compressao,
            esp_get_free_heap_size(),
            esp_timer_get_time() / 1000000LL
        );
        
        // Enviar estatísticas
        int msg_id = esp_mqtt_client_publish(mqtt_client, TOPIC_NETWORK_STATS, 
                                           stats_report, 0, 1, 0);
        if (msg_id >= 0) {
            ESP_LOGI(TAG, "📊 Estatísticas de rede enviadas");
            net_stats.bytes_enviados += strlen(stats_report);
        }
        
        // Calcular e exibir eficiência
        uint32_t total_imgs = net_stats.imagens_enviadas + net_stats.imagens_descartadas;
        float eficiencia = total_imgs > 0 ? (float)net_stats.imagens_descartadas / total_imgs * 100 : 0;
        
        ESP_LOGI(TAG, "📈 Stats - Enviados: %lu bytes, Imagens: %lu/%lu, Eficiência: %.1f%%, Compressão: %.1f%%", 
                net_stats.bytes_enviados, net_stats.imagens_enviadas, total_imgs,
                eficiencia, net_stats.taxa_compressao * 100);
        
        vTaskDelay(pdMS_TO_TICKS(NETWORK_MONITOR_INTERVAL));
    }
    
    vTaskDelete(NULL);
}

void app_main(void) {
    ESP_LOGI(TAG, "=== Iniciando Sistema de Monitoramento de Enchentes (TESTE SEM CÂMERA) ===");
    ESP_LOGI(TAG, "Projeto IC - Gabriel Passos de Oliveira - IGCE/UNESP");
    ESP_LOGI(TAG, "Modo: Simulação para teste de rede e MQTT");
    
    // Inicializar NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    
    // Inicializar gerador de números aleatórios
    srand(time(NULL));
    
    // Inicializar WiFi
    wifi_init_sta();
    
    // Inicializar MQTT
    mqtt_init();
    
    // Aguardar um pouco para estabilizar conexões
    vTaskDelay(pdMS_TO_TICKS(2000));
    
    // Criar tasks
    xTaskCreate(image_simulation_task, "image_simulation", 8192, NULL, 5, NULL);
    xTaskCreate(network_monitor_task, "network_monitor", 4096, NULL, 3, NULL);
    
    ESP_LOGI(TAG, "✅ Sistema inicializado com sucesso!");
    ESP_LOGI(TAG, "📸 Simulação de captura de imagens a cada %d segundos", IMAGE_CAPTURE_INTERVAL / 1000);
    ESP_LOGI(TAG, "🔍 Threshold de mudança: %.1f%%", CHANGE_THRESHOLD * 100);
    ESP_LOGI(TAG, "🌐 Monitoramento de rede a cada %d segundos", NETWORK_MONITOR_INTERVAL / 1000);
    
    // Task principal - monitoramento geral
    while (1) {
        uint32_t total_imgs = net_stats.imagens_enviadas + net_stats.imagens_descartadas;
        float eficiencia = total_imgs > 0 ? (float)net_stats.imagens_descartadas / total_imgs * 100 : 0;
        
        ESP_LOGI(TAG, "🔧 Sistema funcionando - Memória livre: %lu bytes | Eficiência: %.1f%%", 
                esp_get_free_heap_size(), eficiencia);
        vTaskDelay(pdMS_TO_TICKS(60000)); // Log a cada minuto
    }
}
