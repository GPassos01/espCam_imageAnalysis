/**
 * @file mqtt_send.c
 * @brief Implementação do envio de dados via MQTT
 * 
 * @author Gabriel Passos - UNESP 2025
 */
#include <sys/param.h>
#include "mqtt_send.h"
#include "config.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "mqtt_client.h"
#include "cJSON.h"
#include <stdio.h>
#include <string.h>
#include "init_net.h"
#include "mbedtls/base64.h"

static const char *TAG = "MQTT_SEND";

esp_err_t mqtt_send_image(camera_fb_t* frame, const char* topic) {
    if (!frame || !topic) {
        ESP_LOGE(TAG, "Parâmetros inválidos");
        return ESP_ERR_INVALID_ARG;
    }

    if (!mqtt_client) {
        ESP_LOGE(TAG, "Cliente MQTT não inicializado");
        return ESP_ERR_INVALID_STATE;
    }

    // Calcular tamanho necessário para base64 (4/3 do tamanho original + padding)
    size_t base64_len = ((frame->len + 2) / 3) * 4 + 1;
    char* base64_buffer = malloc(base64_len);
    if (!base64_buffer) {
        ESP_LOGE(TAG, "Falha ao alocar memória para base64");
        return ESP_ERR_NO_MEM;
    }

    // Converter imagem para base64
    size_t out_len = 0;
    int ret = mbedtls_base64_encode((unsigned char*)base64_buffer, base64_len, &out_len, frame->buf, frame->len);
    if (ret != 0) {
        ESP_LOGE(TAG, "Falha na codificação base64: %d", ret);
        free(base64_buffer);
        return ESP_FAIL;
    }

    // Criar JSON com a imagem em base64
    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "device_id", DEVICE_ID);
    cJSON_AddNumberToObject(root, "timestamp", esp_timer_get_time() / 1000000);
    cJSON_AddNumberToObject(root, "width", frame->width);
    cJSON_AddNumberToObject(root, "height", frame->height);
    cJSON_AddNumberToObject(root, "format", frame->format);
    cJSON_AddNumberToObject(root, "size", frame->len);
    cJSON_AddStringToObject(root, "image", base64_buffer);
    
    char *json_payload = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    free(base64_buffer);

    if (!json_payload) {
        ESP_LOGE(TAG, "Falha ao criar JSON payload");
        return ESP_ERR_NO_MEM;
    }

    // Enviar JSON com imagem via MQTT
    int msg_id = esp_mqtt_client_publish(mqtt_client, topic, json_payload, strlen(json_payload), 1, 0);
    free(json_payload);

    if (msg_id < 0) {
        ESP_LOGE(TAG, "Falha ao publicar imagem via MQTT");
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Imagem enviada via MQTT: %zu bytes -> %zu bytes base64", frame->len, out_len);
    return ESP_OK;
}

esp_err_t mqtt_send_monitoring(uint32_t free_heap, uint32_t free_psram, uint32_t uptime) {
    if (!mqtt_client) {
        ESP_LOGE(TAG, "Cliente MQTT não inicializado");
        return ESP_ERR_INVALID_STATE;
    }

    cJSON *root = cJSON_CreateObject();
    if (!root) {
        ESP_LOGE(TAG, "Falha ao criar objeto JSON");
        return ESP_ERR_NO_MEM;
    }
    
    cJSON_AddStringToObject(root, "device_id", DEVICE_ID);
    cJSON_AddNumberToObject(root, "free_heap", free_heap);
    cJSON_AddNumberToObject(root, "free_psram", free_psram);
    cJSON_AddNumberToObject(root, "uptime", uptime);
    
    char *payload = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    
    if (!payload) {
        ESP_LOGE(TAG, "Falha ao serializar JSON");
        return ESP_ERR_NO_MEM;
    }

    char topic[64];
    snprintf(topic, sizeof(topic), "%s/%s", MQTT_TOPIC_BASE, MQTT_TOPIC_STATUS);
    
    int msg_id = esp_mqtt_client_publish(mqtt_client, topic, payload, strlen(payload), 1, 0);
    free(payload);
    
    if (msg_id < 0) {
        ESP_LOGE(TAG, "Falha ao publicar status via MQTT");
        return ESP_FAIL;
    }

    return ESP_OK;
}

esp_err_t mqtt_send_alert(float difference_percent, camera_fb_t* frame) {
    if (!mqtt_client) {
        ESP_LOGE(TAG, "Cliente MQTT não inicializado");
        return ESP_ERR_INVALID_STATE;
    }

    // Envia alerta
    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "device_id", DEVICE_ID);
    cJSON_AddNumberToObject(root, "difference", difference_percent);
    cJSON_AddStringToObject(root, "type", "motion");
    cJSON_AddNumberToObject(root, "timestamp", esp_timer_get_time() / 1000000);
    
    char *payload = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);

    char topic[64];
    snprintf(topic, sizeof(topic), "%s/%s", MQTT_TOPIC_BASE, MQTT_TOPIC_ALERT);
    
    esp_mqtt_client_publish(mqtt_client, topic, payload, strlen(payload), 1, 0);
    free(payload);

    // Se configurado, envia a imagem
    if (SEND_IMAGE_ON_ALERT && frame) {
        snprintf(topic, sizeof(topic), "%s/%s", MQTT_TOPIC_BASE, MQTT_TOPIC_IMAGE);
        return mqtt_send_image(frame, topic);
    }

    return ESP_OK;
}

esp_err_t mqtt_send_sniffer_stats(uint32_t total_packets, uint32_t total_bytes,
                                 uint32_t mqtt_packets, uint32_t mqtt_bytes) {
    if (!mqtt_client) {
        ESP_LOGE(TAG, "Cliente MQTT não inicializado");
        return ESP_ERR_INVALID_STATE;
    }

    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, "device_id", DEVICE_ID);
    cJSON_AddNumberToObject(root, "total_packets", total_packets);
    cJSON_AddNumberToObject(root, "total_bytes", total_bytes);
    cJSON_AddNumberToObject(root, "mqtt_packets", mqtt_packets);
    cJSON_AddNumberToObject(root, "mqtt_bytes", mqtt_bytes);
    cJSON_AddNumberToObject(root, "timestamp", esp_timer_get_time() / 1000000);
    
    char *payload = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);

    char topic[64];
    snprintf(topic, sizeof(topic), "%s/%s", MQTT_TOPIC_BASE, MQTT_TOPIC_STATS);
    
    esp_mqtt_client_publish(mqtt_client, topic, payload, strlen(payload), 1, 0);
    free(payload);

    return ESP_OK;
}

esp_err_t mqtt_send_monitoring_data(float difference, uint32_t image_size, 
                                   uint16_t width, uint16_t height, 
                                   uint8_t format, const char* device_id) {
    if (!mqtt_client) {
        ESP_LOGE(TAG, "Cliente MQTT não inicializado");
        return ESP_ERR_INVALID_STATE;
    }
    
    if (!device_id || strlen(device_id) == 0) {
        ESP_LOGE(TAG, "Device ID inválido");
        return ESP_ERR_INVALID_ARG;
    }
    
    if (difference < 0.0f || difference > 100.0f) {
        ESP_LOGW(TAG, "Diferença fora do range esperado: %.3f%%", difference);
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