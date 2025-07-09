/**
 * Sistema de Monitoramento ESP32-CAM - Versão Simples (Sem Comparação)
 * 
 * Funcionalidades básicas:
 * - Captura fotos a cada 15 segundos (HVGA 480x320)
 * - Envia TODAS as imagens via MQTT
 * - Dados de monitoramento básico
 * - Sistema anti-esverdeado inteligente
 * - WiFi sniffer para monitoramento de tráfego
 * 
 * @version 2.0 - Versão de comparação para testes
 * @author Gabriel Passos - UNESP 2025
 */

#include <stdio.h>
#include <string.h>
#include <time.h>
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
#include "model/mqtt_send.h"
#include "model/init_net.h"
#include "model/init_hw.h"
#include "model/wifi_sniffer.h"
#include "model/chip_info.h"
#include "config.h"

static const char *TAG = "IMG_MONITOR_SIMPLE";

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
    .frame_size = FRAMESIZE_HVGA,        // 480x320 (otimizado para qualidade)
    .jpeg_quality = JPEG_QUALITY,        // Qualidade premium
    .fb_count = 2,                       // Double buffering
    .fb_location = CAMERA_FB_IN_PSRAM,
    .grab_mode = CAMERA_GRAB_WHEN_EMPTY
};

// Variáveis globais
static uint32_t total_bytes_sent = 0;
static uint32_t total_photos_sent = 0;
static uint32_t capture_count = 0;

// Handler de eventos MQTT
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
    esp_err_t err = mqtt_send_image_with_info(fb, topic, reason, 0.0f);
    
    if (err == ESP_OK) {
        total_bytes_sent += fb->len;
        total_photos_sent++;
        ESP_LOGI(TAG, "📸 Imagem enviada: %zu bytes - %s", fb->len, reason);
    } else {
        ESP_LOGE(TAG, "❌ Falha ao enviar imagem: %s", esp_err_to_name(err));
    }
    
    if (SNIFFER_ENABLED && wifi_sniffer_is_active()) {
        wifi_sniffer_mark_image_end();
    }
}

// Capturar e enviar foto (versão simples)
static void capture_and_send_photo(void)
{
    ESP_LOGI(TAG, "📸 Capturando foto...");
    
    // Aplicar configurações adaptativas por horário periodicamente
    if (capture_count % 10 == 0) {
        apply_time_based_settings();
    }
    
    // Warm-up periódico para evitar tint verde
    if (capture_count == 0 || (capture_count % 10 == 0)) {
        ESP_LOGI(TAG, "🔥 Realizando warm-up periódico...");
        camera_warmup_capture();
    }
    
    // Usar captura inteligente com correção automática
    camera_fb_t *fb;
    if (smart_capture_with_correction(&fb) != ESP_OK) {
        ESP_LOGE(TAG, "❌ Falha na captura inteligente da câmera");
        return;
    }
    
    capture_count++;
    ESP_LOGI(TAG, "📷 Foto capturada: %zu bytes (%zux%zu)", 
             fb->len, fb->width, fb->height);

    // Determinar motivo do envio
    const char* reason;
    if (capture_count == 1) {
        reason = "first_capture";
    } else if (capture_count % 4 == 0) {
        reason = "periodic_sample";
    } else {
        reason = "periodic";
    }
    
    // SEMPRE enviar imagem (sem comparação)
    send_image_via_mqtt(fb, reason);
    
    // Enviar dados básicos de monitoramento
    mqtt_send_monitoring_data(0.0f, fb->len, fb->width, fb->height, fb->format, DEVICE_ID);
    
    // Enviar status do sistema
    mqtt_send_monitoring(esp_get_free_heap_size(), 
                        heap_caps_get_free_size(MALLOC_CAP_SPIRAM), 
                        esp_timer_get_time() / 1000000);
    
    // Liberar frame
    esp_camera_fb_return(fb);
}

// Imprimir estatísticas
static void print_statistics(void)
{
    ESP_LOGI(TAG, "\n\n📈 === ESTATÍSTICAS DE MONITORAMENTO SIMPLES ===");
    ESP_LOGI(TAG, "📷 Fotos: %lu enviadas / %lu processadas (100%% taxa de envio)", total_photos_sent, capture_count);
    ESP_LOGI(TAG, "📡 Dados: %.2f KB transmitidos", (float)total_bytes_sent / 1024.0);
    
    if (total_photos_sent > 0) {
        ESP_LOGI(TAG, "📊 Média: %lu bytes/foto", total_bytes_sent / total_photos_sent);
    }
    
    size_t free_heap = esp_get_free_heap_size();
    size_t free_psram = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
    
    ESP_LOGI(TAG, "💾 Heap: %zu KB livre", free_heap / 1024);
    ESP_LOGI(TAG, "💾 PSRAM: %zu KB livre", free_psram / 1024);
    ESP_LOGI(TAG, "🔄 Modo: ENVIO TOTAL (sem comparação de imagens)");
    
    if (SNIFFER_ENABLED && wifi_sniffer_is_active()) {
        wifi_sniffer_print_stats();
    }
    
    ESP_LOGI(TAG, "=======================================\n\n");
}

// Task principal de monitoramento
static void monitoring_task(void *pvParameter)
{
    ESP_LOGI(TAG, "\n\n🚀 Iniciando task de monitoramento SIMPLES");
    
    TickType_t last_capture = 0;
    TickType_t last_stats = 0;
    TickType_t last_sniffer_stats = 0;
    
    while (1) {
        TickType_t now = xTaskGetTickCount();
        
        // Captura a cada 15 segundos
        if ((now - last_capture) >= pdMS_TO_TICKS(CAPTURE_INTERVAL_MS)) {
            if (mqtt_is_connected()) {
                capture_and_send_photo();
            } else {
                ESP_LOGW(TAG, "MQTT desconectado, pulando captura");
            }
            last_capture = now;
        }
        
        // Estatísticas a cada 5 minutos
        if ((now - last_stats) >= pdMS_TO_TICKS(300000)) {
            print_statistics();
            last_stats = now;
        }
        
        // Estatísticas do sniffer a cada 1 minuto
        if (SNIFFER_ENABLED && wifi_sniffer_is_active() && 
            (now - last_sniffer_stats) >= pdMS_TO_TICKS(SNIFFER_STATS_INTERVAL * 1000)) {
            wifi_sniffer_print_stats();
            if (mqtt_is_connected()) {
                wifi_sniffer_send_mqtt_stats(mqtt_client, DEVICE_ID);
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
    ESP_LOGI(TAG, "🔍 Sistema ESP32-CAM - VERSÃO SIMPLES");
    ESP_LOGI(TAG, "📊 Envio Total (sem comparação)");
    ESP_LOGI(TAG, "Gabriel Passos - UNESP 2025");
    ESP_LOGI(TAG, "========================================");
    
    // Informações do hardware
    print_chip_info();
    
    // Inicialização
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
  
    // Aguardar conexões
    ESP_LOGI(TAG, "Aguardando WiFi...");
    if (wifi_wait_connected(30000) != ESP_OK) {
        ESP_LOGE(TAG, "Falha WiFi. Reiniciando...");
        esp_restart();
    }
    ESP_LOGI(TAG, "✅ WiFi conectado!");
  
    ESP_LOGI(TAG, "Aguardando MQTT...");
    if (mqtt_wait_connected(30000) != ESP_OK) {
        ESP_LOGE(TAG, "Falha MQTT. Reiniciando...");
        esp_restart();
    }
    ESP_LOGI(TAG, "✅ MQTT conectado!");
    
    // Configurações finais
    ESP_LOGI(TAG, "🔍 Configuração SIMPLES:");
    ESP_LOGI(TAG, "   - Resolução: HVGA 480x320 (qualidade premium)");
    ESP_LOGI(TAG, "   - JPEG Quality: %d", JPEG_QUALITY);
    ESP_LOGI(TAG, "   - Modo: ENVIO TOTAL (100%% das imagens)");
    ESP_LOGI(TAG, "   - Intervalo: %d segundos", CAPTURE_INTERVAL_MS / 1000);
    ESP_LOGI(TAG, "   - Comparação: DESABILITADA");

    // Inicializar WiFi sniffer
    if (SNIFFER_ENABLED) {
        ESP_LOGI(TAG, "📡 Inicializando WiFi Sniffer...");
        if (wifi_sniffer_init(SNIFFER_CHANNEL) == ESP_OK && wifi_sniffer_start() == ESP_OK) {
            ESP_LOGI(TAG, "✅ WiFi Sniffer ativo no canal %d", SNIFFER_CHANNEL);
        } else {
            ESP_LOGW(TAG, "⚠️  WiFi Sniffer desabilitado");
        }
    }

    // Iniciar monitoramento
    xTaskCreate(monitoring_task, "monitoring_task_simple", 8192, NULL, 5, NULL);
    ESP_LOGI(TAG, "✅ Sistema SIMPLES iniciado!");
} 