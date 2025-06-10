/**
 * Sistema de Monitoramento ESP32-CAM
 * 
 * Funcionalidades principais:
 * - Captura fotos a cada 15 segundos (QVGA 320x240)
 * - Compara imagens usando algoritmo de 30 pontos
 * - Detecta mudanças (10%) e alertas (30%)
 * - Envia dados via MQTT com reconexão automática
 * - Gerencia memória PSRAM para imagens grandes
 * - Monitoramento de sistema em tempo real
 * 
 * Thresholds configuráveis:
 * - CHANGE_THRESHOLD: 10% para detectar mudança
 * - ALERT_THRESHOLD: 30% para alerta crítico
 * 
 * @author Gabriel Passos - UNESP 2025
 */

#include <stdio.h>
#include <string.h>
#include <math.h>
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
// #include "driver/gpio.h"  // Não necessário para flash desabilitado

// Módulos do projeto
#include "model/compare.h"
#include "model/mqtt_send.h"
#include "model/init_net.h"
#include "model/wifi_sniffer.h"
#include "config.h"

// Configurações de análise
#define CHANGE_THRESHOLD 0.10f    // 10% de diferença para detectar mudança (mais sensível)
#define ALERT_THRESHOLD 0.30f     // 30% para alerta de mudança significativa (mais sensível)

// Pinos da câmera ESP32-CAM
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
#define CAM_PIN_FLASH   4

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
extern esp_mqtt_client_handle_t mqtt_client;  // Declarado em init_net.c
static bool mqtt_connected = false;
static uint32_t total_bytes_sent = 0;
static uint32_t total_photos_sent = 0;
static camera_frame_t *previous_frame = NULL;

/**
 * Handler de eventos MQTT
 * Gerencia conexão/desconexão e erros do cliente MQTT
 * Atualiza flag global mqtt_connected para controle de envios
 */
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, 
                              int32_t event_id, void *event_data)
{
    esp_mqtt_event_handle_t event = event_data;
    
    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT Conectado");
            mqtt_connected = true;
            break;
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "MQTT Desconectado");
            mqtt_connected = false;
            break;
        case MQTT_EVENT_ERROR:
            ESP_LOGE(TAG, "MQTT Erro");
            break;
        default:
            break;
    }
}

/**
 * Inicializar câmera ESP32-CAM OV2640
 * Configuração: QVGA (320x240), JPEG qualidade 10, double buffering
 * Requer PSRAM habilitado para funcionamento adequado
 * 
 * @return ESP_OK se inicializada com sucesso
 */
static esp_err_t init_camera(void)
{
    esp_err_t err = esp_camera_init(&camera_config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Falha na inicialização da câmera: 0x%x", err);
    } else {
        ESP_LOGI(TAG, "Câmera inicializada com sucesso");
    }
    return err;
}

// Criar frame a partir do framebuffer da câmera
static camera_frame_t* create_camera_frame(camera_fb_t *fb)
{
    if (!fb) return NULL;
    
    camera_frame_t *frame = heap_caps_malloc(sizeof(camera_frame_t), MALLOC_CAP_SPIRAM);
    if (!frame) {
        ESP_LOGE(TAG, "Falha ao alocar frame");
        return NULL;
    }
    
    // Alocar buffer para cópia da imagem
    frame->buf = heap_caps_malloc(fb->len, MALLOC_CAP_SPIRAM);
    if (!frame->buf) {
        ESP_LOGE(TAG, "Falha ao alocar buffer de imagem");
        free(frame);
        return NULL;
    }
    
    // Copiar dados
    memcpy(frame->buf, fb->buf, fb->len);
    frame->len = fb->len;
    frame->width = fb->width;
    frame->height = fb->height;
    frame->format = fb->format;
    frame->timestamp = (uint32_t)(esp_timer_get_time() / 1000000LL);
    
    return frame;
}

// Liberar frame
static void free_camera_frame(camera_frame_t *frame)
{
    if (frame) {
        if (frame->buf) {
            free(frame->buf);
        }
        free(frame);
    }
}

// Enviar dados via MQTT
static void send_monitoring_data(float difference, camera_fb_t *fb, bool is_alert)
{
    if (!mqtt_connected) return;
    
    // Enviar dados de monitoramento
    char json[256];
    snprintf(json, sizeof(json),
             "{\"timestamp\":%lu,\"image_size\":%zu,\"difference\":%.3f,"
             "\"width\":%zu,\"height\":%zu,\"format\":%d,"
             "\"location\":\"monitoring_esp32cam\",\"mode\":\"image_comparison\"}",
             (uint32_t)(esp_timer_get_time() / 1000000LL),
             fb->len, difference, fb->width, fb->height, fb->format);
    
    esp_mqtt_client_publish(mqtt_client, "monitoring/data", json, 0, 1, 0);
    
    // Se for alerta, enviar notificação especial
    if (is_alert) {
        char alert[200];
        snprintf(alert, sizeof(alert),
                 "{\"alert\":\"significant_change\",\"difference\":%.3f,"
                 "\"timestamp\":%lu,\"image_size\":%zu,"
                 "\"location\":\"monitoring_esp32cam\",\"mode\":\"image_comparison\"}",
                 difference, (uint32_t)(esp_timer_get_time() / 1000000LL), fb->len);
        
        esp_mqtt_client_publish(mqtt_client, "monitoring/alert", alert, 0, 1, 0);
        ESP_LOGW(TAG, "🚨 ALERTA: Mudança significativa detectada (%.1f%%)", difference * 100);
    }
}

// Enviar imagem via MQTT
static void send_image_via_mqtt(camera_fb_t *fb, const char* reason)
{
    if (!mqtt_connected || !fb) return;
    
    // Marcar início da transmissão de imagem para o sniffer
    if (SNIFFER_ENABLED && wifi_sniffer_is_active()) {
        wifi_sniffer_mark_image_start();
    }
    
    const size_t chunk_size = 1024;
    uint32_t timestamp = (uint32_t)(esp_timer_get_time() / 1000000LL);
    uint32_t bytes_sent = 0;
    
    // Enviar metadados
    char metadata[300];
    snprintf(metadata, sizeof(metadata), 
             "{\"timestamp\":%lu,\"size\":%zu,\"device\":\"%s\",\"reason\":\"%s\","
             "\"width\":%zu,\"height\":%zu,\"format\":%d}",
             timestamp, fb->len, DEVICE_ID, reason, fb->width, fb->height, fb->format);
    
    int msg_id = esp_mqtt_client_publish(mqtt_client, "monitoring/image/metadata", 
                                        metadata, 0, 1, 0);
    if (msg_id >= 0) {
        bytes_sent += strlen(metadata);
    }

    // Enviar dados da imagem em chunks
    size_t chunks_sent = 0;
    for (size_t offset = 0; offset < fb->len; offset += chunk_size) {
        size_t current_chunk = (offset + chunk_size > fb->len) ? 
                              (fb->len - offset) : chunk_size;
        
        char topic[80];
        snprintf(topic, sizeof(topic), "monitoring/image/data/%lu/%zu", timestamp, offset);
        
        msg_id = esp_mqtt_client_publish(mqtt_client, topic,
                                       (char*)(fb->buf + offset), 
                                       current_chunk, 0, 0);
        if (msg_id >= 0) {
            chunks_sent++;
            bytes_sent += current_chunk;
        }
        
        vTaskDelay(pdMS_TO_TICKS(20)); // Pequena pausa entre chunks
    }
    
    total_bytes_sent += bytes_sent;
    total_photos_sent++;
    
    // Marcar fim da transmissão de imagem para o sniffer
    if (SNIFFER_ENABLED && wifi_sniffer_is_active()) {
        wifi_sniffer_mark_image_end();
    }
    
    ESP_LOGI(TAG, "📸 Imagem enviada: %zu chunks (%lu bytes) - Motivo: %s", 
             chunks_sent, bytes_sent, reason);
}

// Capturar e analisar foto
static void capture_and_analyze_photo(void)
{
    if (!mqtt_connected) {
        ESP_LOGW(TAG, "MQTT desconectado, pulando captura");
        return;
    }

    ESP_LOGI(TAG, "📸 Capturando foto...");
    
    // Capturar imagem sem flash
    camera_fb_t *fb = esp_camera_fb_get();
    
    if (!fb) {
        ESP_LOGE(TAG, "Falha na captura");
        return;
    }

    ESP_LOGI(TAG, "📷 Foto capturada: %zu bytes (%zux%zu)", 
             fb->len, fb->width, fb->height);

    // Criar frame atual
    camera_frame_t *current_frame = create_camera_frame(fb);
    if (!current_frame) {
        ESP_LOGE(TAG, "Falha ao criar frame");
        esp_camera_fb_return(fb);
        return;
    }
    
    float difference = 0.0f;
    bool is_alert = false;
    bool send_image = false;
    const char* reason = "periodic";
    
    if (previous_frame) {
        // Calcular diferença entre imagens
        difference = calculate_image_difference(previous_frame, current_frame);
        
        ESP_LOGI(TAG, "🔍 Diferença calculada: %.1f%%", difference * 100);
        
        // Verificar se houve mudança significativa
        if (difference >= CHANGE_THRESHOLD) {
            ESP_LOGI(TAG, "📊 Mudança detectada: %.1f%%", difference * 100);
            send_image = true;
            reason = "change_detected";
            
            if (difference >= ALERT_THRESHOLD) {
                is_alert = true;
                reason = "significant_change";
            }
        }
    } else {
        // Primeira imagem - sempre enviar
        ESP_LOGI(TAG, "📷 Primeira captura - enviando imagem de referência");
        send_image = true;
        reason = "first_capture";
    }
    
    // Enviar dados de monitoramento sempre
    send_monitoring_data(difference, fb, is_alert);
    
    // Enviar imagem conforme necessário (ANTES de atualizar o frame anterior)
    if (send_image) {
        send_image_via_mqtt(fb, reason);
    }
    
    // Liberar frame anterior e atualizar com atual
    if (previous_frame) {
        free_camera_frame(previous_frame);
    }
    previous_frame = current_frame;
    
    esp_camera_fb_return(fb);
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
    
    // Informações de memória
    size_t free_heap = esp_get_free_heap_size();
    size_t free_psram = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
    size_t min_free_heap = esp_get_minimum_free_heap_size();
    
    ESP_LOGI(TAG, "💾 Heap livre: %zu KB (mínimo: %zu KB)", 
             free_heap / 1024, min_free_heap / 1024);
    ESP_LOGI(TAG, "💾 PSRAM livre: %zu KB", free_psram / 1024);
    
    // Alerta de memória baixa
    if (free_heap < 50000) {  // Menos que 50KB
        ESP_LOGW(TAG, "⚠️  MEMÓRIA HEAP BAIXA!");
    }
    if (free_psram < 1000000) {  // Menos que 1MB
        ESP_LOGW(TAG, "⚠️  MEMÓRIA PSRAM BAIXA!");
    }
    
    ESP_LOGI(TAG, "=======================================");
    
    // Imprimir estatísticas do sniffer se habilitado
    if (SNIFFER_ENABLED && wifi_sniffer_is_active()) {
        wifi_sniffer_print_stats();
    }
}

// Task principal de monitoramento
static void monitoring_task(void *pvParameter)
{
    ESP_LOGI(TAG, "🚀 Iniciando monitoramento de imagens (15s de intervalo)");
    
    TickType_t last_capture = 0;
    TickType_t last_stats = 0;
    TickType_t last_sniffer_stats = 0;
    
    while (1) {
        TickType_t now = xTaskGetTickCount();
        
        // Capturar e analisar a cada 15 segundos
        if ((now - last_capture) >= pdMS_TO_TICKS(15000)) {
            capture_and_analyze_photo();
            last_capture = now;
        }
        
        // Imprimir estatísticas a cada 5 minutos
        if ((now - last_stats) >= pdMS_TO_TICKS(300000)) {
            print_statistics();
            last_stats = now;
        }
        
        // Estatísticas do sniffer mais frequentes (se habilitado)
        if (SNIFFER_ENABLED && wifi_sniffer_is_active() && 
            (now - last_sniffer_stats) >= pdMS_TO_TICKS(SNIFFER_STATS_INTERVAL * 1000)) {
            wifi_sniffer_print_stats();
            
            // Enviar estatísticas via MQTT se conectado
            if (mqtt_connected) {
                esp_err_t ret = wifi_sniffer_send_mqtt_stats(mqtt_client, DEVICE_ID);
                if (ret != ESP_OK) {
                    ESP_LOGW(TAG, "Falha ao enviar stats do sniffer via MQTT");
                }
            }
            
            last_sniffer_stats = now;
        }
        
        vTaskDelay(pdMS_TO_TICKS(1000)); // Check a cada segundo
    }
}

void app_main(void)
{
    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "🔍 Sistema de Monitoramento por Imagens");
    ESP_LOGI(TAG, "📊 Detecção de Mudanças Visuais");
    ESP_LOGI(TAG, "Gabriel Passos - UNESP 2025");
    ESP_LOGI(TAG, "========================================");
    
    // Inicializar NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    
    // Inicializar networking
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();
    
    // Flash LED desabilitado para economia de energia

    // Inicializar componentes
    ESP_LOGI(TAG, "📷 Inicializando câmera...");
    ESP_ERROR_CHECK(init_camera());

    ESP_LOGI(TAG, "🌐 Conectando WiFi...");
    wifi_init_sta(WIFI_SSID, WIFI_PASS);
    
    ESP_LOGI(TAG, "📡 Conectando MQTT...");
    mqtt_init(MQTT_BROKER_URI, MQTT_USERNAME, MQTT_PASSWORD);
    
    // Registrar handler de eventos MQTT
    esp_mqtt_client_register_event(mqtt_client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    
    // Aguardar conexão MQTT
    vTaskDelay(pdMS_TO_TICKS(3000));
    
    ESP_LOGI(TAG, "🔍 Configuração de detecção:");
    ESP_LOGI(TAG, "   - Mudança mínima: %.1f%%", CHANGE_THRESHOLD * 100);
    ESP_LOGI(TAG, "   - Alerta em: %.1f%%", ALERT_THRESHOLD * 100);
    ESP_LOGI(TAG, "   - Intervalo: 15 segundos");

    // Inicializar e iniciar WiFi sniffer se habilitado
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

    // Iniciar task de monitoramento
    xTaskCreate(monitoring_task, "monitoring", 8192, NULL, 5, NULL);
    
    ESP_LOGI(TAG, "✅ Sistema de monitoramento iniciado!");
}
