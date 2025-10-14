/**
 * Sistema de Monitoramento ESP32-CAM - Versão Inteligente
 * 
 * Funcionalidades avançadas:
 * - Captura fotos a cada 15 segundos (HVGA 480x320)
 * - Análise de comparação RGB565 por blocos 32x32
 * - Envia APENAS imagens com mudanças significativas (>8%)
 * - Sistema de referência estática para estabilidade
 * - Economia de dados ~90% vs versão simples
 * - WiFi sniffer para monitoramento de tráfego
 * 
 * @version 2.0 - Versão inteligente principal
 * @author Gabriel Passos - UNESP 2025
 */

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <inttypes.h>
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
#include "model/compare.h"
#include "model/ei_inference.h"
#include "model/test_dataset.h"
#include "config.h"

static const char *TAG = "IMG_MONITOR_INTELLIGENT";

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

// Variáveis globais para detecção inteligente
static uint32_t total_bytes_sent = 0;
static uint32_t total_photos_sent = 0;
static uint32_t total_photos_captured = 0;
static uint32_t capture_count = 0;
static camera_fb_t *reference_frame = NULL;
static uint32_t reference_count = 0;
static float last_difference = 0.0f;

// Configurações de detecção
#define CHANGE_THRESHOLD 8.0f        // 8% mudança mínima
#define ALERT_THRESHOLD 15.0f        // 15% alerta crítico
#define REFERENCE_UPDATE_INTERVAL 20 // Atualizar referência a cada 20 capturas

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

// Atualizar frame de referência
static void update_reference_frame(camera_fb_t *fb)
{
    if (reference_frame) {
        esp_camera_fb_return(reference_frame);
        reference_frame = NULL;
    }
    
    // Alocar e copiar novo frame de referência
    reference_frame = (camera_fb_t*)heap_caps_malloc(sizeof(camera_fb_t), MALLOC_CAP_SPIRAM);
    if (reference_frame) {
        memcpy(reference_frame, fb, sizeof(camera_fb_t));
        
        // Alocar e copiar buffer de imagem
        reference_frame->buf = (uint8_t*)heap_caps_malloc(fb->len, MALLOC_CAP_SPIRAM);
        if (reference_frame->buf) {
            memcpy(reference_frame->buf, fb->buf, fb->len);
            reference_count++;
            ESP_LOGI(TAG, "📸 Referência atualizada #%" PRIu32 " (%zu bytes)", (uint32_t)reference_count, fb->len);
        } else {
            free(reference_frame);
            reference_frame = NULL;
            ESP_LOGE(TAG, "❌ Falha ao alocar buffer de referência");
        }
    } else {
        ESP_LOGE(TAG, "❌ Falha ao alocar frame de referência");
    }
}

// Enviar imagem via MQTT
static void send_image_via_mqtt(camera_fb_t *fb, const char* reason, float difference)
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
    esp_err_t err = mqtt_send_image_with_info(fb, topic, reason, difference);
    
    if (err == ESP_OK) {
        total_bytes_sent += fb->len;
        total_photos_sent++;
        ESP_LOGI(TAG, "📸 Imagem enviada: %zu bytes - %s (%.1f%%)", fb->len, reason, difference);
    } else {
        ESP_LOGE(TAG, "❌ Falha ao enviar imagem: %s", esp_err_to_name(err));
    }
    
    if (SNIFFER_ENABLED && wifi_sniffer_is_active()) {
        wifi_sniffer_mark_image_end();
    }
}

// Capturar e analisar foto (versão inteligente)
static void capture_and_analyze_photo(void)
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
    total_photos_captured++;
    ESP_LOGI(TAG, "📷 Foto capturada: %zu bytes (%zux%zu)", 
             fb->len, fb->width, fb->height);

    bool should_send = false;
    float difference = 0.0f;
    const char* reason = "unknown";
    ei_result_t ei_result = {0};
    
    // Primeira captura sempre é enviada e vira referência
    if (!reference_frame) {
        should_send = true;
        reason = "reference_established";
        difference = 0.0f;
        update_reference_frame(fb);
        ESP_LOGI(TAG, "🎯 Primeira captura - estabelecendo referência");
    } else {
        // Comparar com frame de referência
        difference = calculate_image_difference(reference_frame, fb);
        
        // Opcional: rodar inferência Edge Impulse para suporte a classes
        // Se o usuário habilitar EI, os resultados poderão ser enviados via MQTT
        if (ei_run_inference(fb, &ei_result) == ESP_OK) {
            ESP_LOGI(TAG, "🧠 EI: %d classes, %.2f ms", ei_result.count, ei_result.latency_ms);
        }
        last_difference = difference;
        
        ESP_LOGI(TAG, "🔍 Diferença calculada: %.1f%%", difference);
        
        // Determinar se deve enviar baseado na diferença
        if (difference >= ALERT_THRESHOLD) {
            should_send = true;
            reason = "anomaly_detected";
            ESP_LOGI(TAG, "🚨 ANOMALIA DETECTADA: %.1f%% (>= %.1f%%)", difference, ALERT_THRESHOLD);
        } else if (difference >= CHANGE_THRESHOLD) {
            should_send = true;
            reason = "significant_change";
            ESP_LOGI(TAG, "📊 Mudança significativa: %.1f%% (>= %.1f%%)", difference, CHANGE_THRESHOLD);
        } else {
            should_send = false;
            reason = "no_change";
            ESP_LOGI(TAG, "✅ Sem mudanças significativas: %.1f%% (< %.1f%%)", difference, CHANGE_THRESHOLD);
        }
        
        // Atualizar referência periodicamente ou em grandes mudanças
        if ((capture_count % REFERENCE_UPDATE_INTERVAL == 0) || (difference >= ALERT_THRESHOLD)) {
            update_reference_frame(fb);
            ESP_LOGI(TAG, "🔄 Referência atualizada (ciclo: %" PRIu32 ", diferença: %.1f%%)", 
                     (uint32_t)capture_count, difference);
        }
    }
    
    // Enviar imagem apenas se necessário
    if (should_send) {
        send_image_via_mqtt(fb, reason, difference);
        
        // Enviar alerta se for anomalia
        if (difference >= ALERT_THRESHOLD) {
            mqtt_send_alert(difference, fb);
        }
    } else {
        ESP_LOGI(TAG, "⏭️  Imagem não enviada (sem mudanças significativas)");
    }
    
    // Sempre enviar dados de monitoramento (para estatísticas)
    mqtt_send_monitoring_data(difference, fb->len, fb->width, fb->height, fb->format, DEVICE_ID);
    
    // Enviar status do sistema
    mqtt_send_monitoring(esp_get_free_heap_size(), 
                        heap_caps_get_free_size(MALLOC_CAP_SPIRAM), 
                        esp_timer_get_time() / 1000000);
    
    // Liberar memória EI
    ei_free_result(&ei_result);
    
    // Liberar frame
    esp_camera_fb_return(fb);
}

// Declaração da função de estatísticas
static void print_statistics(void);

// Task de monitoramento inteligente
static void monitoring_task(void *pvParameter)
{
    ESP_LOGI(TAG, "🚀 Task de monitoramento inteligente iniciada");
    
    while (1) {
        capture_and_analyze_photo();
        
        // Imprimir estatísticas a cada 10 capturas
        if (capture_count % 10 == 0) {
            print_statistics();
        }
        
        vTaskDelay(pdMS_TO_TICKS(CAPTURE_INTERVAL_MS));
    }
}

// Imprimir estatísticas
static void print_statistics(void)
{
    float send_ratio = total_photos_captured > 0 ? 
        (float)total_photos_sent / (float)total_photos_captured * 100.0f : 0.0f;
    
    ESP_LOGI(TAG, "\n📈 === ESTATÍSTICAS DE MONITORAMENTO INTELIGENTE ===");
    ESP_LOGI(TAG, "📷 Fotos: %" PRIu32 " enviadas / %" PRIu32 " capturadas (%.1f%% taxa de envio)",
             (uint32_t)total_photos_sent, (uint32_t)total_photos_captured, send_ratio);
    ESP_LOGI(TAG, "📡 Dados: %.2f KB transmitidos", total_bytes_sent / 1024.0f);
    ESP_LOGI(TAG, "📊 Média: %" PRIu32 " bytes/foto", 
             (uint32_t)(total_photos_sent > 0 ? total_bytes_sent / total_photos_sent : 0));
    ESP_LOGI(TAG, "🔍 Última diferença: %.1f%%", last_difference);
    ESP_LOGI(TAG, "🎯 Referências: %" PRIu32 " atualizações", (uint32_t)reference_count);
    ESP_LOGI(TAG, "💾 Heap: %" PRIu32 " KB livre", (uint32_t)(esp_get_free_heap_size() / 1024));
    ESP_LOGI(TAG, "💾 PSRAM: %" PRIu32 " KB livre", (uint32_t)(heap_caps_get_free_size(MALLOC_CAP_SPIRAM) / 1024));
    ESP_LOGI(TAG, "🔄 Modo: DETECÇÃO INTELIGENTE (%.1f%% threshold)", CHANGE_THRESHOLD);
    ESP_LOGI(TAG, "=======================================");
}

void app_main(void)
{
    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "🧠 Sistema ESP32-CAM - VERSÃO INTELIGENTE");
    ESP_LOGI(TAG, "📊 Detecção Robusta com Análise RGB565");
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
    
    // Inicializar EI (opcional; não falhar se indisponível)
    if (ei_init() == ESP_OK) {
        ESP_LOGI(TAG, "🧠 Edge Impulse inicializado");
        
        // Inicializar SPIFFS para testes com dataset
        if (init_spiffs() == ESP_OK) {
            ESP_LOGI(TAG, "📁 SPIFFS inicializado - Teste de dataset disponível");
            
            // Executar teste automático se configurado
            #ifdef RUN_DATASET_TEST_ON_BOOT
            ESP_LOGI(TAG, "🧪 Executando teste automático do dataset...");
            vTaskDelay(pdMS_TO_TICKS(2000));
            test_dataset_images();
            #endif
        }
    } else {
        ESP_LOGW(TAG, "⚠️  Edge Impulse não inicializado (stub)");
    }

    // Configurações finais
    ESP_LOGI(TAG, "🧠 Configuração INTELIGENTE:");
    ESP_LOGI(TAG, "   - Resolução: HVGA 480x320 (qualidade premium)");
    ESP_LOGI(TAG, "   - JPEG Quality: %d", JPEG_QUALITY);
    ESP_LOGI(TAG, "   - Algoritmo: RGB565 + análise por blocos 32x32");
    ESP_LOGI(TAG, "   - Threshold mudança: %.1f%%", CHANGE_THRESHOLD);
    ESP_LOGI(TAG, "   - Threshold alerta: %.1f%%", ALERT_THRESHOLD);
    ESP_LOGI(TAG, "   - Intervalo: %d segundos", CAPTURE_INTERVAL_MS / 1000);
    ESP_LOGI(TAG, "   - Economia esperada: ~90%% vs versão simples");

    // Inicializar WiFi sniffer
    if (SNIFFER_ENABLED) {
        ESP_LOGI(TAG, "📡 Inicializando WiFi Sniffer...");
        if (wifi_sniffer_init(SNIFFER_CHANNEL) == ESP_OK && wifi_sniffer_start() == ESP_OK) {
            ESP_LOGI(TAG, "✅ WiFi Sniffer ativo no canal %d", SNIFFER_CHANNEL);
        } else {
            ESP_LOGW(TAG, "⚠️  WiFi Sniffer desabilitado");
        }
    }

    // Iniciar monitoramento inteligente
    xTaskCreate(monitoring_task, "monitoring_task_intelligent", 8192, NULL, 5, NULL);
    ESP_LOGI(TAG, "✅ Sistema INTELIGENTE iniciado!");
} 