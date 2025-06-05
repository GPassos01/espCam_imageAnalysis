/*
 * Sistema de Monitoramento de Nível d'Água - ESP32-CAM + HC-SR04
 * Projeto de Iniciação Científica - Gabriel Passos de Oliveira
 * IGCE/UNESP - 2025
 * 
 * Foco: Processamento de imagem embarcado para detecção de nível d'água
 * Sensoriamento complementar com HC-SR04
 * Comunicação MQTT otimizada com dados processados
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

// Módulos do projeto modularizado
#include "config.h"
#include "model/init_hw.h"
#include "model/init_net.h"
#include "model/sensor.h"
#include "model/image_processing.h"
#include "model/mqtt_send.h"

static const char *TAG = "WATER_MONITOR_IC";

// Estado global do sistema
typedef struct {
    float last_image_level;
    float last_sensor_level;
    float last_confidence;
    uint32_t readings_count;
    uint32_t alerts_sent;
    bool system_initialized;
} system_state_t;

static system_state_t g_state = {0};

// Configuração da câmera para IC
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
    .frame_size = FRAMESIZE_QVGA,        // 320x240 otimizado para processamento
    .jpeg_quality = JPEG_QUALITY,
    .fb_count = 2,
    .fb_location = CAMERA_FB_IN_PSRAM,
    .grab_mode = CAMERA_GRAB_LATEST
};

static camera_fb_t* capture_image_with_validation(void) {
    if (xSemaphoreTake(camera_mutex, pdMS_TO_TICKS(3000)) != pdTRUE) {
        ESP_LOGE(TAG, "Timeout ao obter mutex da câmera");
        return NULL;
    }
    
    // Flash LED para melhor qualidade
    gpio_set_level(CAM_PIN_FLASH, 1);
    vTaskDelay(pdMS_TO_TICKS(50));
    
    camera_fb_t *fb = esp_camera_fb_get();
    
    gpio_set_level(CAM_PIN_FLASH, 0);
    xSemaphoreGive(camera_mutex);
    
    if (!fb) {
        ESP_LOGE(TAG, "Falha na captura da câmera");
        return NULL;
    }
    
    // Validação básica
    if (fb->len < 2048 || fb->len > 102400) { // 2KB - 100KB
        ESP_LOGW(TAG, "Tamanho de imagem suspeito: %zu bytes", fb->len);
        esp_camera_fb_return(fb);
        return NULL;
    }
    
    return fb;
}

static void process_and_send_data(void) {
    ESP_LOGI(TAG, "🔄 Iniciando ciclo de leitura e processamento...");
    
    // 1. Capturar imagem
    camera_fb_t *fb = capture_image_with_validation();
    if (!fb) {
        ESP_LOGE(TAG, "❌ Falha na captura - pulando ciclo");
        return;
    }
    
    // 2. Processar imagem para detectar nível d'água
    water_analysis_result_t analysis = analyze_water_level_advanced(fb, g_state.last_image_level);
    
    // 3. Ler sensor HC-SR04
    float sensor_distance = hc_sr04_read_distance();
    float sensor_level = hc_sr04_calculate_water_level(sensor_distance, TANK_HEIGHT_CM);
    
    ESP_LOGI(TAG, "📊 Análise: IMG=%.1f%% (conf=%.2f) SENS=%.1f%% (%.1fcm)", 
             analysis.image_level, analysis.confidence, sensor_level, sensor_distance);
    
    // 4. Validar dados antes de enviar
    bool should_send_data = false;
    bool should_send_alert = false;
    bool should_send_image = false;
    
    if (analysis.is_valid && analysis.confidence >= CONFIDENCE_THRESHOLD) {
        // Verificar se houve mudança significativa
        float level_change = fabs(analysis.image_level - g_state.last_image_level);
        
        if (g_state.readings_count == 0 || level_change >= LEVEL_CHANGE_THRESHOLD) {
            should_send_data = true;
            
            // Verificar condições de alerta
            if (analysis.level_status == LEVEL_HIGH || analysis.level_status == LEVEL_LOW) {
                should_send_alert = true;
                if (SEND_IMAGE_ON_ALERT) {
                    should_send_image = true;
                }
            }
        }
        
        // Atualizar estado
        g_state.last_image_level = analysis.image_level;
        g_state.last_confidence = analysis.confidence;
    } else {
        ESP_LOGW(TAG, "⚠️ Análise de imagem com baixa confiança - usando apenas sensor");
        
        // Se análise de imagem falhar, usar apenas sensor como fallback
        if (sensor_level >= 0) {
            should_send_data = true;
            analysis.image_level = -1; // Indicar falha na análise de imagem
        }
    }
    
    // Atualizar sensor sempre que válido
    if (sensor_level >= 0) {
        g_state.last_sensor_level = sensor_level;
    }
    
    // 5. Enviar dados via MQTT
    if (should_send_data) {
        esp_err_t ret = mqtt_send_water_level_data(
            analysis.image_level, 
            g_state.last_sensor_level, 
            analysis.confidence, 
            DEVICE_ID
        );
        
        if (ret == ESP_OK) {
            g_state.readings_count++;
            ESP_LOGI(TAG, "✅ Dados enviados com sucesso (leitura #%lu)", g_state.readings_count);
        } else {
            ESP_LOGE(TAG, "❌ Falha no envio de dados");
        }
    }
    
    // 6. Enviar alertas se necessário
    if (should_send_alert) {
        const char* alert_type = (analysis.level_status == LEVEL_HIGH) ? 
                                "high_water_level" : "low_water_level";
        
        esp_err_t ret = mqtt_send_alert(analysis.image_level, alert_type, DEVICE_ID);
        if (ret == ESP_OK) {
            g_state.alerts_sent++;
            ESP_LOGW(TAG, "🚨 Alerta enviado: %s (alerta #%lu)", alert_type, g_state.alerts_sent);
        }
    }
    
    // 7. Enviar imagem como fallback se necessário
    if (should_send_image || (!analysis.is_valid && should_send_data)) {
        const char* reason = should_send_image ? "alert_triggered" : "analysis_failed";
        mqtt_send_image_fallback(fb, reason, DEVICE_ID);
    }
    
    // Liberar buffer da câmera
    esp_camera_fb_return(fb);
    
    ESP_LOGI(TAG, "✅ Ciclo completo - próximo em %d segundos", CAPTURE_INTERVAL_MS / 1000);
}

static void send_system_status(void) {
    uint32_t uptime = esp_timer_get_time() / 1000000LL;
    size_t free_heap = esp_get_free_heap_size();
    size_t free_psram = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
    
    esp_err_t ret = mqtt_send_system_status(uptime, free_heap, free_psram, DEVICE_ID);
    
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "📊 Status do sistema: Uptime=%lus, Heap=%zuKB, PSRAM=%zuKB", 
                 uptime, free_heap/1024, free_psram/1024);
    }
}

// Task principal unificada - foco da IC
static void water_monitoring_task(void *pvParameters) {
    ESP_LOGI(TAG, "🚀 Iniciando monitoramento de nível d'água - IC");
    
    TickType_t last_capture = 0;
    TickType_t last_status = 0;
    
    while (1) {
        TickType_t now = xTaskGetTickCount();
        
        // Verificar se é hora de fazer leitura e processamento
        if ((now - last_capture) >= pdMS_TO_TICKS(CAPTURE_INTERVAL_MS)) {
            process_and_send_data();
            last_capture = now;
        }
        
        // Verificar se é hora de enviar status do sistema
        if ((now - last_status) >= pdMS_TO_TICKS(STATUS_INTERVAL_MS)) {
            send_system_status();
            last_status = now;
        }
        
        // Sleep por 1 segundo para não sobrecarregar CPU
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
    
    vTaskDelete(NULL);
}

void app_main(void) {
    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "Sistema de Monitoramento de Nível d'Água");
    ESP_LOGI(TAG, "Projeto IC - Gabriel Passos - IGCE/UNESP");
    ESP_LOGI(TAG, "Processamento Embarcado + HC-SR04");
    ESP_LOGI(TAG, "========================================");
    
    // Inicialização do sistema
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    
    // Inicializar rede
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();
    
    // Inicializar hardware usando módulos
    ESP_LOGI(TAG, "📷 Inicializando hardware...");
    
    create_camera_mutex();
    
    if (init_camera(&camera_config) != ESP_OK) {
        ESP_LOGE(TAG, "❌ Falha na inicialização da câmera");
        return;
    }
    ESP_LOGI(TAG, "✅ Câmera ESP32-CAM inicializada");
    
    if (hc_sr04_init(HC_SR04_TRIG_PIN, HC_SR04_ECHO_PIN) != ESP_OK) {
        ESP_LOGE(TAG, "❌ Falha na inicialização do HC-SR04");
        return;
    }
    ESP_LOGI(TAG, "✅ Sensor HC-SR04 inicializado");
    
    // Inicializar rede usando módulos
    ESP_LOGI(TAG, "🌐 Conectando à rede...");
    wifi_init_sta(WIFI_SSID, WIFI_PASS);
    ESP_LOGI(TAG, "✅ WiFi conectado");
    
    mqtt_init(MQTT_BROKER_URI, MQTT_USERNAME, MQTT_PASSWORD);
    ESP_LOGI(TAG, "✅ MQTT conectado");
    
    // Aguardar estabilização
    vTaskDelay(pdMS_TO_TICKS(3000));
    
    // Marcar sistema como inicializado
    g_state.system_initialized = true;
    
    ESP_LOGI(TAG, "🎯 Configurações da IC:");
    ESP_LOGI(TAG, "   📏 Altura do tanque: %.1f cm", TANK_HEIGHT_CM);
    ESP_LOGI(TAG, "   ⏱️  Intervalo de captura: %d segundos", CAPTURE_INTERVAL_MS / 1000);
    ESP_LOGI(TAG, "   🎚️  Threshold de confiança: %.1f", CONFIDENCE_THRESHOLD);
    ESP_LOGI(TAG, "   📊 Threshold de mudança: %.1f%%", LEVEL_CHANGE_THRESHOLD);
    ESP_LOGI(TAG, "   📷 Resolução: %dx%d JPEG Q%d", IMAGE_WIDTH, IMAGE_HEIGHT, JPEG_QUALITY);
    
    // Iniciar task principal unificada
    xTaskCreate(water_monitoring_task, "water_monitoring", 16384, NULL, 5, NULL);
    
    ESP_LOGI(TAG, "🚀 Sistema de monitoramento iniciado com sucesso!");
    ESP_LOGI(TAG, "🔬 Projeto de IC focado em processamento embarcado");
}
