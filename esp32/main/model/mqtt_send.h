#ifndef MQTT_SEND_H
#define MQTT_SEND_H
#include "esp_err.h"
#include "esp_camera.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Tópicos MQTT otimizados para IC
#define TOPIC_WATER_LEVEL     "ic/water_level/data"
#define TOPIC_ALERTS          "ic/alerts"
#define TOPIC_SYSTEM_STATUS   "ic/system/status"
#define TOPIC_IMAGE_METADATA  "ic/image/metadata"
#define TOPIC_IMAGE_DATA      "ic/image/data"

// Funções principais para comunicação MQTT da IC
esp_err_t mqtt_send_water_level_data(float image_level, float sensor_level, 
                                   float confidence, const char* device_id);

esp_err_t mqtt_send_alert(float level, const char* alert_type, const char* device_id);

esp_err_t mqtt_send_system_status(uint32_t uptime, size_t free_heap, 
                                size_t free_psram, const char* device_id);

esp_err_t mqtt_send_image_fallback(camera_fb_t *fb, const char* reason, const char* device_id);

#endif // MQTT_SEND_H 