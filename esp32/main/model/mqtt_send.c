#include "mqtt_send.h"
#include "esp_log.h"
#include "esp_timer.h"
#include <stdio.h>
#include <string.h>

static const char *TAG = "MQTT_SEND";

extern esp_mqtt_client_handle_t mqtt_client;

esp_err_t mqtt_send_water_level_data(float image_level, float sensor_level, 
                                   float confidence, const char* device_id) {
    if (!mqtt_client) {
        ESP_LOGE(TAG, "Cliente MQTT não inicializado");
        return ESP_ERR_INVALID_STATE;
    }
    
    // Payload otimizado para dados da IC
    char payload[300];
    uint64_t timestamp = esp_timer_get_time() / 1000000LL;
    
    int ret = snprintf(payload, sizeof(payload),
        "{"
        "\"timestamp\":%llu,"
        "\"device_id\":\"%s\","
        "\"image_level\":%.2f,"
        "\"sensor_level\":%.2f,"
        "\"confidence\":%.2f,"
        "\"mode\":\"embedded_processing\""
        "}",
        timestamp, device_id, image_level, sensor_level, confidence);
    
    if (ret < 0 || ret >= sizeof(payload)) {
        ESP_LOGE(TAG, "Erro ao formatar payload");
        return ESP_ERR_INVALID_SIZE;
    }
    
    int msg_id = esp_mqtt_client_publish(mqtt_client, TOPIC_WATER_LEVEL, 
                                       payload, 0, 1, 0);
    
    if (msg_id < 0) {
        ESP_LOGE(TAG, "Falha ao enviar dados de nível");
        return ESP_FAIL;
    }
    
    ESP_LOGI(TAG, "Dados enviados: IMG=%.1f%% SENS=%.1fcm CONF=%.2f", 
             image_level, sensor_level, confidence);
    return ESP_OK;
}

esp_err_t mqtt_send_alert(float level, const char* alert_type, const char* device_id) {
    if (!mqtt_client) {
        ESP_LOGE(TAG, "Cliente MQTT não inicializado");
        return ESP_ERR_INVALID_STATE;
    }
    
    char payload[250];
    uint64_t timestamp = esp_timer_get_time() / 1000000LL;
    
    int ret = snprintf(payload, sizeof(payload),
        "{"
        "\"timestamp\":%llu,"
        "\"device_id\":\"%s\","
        "\"alert_type\":\"%s\","
        "\"level\":%.2f,"
        "\"severity\":\"high\""
        "}",
        timestamp, device_id, alert_type, level);
    
    if (ret < 0 || ret >= sizeof(payload)) {
        ESP_LOGE(TAG, "Erro ao formatar alerta");
        return ESP_ERR_INVALID_SIZE;
    }
    
    int msg_id = esp_mqtt_client_publish(mqtt_client, TOPIC_ALERTS, 
                                       payload, 0, 1, 0);
    
    if (msg_id < 0) {
        ESP_LOGE(TAG, "Falha ao enviar alerta");
        return ESP_FAIL;
    }
    
    ESP_LOGW(TAG, "🚨 ALERTA enviado: %s - Nível: %.1f%%", alert_type, level);
    return ESP_OK;
}

esp_err_t mqtt_send_system_status(uint32_t uptime, size_t free_heap, 
                                size_t free_psram, const char* device_id) {
    if (!mqtt_client) {
        ESP_LOGE(TAG, "Cliente MQTT não inicializado");
        return ESP_ERR_INVALID_STATE;
    }
    
    char payload[400];
    uint64_t timestamp = esp_timer_get_time() / 1000000LL;
    
    int ret = snprintf(payload, sizeof(payload),
        "{"
        "\"timestamp\":%llu,"
        "\"device_id\":\"%s\","
        "\"uptime\":%lu,"
        "\"free_heap\":%zu,"
        "\"free_psram\":%zu,"
        "\"status\":\"operational\","
        "\"firmware_version\":\"IC_v1.0\""
        "}",
        timestamp, device_id, uptime, free_heap, free_psram);
    
    if (ret < 0 || ret >= sizeof(payload)) {
        ESP_LOGE(TAG, "Erro ao formatar status");
        return ESP_ERR_INVALID_SIZE;
    }
    
    int msg_id = esp_mqtt_client_publish(mqtt_client, TOPIC_SYSTEM_STATUS, 
                                       payload, 0, 0, 0); // QoS 0 para status
    
    if (msg_id < 0) {
        ESP_LOGE(TAG, "Falha ao enviar status");
        return ESP_FAIL;
    }
    
    ESP_LOGI(TAG, "Status do sistema enviado");
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
        "\"device_id\":\"%s\","
        "\"reason\":\"%s\","
        "\"image_size\":%zu,"
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
    int msg_id = esp_mqtt_client_publish(mqtt_client, TOPIC_IMAGE_METADATA, 
                                       metadata, 0, 1, 0);
    
    if (msg_id < 0) {
        ESP_LOGE(TAG, "Falha ao enviar metadados da imagem");
        return ESP_FAIL;
    }
    
    // Enviar imagem em chunks (apenas para fallback/debug)
    const size_t chunk_size = 1024;
    size_t chunks_sent = 0;
    
    for (size_t offset = 0; offset < fb->len; offset += chunk_size) {
        size_t current_chunk = (offset + chunk_size > fb->len) ? 
                              (fb->len - offset) : chunk_size;
        
        char topic[128];
        snprintf(topic, sizeof(topic), "%s/%llu/%zu/%zu", 
                TOPIC_IMAGE_DATA, timestamp, offset, fb->len);
        
        msg_id = esp_mqtt_client_publish(mqtt_client, topic,
                                       (char*)(fb->buf + offset), current_chunk, 0, 0);
        if (msg_id >= 0) {
            chunks_sent++;
        }
        
        vTaskDelay(pdMS_TO_TICKS(100)); // Delay para não sobrecarregar
    }
    
    ESP_LOGW(TAG, "Imagem de fallback enviada: %zu chunks (%zu bytes) - Razão: %s", 
             chunks_sent, fb->len, reason);
    
    return ESP_OK;
} 