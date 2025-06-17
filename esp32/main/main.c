/**
 * Sistema de Monitoramento ESP32-CAM
 * 
 * Funcionalidades principais:
 * - Captura fotos a cada 15 segundos (QVGA 320x240)
 * - Compara imagens usando algoritmo simplificado
 * - Detecta mudanças (10%) e alertas (30%)
 * - Envia dados via MQTT com reconexão automática
 * - Gerencia memória PSRAM para imagens grandes
 * - Monitoramento de sistema em tempo real
 * 
 * @author Gabriel Passos - UNESP 2025
 */

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "esp_timer.h"
#include "esp_wifi.h"
#include "esp_camera.h"
#include "mqtt_client.h"
#include "esp_heap_caps.h"

// Módulos do projeto
#include "model/compare.h"
#include "model/mqtt_send.h"
#include "model/init_net.h"
#include "model/wifi_sniffer.h"
#include "config.h"

static const char *TAG = "IMG_MONITOR";

// Configuração da câmera
static camera_config_t camera_config = {
    .pin_pwdn = CAM_PIN_PWDN,
    .pin_reset = CAM_PIN_RESET,
    .pin_xclk = CAM_PIN_XCLK,
    .pin_sccb_sda = CAM_PIN_SIOD,
    .pin_sccb_scl = CAM_PIN_SIOC,
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
    .frame_size = FRAMESIZE_QVGA,        // 320x240
    .jpeg_quality = JPEG_QUALITY,        // Usar definição do config.h
    .fb_count = 2,                       // Double buffering
    .fb_location = CAMERA_FB_IN_PSRAM,
    .grab_mode = CAMERA_GRAB_WHEN_EMPTY
};

// Variáveis globais
static uint32_t total_bytes_sent = 0;
static uint32_t total_photos_sent = 0;
static camera_fb_t *previous_frame = NULL;

// Handler de eventos MQTT - Apenas para logs
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, 
                              int32_t event_id, void *event_data)
{
    esp_mqtt_event_handle_t event = event_data;
    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT Event: Conectado");
            break;
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "MQTT Event: Desconectado");
            break;
        default:
            break;
    }
}

// Inicializar câmera
static esp_err_t init_camera(void)
{
    esp_err_t err = esp_camera_init(&camera_config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Falha na inicialização da câmera: %s", esp_err_to_name(err));
    } else {
        ESP_LOGI(TAG, "Câmera inicializada com sucesso");
    }
    return err;
}

// Enviar dados via MQTT
static void send_monitoring_data(float difference, camera_fb_t *fb, bool is_alert)
{
    char json[256];
    snprintf(json, sizeof(json),
             "{\"timestamp\":%lu,\"image_size\":%zu,\"difference\":%.3f,"
             "\"width\":%zu,\"height\":%zu,\"format\":%d,"
             "\"location\":\"monitoring_esp32cam\",\"mode\":\"image_comparison\"}",
             (uint32_t)(esp_timer_get_time() / 1000000LL),
             fb->len, difference, fb->width, fb->height, fb->format);
    
    esp_mqtt_client_publish(mqtt_client, "monitoring/data", json, 0, 1, 0);
    
    if (is_alert) {
        char alert[200];
        snprintf(alert, sizeof(alert),
                 "{\"alert\":\"significant_change\",\"difference\":%.3f,"
                 "\"timestamp\":%lu,\"image_size\":%zu,"
                 "\"location\":\"monitoring_esp32cam\",\"mode\":\"image_comparison\"}",
                 difference, (uint32_t)(esp_timer_get_time() / 1000000LL), fb->len);
        
        esp_mqtt_client_publish(mqtt_client, "monitoring/alert", alert, 0, 1, 0);
        ESP_LOGW(TAG, "\n\n🚨 ALERTA: Mudança significativa detectada (%.1f%%)\n\n", difference);
    }
}

// Enviar imagem via MQTT
static void send_image_via_mqtt(camera_fb_t *fb, const char* reason)
{
    if (!fb) {
        ESP_LOGW(TAG, "Frame inválido, não é possível enviar imagem");
        return;
    }
    
    if (SNIFFER_ENABLED && wifi_sniffer_is_active()) {
        wifi_sniffer_mark_image_start();
    }
    
    char topic[128];
    snprintf(topic, sizeof(topic), "%s/%s", MQTT_TOPIC_BASE, MQTT_TOPIC_IMAGE);
    esp_err_t err = mqtt_send_image(fb, topic);
    
    if (err == ESP_OK) {
        total_bytes_sent += fb->len;
        total_photos_sent++;
        ESP_LOGI(TAG, "📸 Imagem enviada com sucesso - Motivo: %s", reason);
    } else {
        ESP_LOGE(TAG, "❌ Falha ao enviar imagem: %s", esp_err_to_name(err));
    }
    
    if (SNIFFER_ENABLED && wifi_sniffer_is_active()) {
        wifi_sniffer_mark_image_end();
    }
}

// Capturar e analisar foto
static void capture_and_analyze_photo(void)
{
    ESP_LOGI(TAG, "📸 Capturando foto...");
    
    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb) {
        ESP_LOGE(TAG, "Falha na captura da câmera");
        return;
    }

    ESP_LOGI(TAG, "📷 Foto capturada: %zu bytes (%zux%zu)", 
             fb->len, fb->width, fb->height);

    float difference = 0.0f;
    bool is_alert = false;
    bool send_image = false;
    const char* reason = "periodic";

    if (previous_frame) {
        difference = calculate_image_difference(fb, previous_frame);
        ESP_LOGI(TAG, "🔍 Diferença calculada: %.1f%%", difference);
        
        if (difference >= CHANGE_THRESHOLD) {
            ESP_LOGI(TAG, "📊 Mudança detectada: %.1f%%", difference);
            send_image = true;
            reason = "change_detected";
            
            if (difference >= ALERT_THRESHOLD) {
                is_alert = true;
                reason = "significant_change";
                ESP_LOGW(TAG, "\n\n🚨 ALERTA: Mudança significativa detectada (%.1f%%)\n\n", difference);
            }
        }
    } else {
        ESP_LOGI(TAG, "📷 Primeira captura - enviando imagem de referência");
        send_image = true;
        reason = "first_capture";
    }
    
    mqtt_send_monitoring(esp_get_free_heap_size(), heap_caps_get_free_size(MALLOC_CAP_SPIRAM), esp_timer_get_time() / 1000000);
    
    if (is_alert) {
        mqtt_send_alert(difference, fb);
    }
    
    if (send_image) {
        send_image_via_mqtt(fb, reason);
    }
    
    if (previous_frame) {
        esp_camera_fb_return(previous_frame);
    }
    
    previous_frame = fb;
}

// Imprimir estatísticas
static void print_statistics(void)
{
    ESP_LOGI(TAG, "📈 === ESTATÍSTICAS DE MONITORAMENTO ===");
    ESP_LOGI(TAG, "📷 Fotos processadas: %lu", total_photos_sent);
    ESP_LOGI(TAG, "📡 Bytes transmitidos: %lu (%.2f KB)", 
             total_bytes_sent, (float)total_bytes_sent / 1024.0);
    if (total_photos_sent > 0) {
        ESP_LOGI(TAG, "📊 Média por foto: %lu bytes", 
                 total_bytes_sent / total_photos_sent);
    }
    
    size_t free_heap = esp_get_free_heap_size();
    size_t free_psram = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
    size_t min_free_heap = esp_get_minimum_free_heap_size();
    
    ESP_LOGI(TAG, "💾 Heap livre: %zu KB (mínimo: %zu KB)", 
             free_heap / 1024, min_free_heap / 1024);
    ESP_LOGI(TAG, "💾 PSRAM livre: %zu KB", free_psram / 1024);
    
    if (free_heap < 50000) {
        ESP_LOGW(TAG, "⚠️  MEMÓRIA HEAP BAIXA!");
    }
    if (free_psram < 1000000) {
        ESP_LOGW(TAG, "⚠️  MEMÓRIA PSRAM BAIXA!");
    }
    
    ESP_LOGI(TAG, "=======================================");
    
    if (SNIFFER_ENABLED && wifi_sniffer_is_active()) {
        wifi_sniffer_print_stats();
    }
}

// Task principal de monitoramento
static void monitoring_task(void *pvParameter)
{
    ESP_LOGI(TAG, "🚀 Iniciando task de monitoramento");
    
    TickType_t last_capture = 0;
    TickType_t last_stats = 0;
    TickType_t last_sniffer_stats = 0;
    
    while (1) {
        TickType_t now = xTaskGetTickCount();
        
        if ((now - last_capture) >= pdMS_TO_TICKS(CAPTURE_INTERVAL_MS)) {
            if (mqtt_is_connected()) {
                capture_and_analyze_photo();
            } else {
                ESP_LOGW(TAG, "MQTT desconectado, pulando captura.");
            }
            last_capture = now;
        }
        
        if ((now - last_stats) >= pdMS_TO_TICKS(300000)) {
            print_statistics();
            last_stats = now;
        }
        
        if (SNIFFER_ENABLED && wifi_sniffer_is_active() && (now - last_sniffer_stats) >= pdMS_TO_TICKS(SNIFFER_STATS_INTERVAL * 1000)) {
            wifi_sniffer_print_stats();
            if (mqtt_is_connected()) {
                wifi_traffic_stats_t stats;
                wifi_sniffer_get_stats(&stats);
                
                if (mqtt_send_sniffer_stats(stats.total_packets, stats.total_bytes, stats.mqtt_packets, stats.mqtt_bytes) != ESP_OK) {
                    ESP_LOGW(TAG, "Falha ao enviar stats do sniffer via MQTT");
                }
            }
            last_sniffer_stats = now;
        }
        
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

// Função principal
void app_main(void)
{
    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "🔍 Sistema de Monitoramento por Imagens");
    ESP_LOGI(TAG, "📊 Detecção de Mudanças Visuais");
    ESP_LOGI(TAG, "Gabriel Passos - UNESP 2025");
    ESP_LOGI(TAG, "========================================");
    
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    
    ESP_LOGI(TAG, "📷 Inicializando câmera...");
    ESP_ERROR_CHECK(init_camera());

    ESP_LOGI(TAG, "🌐 Conectando WiFi...");
    esp_netif_create_default_wifi_sta();
    wifi_init_sta();
    
    ESP_LOGI(TAG, "📡 Conectando MQTT...");
    mqtt_init();
    
    ESP_ERROR_CHECK(esp_mqtt_client_register_event(mqtt_client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL));
  
    ESP_LOGI(TAG, "Aguardando conexão WiFi...");
    esp_err_t wifi_status = wifi_wait_connected(30000);
  
    if (wifi_status != ESP_OK) {
        ESP_LOGE(TAG, "Falha ao conectar ao WiFi. Reiniciando...");
        esp_restart();
    }
    ESP_LOGI(TAG, "WiFi conectado!");
  
    ESP_LOGI(TAG, "Aguardando conexão MQTT...");
    esp_err_t mqtt_status = mqtt_wait_connected(30000);
  
    if (mqtt_status != ESP_OK) {
        ESP_LOGE(TAG, "Falha ao conectar ao MQTT. Reiniciando...");
        esp_restart();
    }
    ESP_LOGI(TAG, "MQTT conectado!");
    
    ESP_LOGI(TAG, "🔍 Configuração de detecção:");
    ESP_LOGI(TAG, "   - Mudança mínima: %.1f%%", CHANGE_THRESHOLD);
    ESP_LOGI(TAG, "   - Alerta em: %.1f%%", ALERT_THRESHOLD);
    ESP_LOGI(TAG, "   - Intervalo: %d segundos", CAPTURE_INTERVAL_MS / 1000);

    if (SNIFFER_ENABLED) {
        ESP_LOGI(TAG, "📡 Inicializando WiFi Sniffer...");
        esp_err_t sniffer_ret = wifi_sniffer_init(SNIFFER_CHANNEL);
        if (sniffer_ret == ESP_OK) {
            sniffer_ret = wifi_sniffer_start();
            if (sniffer_ret == ESP_OK) {
                ESP_LOGI(TAG, "✅ WiFi Sniffer ativo no canal %d", SNIFFER_CHANNEL);
                ESP_LOGI(TAG, "📊 Monitoramento de tráfego habilitado");
            } else {
                ESP_LOGW(TAG, "⚠️ Falha ao iniciar WiFi Sniffer: %s", esp_err_to_name(sniffer_ret));
            }
        } else {
            ESP_LOGW(TAG, "⚠️ Falha ao inicializar WiFi Sniffer: %s", esp_err_to_name(sniffer_ret));
        }
    } else {
        ESP_LOGI(TAG, "📡 WiFi Sniffer desabilitado");
    }

    xTaskCreate(monitoring_task, "monitoring_task", 8192, NULL, 5, NULL);
    
    ESP_LOGI(TAG, "✅ Sistema de monitoramento iniciado!");
    
}
