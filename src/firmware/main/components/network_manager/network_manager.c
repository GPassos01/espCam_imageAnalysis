// src/firmware/main/components/network_manager/network_manager.c

#include "network_manager.h"
#include "esp_log.h"
#include "esp_wifi.h"
#include "mqtt_client.h"
#include "esp_timer.h"
#include "model/init_net.h"
#include "config.h"

static const char *TAG = "NETWORK_MANAGER";

// Handler de eventos MQTT
static void mqtt_event_handler(void *handler_args, esp_event_base_t base,
                              int32_t event_id, void *event_data)
{
    esp_mqtt_event_handle_t event = event_data;
    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT Conectado");
            break;
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "MQTT Desconectado");
            break;
        default:
            break;
    }
}

esp_err_t network_manager_init(void) {
    ESP_LOGI(TAG, "🌐 Inicializando rede...");

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();
    
    wifi_init_sta();
    
    ESP_LOGI(TAG, "Aguardando WiFi...");
    if (wifi_wait_connected(30000) != ESP_OK) {
        ESP_LOGE(TAG, "❌ Falha na conexão WiFi");
        return ESP_FAIL;
    }
    ESP_LOGI(TAG, "✅ WiFi conectado!");

    ESP_LOGI(TAG, "📡 Inicializando MQTT...");
    mqtt_init();
    ESP_ERROR_CHECK(esp_mqtt_client_register_event(mqtt_client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL));
    
    ESP_LOGI(TAG, "Aguardando MQTT...");
    if (mqtt_wait_connected(30000) != ESP_OK) {
        ESP_LOGE(TAG, "❌ Falha na conexão MQTT");
        return ESP_FAIL;
    }
    ESP_LOGI(TAG, "✅ MQTT conectado!");
    
    return ESP_OK;
}

void network_manager_stop(void) {
    ESP_LOGI(TAG, "⏸️ Pausando Wi-Fi/MQTT...");
    if (mqtt_client) {
        esp_mqtt_client_stop(mqtt_client);
    }
    esp_wifi_stop();
}

void network_manager_start(void) {
    ESP_LOGI(TAG, "▶️ Retomando Wi-Fi/MQTT...");
    esp_wifi_start();
    if (mqtt_client) {
        esp_mqtt_client_start(mqtt_client);
    }
}

void network_manager_send_analysis(const char* filename, camera_fb_t* fb, ei_result_t* ei_result) {
    char topic[128];
    char payload[512];
    
    // Tópico para análise de imagem
    snprintf(topic, sizeof(topic), "esp32cam/analysis/%s", DEVICE_ID);
    
    // Payload com informações da análise
    snprintf(payload, sizeof(payload), 
             "{\"filename\":\"%s\",\"size\":%zu,\"width\":%d,\"height\":%d,"
             "\"timestamp\":%lld,\"ei_available\":%s}",
             filename, fb->len, fb->width, fb->height, 
             esp_timer_get_time(), ei_result ? "true" : "false");
    
    esp_mqtt_client_publish(mqtt_client, topic, payload, strlen(payload), 1, 0);
    
    if (ei_result) {
        char preds[256] = "";
        for (int i = 0; i < ei_result->count; i++) {
            char pred_item[64];
            snprintf(pred_item, sizeof(pred_item), "\"%s\":%.4f", 
                     ei_result->predictions[i].label, ei_result->predictions[i].score);
            strcat(preds, pred_item);
            if (i < ei_result->count - 1) {
                strcat(preds, ",");
            }
        }
        snprintf(topic, sizeof(topic), "esp32cam/ei_results/%s", DEVICE_ID);
        snprintf(payload, sizeof(payload), 
                 "{\"filename\":\"%s\",\"latency_ms\":%.2f,\"predictions\":{%s}}",
                 filename, ei_result->latency_ms, preds);
        
        esp_mqtt_client_publish(mqtt_client, topic, payload, strlen(payload), 1, 0);
    }
    
    ESP_LOGI(TAG, "📡 Análise enviada via MQTT para %s", filename);
}

void network_manager_send_dataset_stats(int processed, int total) {
    char topic[128];
    char payload[256];
    
    snprintf(topic, sizeof(topic), "esp32cam/dataset_stats/%s", DEVICE_ID);
    float success_rate = total > 0 ? ((float)processed / total) * 100.0f : 0.0f;
    snprintf(payload, sizeof(payload), 
             "{\"processed\":%d,\"total\":%d,\"success_rate\":%.1f,\"timestamp\":%lld}",
             processed, total, success_rate, esp_timer_get_time());
    
    esp_mqtt_client_publish(mqtt_client, topic, payload, strlen(payload), 1, 0);
    ESP_LOGI(TAG, "📊 Estatísticas do dataset enviadas via MQTT");
}
