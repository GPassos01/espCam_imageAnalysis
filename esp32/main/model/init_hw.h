#ifndef INIT_HW_H
#define INIT_HW_H
#include "esp_camera.h"
#include "freertos/semphr.h"
#include "esp_err.h"

extern SemaphoreHandle_t camera_mutex;

esp_err_t init_camera(camera_config_t *camera_config);
esp_err_t init_spiffs(void);
void create_camera_mutex(void);

#endif // INIT_HW_H 