/*
 * Projeto de Monitoramento de Enchentes - ESP32
 * Desenvolvido para Iniciação Científica - Gabriel Passos de Oliveira
 * IGCE/UNESP - 2024
 * 
 * Sistema de monitoramento inteligente com análise de imagens e envio otimizado via MQTT
 */

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <math.h>

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
#include "esp_camera.h"
#include "mqtt_client.h"
#include "driver/gpio.h"

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

// Configurações da câmera ESP32-CAM (OV2640)
#define CAM_PIN_PWDN    32
#define CAM_PIN_RESET   -1
#define CAM_PIN_XCLK    0
#define CAM_PIN_SIOD    26
#define CAM_PIN_SIOC    27
#define CAM_PIN_D7      35
#define CAM_PIN_D6      34
#define CAM_PIN_D5      39
#define CAM_PIN_D4      36
#define CAM_PIN_D3      21
#define CAM_PIN_D2      19
#define CAM_PIN_D1      18
#define CAM_PIN_D0      5
#define CAM_PIN_VSYNC   25
#define CAM_PIN_HREF    23
#define CAM_PIN_PCLK    22

// Configurações do sistema
#define IMAGE_CAPTURE_INTERVAL  30000   // 30 segundos
#define NETWORK_MONITOR_INTERVAL 5000   // 5 segundos
#define CHANGE_THRESHOLD        0.15    // 15% de diferença para considerar mudança
#define MAX_IMAGE_SIZE          (50 * 1024) // 50KB máximo por imagem

static const char *TAG = "ENCHENTES_MONITOR";

// Event groups para sincronização
static EventGroupHandle_t wifi_event_group;
const int WIFI_CONNECTED_BIT = BIT0;

// Handles globais
static esp_mqtt_client_handle_t mqtt_client;
static camera_fb_t *last_frame = NULL;
static SemaphoreHandle_t camera_mutex;

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

// Função para calcular diferença entre imagens
static float calculate_image_difference(camera_fb_t *img1, camera_fb_t *img2) {
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
static size_t compress_image_data(camera_fb_t *fb, uint8_t *output, size_t max_output_size) {
    if (!fb || !output || fb->len > max_output_size) {
        return 0;
    }
    
    // Compressão simples - reduzir resolução/qualidade
    size_t compressed_size = fb->len / 2; // Simulação de compressão 50%
    if (compressed_size > max_output_size) {
        compressed_size = max_output_size;
    }
    
    // Copiar dados reduzindo amostragem
    for (size_t i = 0, j = 0; i < fb->len && j < compressed_size; i += 2, j++) {
        output[j] = fb->buf[i];
    }
    
    net_stats.taxa_compressao = (float)compressed_size / fb->len;
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
            ESP_LOGI(TAG, "MQTT Dados recebidos");
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

// Inicializar câmera
static esp_err_t camera_init(void) {
    camera_config_t config = {
        .pin_pwdn = CAM_PIN_PWDN,
        .pin_reset = CAM_PIN_RESET,
        .pin_xclk = CAM_PIN_XCLK,
        .pin_sscb_sda = CAM_PIN_SIOD,
        .pin_sscb_scl = CAM_PIN_SIOC,
        
        .pin_d7 = CAM_PIN_D7,
        .pin_d6 = CAM_PIN_D6,
        .pin_d5 = CAM_PIN_D5,
        .pin_d4 = CAM_PIN_D4,
        .pin_d3 = CAM_PIN_D3,
        .pin_d2 = CAM_PIN_D2,
        .pin_d1 = CAM_PIN_D1,
        .pin_d0 = CAM_PIN_D0,
        .pin_vsync = CAM_PIN_VSYNC,
        .pin_href = CAM_PIN_HREF,
        .pin_pclk = CAM_PIN_PCLK,
        
        .xclk_freq_hz = 20000000,
        .ledc_timer = LEDC_TIMER_0,
        .ledc_channel = LEDC_CHANNEL_0,
        
        .pixel_format = PIXFORMAT_JPEG,
        .frame_size = FRAMESIZE_SVGA, // 800x600
        .jpeg_quality = 12,
        .fb_count = 1,
        .fb_location = CAMERA_FB_IN_PSRAM,
        .grab_mode = CAMERA_GRAB_WHEN_EMPTY,
    };
    
    // Inicializar câmera
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Falha na inicialização da câmera: 0x%x", err);
        return err;
    }
    
    // Ajustar configurações do sensor
    sensor_t *s = esp_camera_sensor_get();
    if (s) {
        s->set_brightness(s, 0);     // -2 a 2
        s->set_contrast(s, 0);       // -2 a 2
        s->set_saturation(s, 0);     // -2 a 2
        s->set_special_effect(s, 0); // 0 a 6 (0=Nenhum, 1=Negativo, etc.)
        s->set_whitebal(s, 1);       // 0 = desabilitado, 1 = habilitado
        s->set_awb_gain(s, 1);       // 0 = desabilitado, 1 = habilitado
        s->set_wb_mode(s, 0);        // 0 a 4 - se awb_gain habilitado
        s->set_exposure_ctrl(s, 1);  // 0 = desabilitado, 1 = habilitado
        s->set_aec2(s, 0);           // 0 = desabilitado, 1 = habilitado
        s->set_ae_level(s, 0);       // -2 a 2
        s->set_aec_value(s, 300);    // 0 a 1200
        s->set_gain_ctrl(s, 1);      // 0 = desabilitado, 1 = habilitado
        s->set_agc_gain(s, 0);       // 0 a 30
        s->set_gainceiling(s, (gainceiling_t)0); // 0 a 6
        s->set_bpc(s, 0);            // 0 = desabilitado, 1 = habilitado
        s->set_wpc(s, 1);            // 0 = desabilitado, 1 = habilitado
        s->set_raw_gma(s, 1);        // 0 = desabilitado, 1 = habilitado
        s->set_lenc(s, 1);           // 0 = desabilitado, 1 = habilitado
        s->set_hmirror(s, 0);        // 0 = desabilitado, 1 = habilitado
        s->set_vflip(s, 0);          // 0 = desabilitado, 1 = habilitado
        s->set_dcw(s, 1);            // 0 = desabilitado, 1 = habilitado
        s->set_colorbar(s, 0);       // 0 = desabilitado, 1 = habilitado
    }
    
    ESP_LOGI(TAG, "Câmera inicializada com sucesso");
    return ESP_OK;
}

// Task para captura e análise de imagens
static void image_capture_task(void *pvParameters) {
    camera_fb_t *fb = NULL;
    uint8_t *compressed_data = malloc(MAX_IMAGE_SIZE);
    
    if (!compressed_data) {
        ESP_LOGE(TAG, "Falha ao alocar memória para compressão");
        vTaskDelete(NULL);
        return;
    }
    
    while (1) {
        // Aguardar conexão WiFi
        xEventGroupWaitBits(wifi_event_group, WIFI_CONNECTED_BIT, pdFALSE, pdFALSE, portMAX_DELAY);
        
        // Capturar imagem
        xSemaphoreTake(camera_mutex, portMAX_DELAY);
        fb = esp_camera_fb_get();
        xSemaphoreGive(camera_mutex);
        
        if (!fb) {
            ESP_LOGE(TAG, "Falha na captura da imagem");
            vTaskDelay(pdMS_TO_TICKS(5000));
            continue;
        }
        
        ESP_LOGI(TAG, "Imagem capturada: %zu bytes", fb->len);
        
        bool should_send = true;
        float difference = 0.0;
        
        // Comparar com última imagem se existir
        if (last_frame) {
            difference = calculate_image_difference(fb, last_frame);
            ESP_LOGI(TAG, "Diferença calculada: %.2f%%", difference * 100);
            
            if (difference < CHANGE_THRESHOLD) {
                should_send = false;
                net_stats.imagens_descartadas++;
                ESP_LOGI(TAG, "Imagem descartada - mudança insuficiente");
            }
        }
        
        if (should_send) {
            // Comprimir dados da imagem
            size_t compressed_size = compress_image_data(fb, compressed_data, MAX_IMAGE_SIZE);
            
            if (compressed_size > 0) {
                // Preparar dados do sensor para envio
                char sensor_data[256];
                snprintf(sensor_data, sizeof(sensor_data),
                    "{\"timestamp\":%lld,\"image_size\":%zu,\"compressed_size\":%zu,\"difference\":%.3f,\"location\":\"rio_monitoring\"}",
                    esp_timer_get_time() / 1000000LL, fb->len, compressed_size, difference);
                
                // Enviar dados do sensor
                int msg_id = esp_mqtt_client_publish(mqtt_client, TOPIC_SENSOR_DATA, 
                                                   sensor_data, 0, 1, 0);
                if (msg_id >= 0) {
                    ESP_LOGI(TAG, "Dados do sensor enviados, msg_id=%d", msg_id);
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
                ESP_LOGI(TAG, "Imagem enviada com sucesso - %zu bytes comprimidos", compressed_size);
                
                // Verificar se é uma mudança significativa (possível enchente)
                if (difference > 0.5) { // 50% de mudança - possível alerta
                    char alert_msg[200];
                    snprintf(alert_msg, sizeof(alert_msg),
                        "{\"alert\":\"significant_change\",\"difference\":%.3f,\"timestamp\":%lld}",
                        difference, esp_timer_get_time() / 1000000LL);
                    
                    esp_mqtt_client_publish(mqtt_client, TOPIC_ALERT, alert_msg, 0, 1, 0);
                    ESP_LOGW(TAG, "ALERTA: Mudança significativa detectada (%.1f%%)", difference * 100);
                }
            }
        }
        
        // Liberar frame anterior e armazenar atual
        if (last_frame) {
            esp_camera_fb_return(last_frame);
        }
        last_frame = fb;
        
        // Aguardar próxima captura
        vTaskDelay(pdMS_TO_TICKS(IMAGE_CAPTURE_INTERVAL));
    }
    
    free(compressed_data);
    vTaskDelete(NULL);
}

// Task para monitoramento de rede
static void network_monitor_task(void *pvParameters) {
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
            "\"uptime\":%lld"
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
            ESP_LOGI(TAG, "Estatísticas de rede enviadas");
            net_stats.bytes_enviados += strlen(stats_report);
        }
        
        ESP_LOGI(TAG, "Stats - Enviados: %lu bytes, Imagens: %lu/%lu, Compressão: %.1f%%", 
                net_stats.bytes_enviados, net_stats.imagens_enviadas, 
                net_stats.imagens_enviadas + net_stats.imagens_descartadas,
                net_stats.taxa_compressao * 100);
        
        vTaskDelay(pdMS_TO_TICKS(NETWORK_MONITOR_INTERVAL));
    }
    
    vTaskDelete(NULL);
}

void app_main(void) {
    ESP_LOGI(TAG, "=== Iniciando Sistema de Monitoramento de Enchentes ===");
    ESP_LOGI(TAG, "Projeto IC - Gabriel Passos de Oliveira - IGCE/UNESP");
    
    // Inicializar NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    
    // Criar mutex para proteção da câmera
    camera_mutex = xSemaphoreCreateMutex();
    if (!camera_mutex) {
        ESP_LOGE(TAG, "Falha ao criar mutex da câmera");
        return;
    }
    
    // Inicializar câmera
    if (camera_init() != ESP_OK) {
        ESP_LOGE(TAG, "Falha na inicialização da câmera");
        return;
    }
    
    // Inicializar WiFi
    wifi_init_sta();
    
    // Inicializar MQTT
    mqtt_init();
    
    // Aguardar um pouco para estabilizar conexões
    vTaskDelay(pdMS_TO_TICKS(2000));
    
    // Criar tasks
    xTaskCreate(image_capture_task, "image_capture", 8192, NULL, 5, NULL);
    xTaskCreate(network_monitor_task, "network_monitor", 4096, NULL, 3, NULL);
    
    ESP_LOGI(TAG, "Sistema inicializado com sucesso!");
    ESP_LOGI(TAG, "Captura de imagens a cada %d segundos", IMAGE_CAPTURE_INTERVAL / 1000);
    ESP_LOGI(TAG, "Threshold de mudança: %.1f%%", CHANGE_THRESHOLD * 100);
    
    // Task principal - monitoramento geral
    while (1) {
        ESP_LOGI(TAG, "Sistema funcionando - Memoria livre: %lu bytes", esp_get_free_heap_size());
        vTaskDelay(pdMS_TO_TICKS(60000)); // Log a cada minuto
    }
}
