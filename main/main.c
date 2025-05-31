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

// Configurações do sistema - MODO TESTE OTIMIZADO
#define IMAGE_CAPTURE_INTERVAL  15000   // 15 segundos (otimizado)
#define NETWORK_MONITOR_INTERVAL 3000   // 3 segundos (mais frequente) 
#define CHANGE_THRESHOLD        0.12    // 12% de diferença (mais sensível)
#define MAX_IMAGE_SIZE          (80 * 1024) // 80KB máximo (aumentado)
#define MIN_IMAGE_SIZE          (15 * 1024) // 15KB mínimo (novo)

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

// Função para gerar dados simulados de imagem - VERSÃO MELHORADA
static simulated_frame_t* generate_simulated_image(void) {
    simulated_frame_t *frame = malloc(sizeof(simulated_frame_t));
    if (!frame) {
        return NULL;
    }
    
    // Simular tamanho de imagem variável baseado em condições "ambientais"
    uint32_t current_time = esp_timer_get_time() / 1000000LL;
    uint32_t time_of_day = current_time % 86400; // Segundos no dia
    
    // Simular variações por "período do dia" 
    float size_factor = 1.0;
    if (time_of_day < 21600 || time_of_day > 75600) { // "Noite" - imagens menores
        size_factor = 0.7;
    } else if (time_of_day > 43200 && time_of_day < 54000) { // "Tarde" - imagens maiores
        size_factor = 1.3;
    }
    
    size_t base_size = MIN_IMAGE_SIZE + (esp_random() % (MAX_IMAGE_SIZE - MIN_IMAGE_SIZE));
    frame->len = (size_t)(base_size * size_factor);
    
    // Limitar ao máximo permitido
    if (frame->len > MAX_IMAGE_SIZE) frame->len = MAX_IMAGE_SIZE;
    if (frame->len < MIN_IMAGE_SIZE) frame->len = MIN_IMAGE_SIZE;
    
    frame->buf = malloc(frame->len);
    frame->timestamp = current_time;
    
    if (!frame->buf) {
        free(frame);
        return NULL;
    }
    
    // Preencher com dados mais realistas
    uint32_t pattern_seed = frame->timestamp / 120; // Muda a cada 2 minutos
    uint8_t noise_level = (esp_random() % 30) + 10; // Ruído de 10-40
    
    for (size_t i = 0; i < frame->len; i++) {
        // Criar padrão mais complexo baseado em posição e tempo
        uint8_t base_pattern = (pattern_seed + i/100) % 256;
        uint8_t noise = (esp_random() % noise_level);
        uint8_t temporal_variation = (frame->timestamp/60 + i/200) % 50;
        
        frame->buf[i] = (base_pattern + noise + temporal_variation) % 256;
    }
    
    ESP_LOGI(TAG, "🎯 Imagem simulada: %zu bytes, fator: %.1f, padrão: %lu", 
             frame->len, size_factor, pattern_seed);
    return frame;
}

// Função para calcular diferença entre imagens simuladas - VERSÃO OTIMIZADA
static float calculate_image_difference(simulated_frame_t *img1, simulated_frame_t *img2) {
    if (!img1 || !img2) {
        return 1.0f; // Máxima diferença se uma imagem for inválida
    }
    
    // Se tamanhos são muito diferentes, considerar mudança significativa
    float size_ratio = (float)img1->len / img2->len;
    if (size_ratio < 0.7 || size_ratio > 1.4) {
        ESP_LOGI(TAG, "📏 Diferença de tamanho significativa: %.2f", size_ratio);
        return 0.8f; // Alta diferença por mudança de tamanho
    }
    
    // Usar o menor tamanho para comparação
    uint32_t compare_size = (img1->len < img2->len) ? img1->len : img2->len;
    uint32_t diff_pixels = 0;
    uint32_t significant_diffs = 0;
    
    // Algoritmo otimizado - comparação por blocos
    uint32_t block_size = 16;
    uint32_t threshold = 25; // Threshold para diferença pixel
    
    for (uint32_t i = 0; i < compare_size; i += block_size) {
        uint32_t block_end = (i + block_size > compare_size) ? compare_size : i + block_size;
        uint32_t block_diffs = 0;
        
        for (uint32_t j = i; j < block_end; j++) {
            int pixel_diff = abs(img1->buf[j] - img2->buf[j]);
            if (pixel_diff > threshold) {
                block_diffs++;
                if (pixel_diff > threshold * 2) {
                    significant_diffs++;
                }
            }
        }
        
        // Se mais de 50% do bloco mudou, contar como bloco diferente
        if (block_diffs > (block_end - i) / 2) {
            diff_pixels += block_diffs;
        }
    }
    
    float base_difference = (float)diff_pixels / (compare_size / block_size);
    float significant_factor = (float)significant_diffs / compare_size;
    
    // Combinar diferenças base e significativas
    float final_difference = (base_difference * 0.7) + (significant_factor * 0.3);
    
    ESP_LOGI(TAG, "🔍 Análise: %.1f%% base, %.1f%% significativa → %.1f%% final", 
             base_difference * 100, significant_factor * 100, final_difference * 100);
    
    return final_difference;
}

// Função para comprimir dados da imagem - VERSÃO MELHORADA
static size_t compress_image_data(simulated_frame_t *frame, uint8_t *output, size_t max_output_size) {
    if (!frame || !output || frame->len > max_output_size * 4) {
        return 0;
    }
    
    // Simular compressão inteligente baseada no conteúdo
    size_t input_size = frame->len;
    
    // Analisar "complexidade" da imagem (variação nos dados)
    uint32_t complexity = 0;
    for (size_t i = 1; i < input_size && i < 1000; i++) {
        complexity += abs(frame->buf[i] - frame->buf[i-1]);
    }
    complexity = complexity / ((input_size < 1000) ? input_size : 1000);
    
    // Taxa de compressão baseada na complexidade
    float compression_ratio;
    if (complexity < 20) {
        compression_ratio = 0.3f; // Imagem "simples" - comprime bem
    } else if (complexity < 50) {
        compression_ratio = 0.5f; // Média complexidade
    } else if (complexity < 80) {
        compression_ratio = 0.7f; // Alta complexidade
    } else {
        compression_ratio = 0.85f; // Muito complexa - comprime pouco
    }
    
    // Adicionar variação aleatória à compressão
    compression_ratio += (esp_random() % 20 - 10) / 100.0f; // ±10%
    
    // Limitar dentro de valores realistas
    if (compression_ratio < 0.25f) compression_ratio = 0.25f;
    if (compression_ratio > 0.9f) compression_ratio = 0.9f;
    
    size_t compressed_size = (size_t)(input_size * compression_ratio);
    if (compressed_size > max_output_size) {
        compressed_size = max_output_size;
        compression_ratio = (float)compressed_size / input_size;
    }
    
    // Simular algoritmo de compressão por blocos
    size_t output_idx = 0;
    for (size_t i = 0; i < input_size && output_idx < compressed_size; i += 2) {
        if (i + 1 < input_size) {
            // Simular compressão por média de pixels adjacentes
            output[output_idx] = (frame->buf[i] + frame->buf[i+1]) / 2;
        } else {
            output[output_idx] = frame->buf[i];
        }
        output_idx++;
    }
    
    net_stats.taxa_compressao = compression_ratio;
    
    ESP_LOGI(TAG, "📦 Compressão: %zu→%zu bytes (%.1f%%), complexidade: %lu", 
             input_size, compressed_size, compression_ratio * 100, complexity);
    
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
