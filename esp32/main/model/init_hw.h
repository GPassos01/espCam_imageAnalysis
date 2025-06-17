/**
 * @file init_hw.h
 * @brief Interface para inicialização de hardware
 * 
 * Este módulo fornece funções para:
 * - Inicialização da câmera
 * - Configuração de GPIOs
 * - Inicialização de periféricos
 * 
 * @author Gabriel Passos - UNESP 2025
 */
#ifndef INIT_HW_H
#define INIT_HW_H

// Incluir cabeçalhos essenciais para as definições usadas abaixo
#include "esp_err.h"
#include "esp_camera.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

// Mutex para acesso seguro à câmera, usado em main.c
extern SemaphoreHandle_t camera_mutex;

/**
 * @brief Inicializa a câmera com configurações padrão
 * 
 * @return esp_err_t ESP_OK em caso de sucesso
 */
esp_err_t camera_init(void);

/**
 * @brief Configura os GPIOs necessários
 * 
 * @return esp_err_t ESP_OK em caso de sucesso
 */
esp_err_t gpio_init(void);

/**
 * @brief Inicializa todos os periféricos necessários
 * 
 * @return esp_err_t ESP_OK em caso de sucesso
 */
esp_err_t peripherals_init(void);

/**
 * @brief Obtém a configuração da câmera
 * 
 * @return camera_config_t Configuração da câmera
 */
camera_config_t get_camera_config(void);

#endif // INIT_HW_H 