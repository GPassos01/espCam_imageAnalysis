#ifndef INIT_HW_H
#define INIT_HW_H

// Incluir cabeçalhos essenciais para as definições usadas abaixo
#include "esp_err.h"
#include "esp_camera.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

// Mutex para acesso seguro à câmera, usado em main.c
extern SemaphoreHandle_t camera_mutex;

// Funções de inicialização de hardware
esp_err_t init_camera(camera_config_t *camera_config);
esp_err_t init_spiffs(void);
void create_camera_mutex(void);

#endif // INIT_HW_H 