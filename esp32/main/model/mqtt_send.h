#ifndef MQTT_SEND_H
#define MQTT_SEND_H

#include "esp_camera.h"
#include "esp_err.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Funções de envio MQTT simplificadas para comparação de imagens
esp_err_t mqtt_send_monitoring_data(float difference, uint32_t image_size, 
                                   uint16_t width, uint16_t height, 
                                   uint8_t format, const char* device_id);

esp_err_t mqtt_send_alert(float difference, const char* alert_type, const char* device_id);

esp_err_t mqtt_send_image_fallback(camera_fb_t *fb, const char* reason, const char* device_id);

#ifdef __cplusplus
}
#endif

#endif // MQTT_SEND_H 