#include "mqtt_send.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "mqtt_client.h"
#include <stdio.h>
#include <string.h>

static const char *TAG = "MQTT_SEND";

extern esp_mqtt_client_handle_t mqtt_client;

esp_err_t mqtt_send_monitoring_data(float difference, uint32_t image_size, 
                                   uint16_t width, uint16_t height, 
                                   uint8_t format, const char* device_id) {
    if (!mqtt_client) {
        ESP_LOGE(TAG, "Cliente MQTT não inicializado");
        return ESP_ERR_INVALID_STATE;
    }
    
    char payload[300];
    uint64_t timestamp = esp_timer_get_time() / 1000000LL;
    
    int ret = snprintf(payload, sizeof(payload),
        "{"
        "\"timestamp\":%llu,"
        "\"device\":\"%s\","
        "\"difference\":%.3f,"
        "\"image_size\":%lu,"
        "\"width\":%u,"
        "\"height\":%u,"
        "\"format\":%u,"
        "\"location\":\"monitoring_esp32cam\","
        "\"mode\":\"image_comparison\""
        "}",
        timestamp, device_id, difference, image_size, width, height, format);
    
    if (ret < 0 || ret >= sizeof(payload)) {
        ESP_LOGE(TAG, "Erro ao formatar payload");
        return ESP_ERR_INVALID_SIZE;
    }
    
    int msg_id = esp_mqtt_client_publish(mqtt_client, "monitoring/data", 
                                       payload, 0, 1, 0);
    
    if (msg_id < 0) {
        ESP_LOGE(TAG, "Falha ao enviar dados de monitoramento");
        return ESP_FAIL;
    }
    
    return ESP_OK;
}

esp_err_t mqtt_send_alert(float difference, const char* alert_type, const char* device_id) {
    if (!mqtt_client) {
        ESP_LOGE(TAG, "Cliente MQTT não inicializado");
        return ESP_ERR_INVALID_STATE;
    }
    
    char payload[250];
    uint64_t timestamp = esp_timer_get_time() / 1000000LL;
    
    int ret = snprintf(payload, sizeof(payload),
        "{"
        "\"timestamp\":%llu,"
        "\"device\":\"%s\","
        "\"alert\":\"%s\","
        "\"difference\":%.3f,"
        "\"location\":\"monitoring_esp32cam\","
        "\"mode\":\"image_comparison\""
        "}",
        timestamp, device_id, alert_type, difference);
    
    if (ret < 0 || ret >= sizeof(payload)) {
        ESP_LOGE(TAG, "Erro ao formatar alerta");
        return ESP_ERR_INVALID_SIZE;
    }
    
    int msg_id = esp_mqtt_client_publish(mqtt_client, "monitoring/alert", 
                                       payload, 0, 1, 0);
    
    if (msg_id < 0) {
        ESP_LOGE(TAG, "Falha ao enviar alerta");
        return ESP_FAIL;
    }
    
    return ESP_OK;
}

esp_err_t mqtt_send_image_fallback(camera_fb_t *fb, const char* reason, const char* device_id) {
    if (!mqtt_client || !fb) {
        ESP_LOGE(TAG, "Parâmetros inválidos para envio de imagem");
        return ESP_ERR_INVALID_ARG;
    }
    
    // Enviar metadados da imagem primeiro
    char metadata[300];
    uint64_t timestamp = esp_timer_get_time() / 1000000LL;
    
    int ret = snprintf(metadata, sizeof(metadata),
        "{"
        "\"timestamp\":%llu,"
        "\"device\":\"%s\","
        "\"reason\":\"%s\","
        "\"size\":%zu,"
        "\"width\":%zu,"
        "\"height\":%zu,"
        "\"format\":%d"
        "}",
        timestamp, device_id, reason, fb->len, 
        fb->width, fb->height, fb->format);
    
    if (ret < 0 || ret >= sizeof(metadata)) {
        ESP_LOGE(TAG, "Erro ao formatar metadados da imagem");
        return ESP_ERR_INVALID_SIZE;
    }
    
    // Enviar metadados
    int msg_id = esp_mqtt_client_publish(mqtt_client, "monitoring/image/metadata", 
                                       metadata, 0, 1, 0);
    
    if (msg_id < 0) {
        ESP_LOGE(TAG, "Falha ao enviar metadados da imagem");
        return ESP_FAIL;
    }
    
    // Enviar imagem em chunks
    const size_t chunk_size = 1024;
    size_t chunks_sent = 0;
    
    for (size_t offset = 0; offset < fb->len; offset += chunk_size) {
        size_t current_chunk = (offset + chunk_size > fb->len) ? 
                              (fb->len - offset) : chunk_size;
        
        char topic[128];
        snprintf(topic, sizeof(topic), "monitoring/image/data/%llu/%zu", 
                timestamp, offset);
        
        msg_id = esp_mqtt_client_publish(mqtt_client, topic,
                                       (char*)(fb->buf + offset), current_chunk, 0, 0);
        if (msg_id >= 0) {
            chunks_sent++;
        }
        
        vTaskDelay(pdMS_TO_TICKS(50)); // Delay para não sobrecarregar
    }
    
    ESP_LOGI(TAG, "Imagem enviada: %zu chunks (%zu bytes) - Razão: %s", 
             chunks_sent, fb->len, reason);
    
    return ESP_OK;
} 