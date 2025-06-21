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

/**
 * @brief Cria mutex da câmera se não existir
 */
void create_camera_mutex(void);

/**
 * @brief Ajusta configurações de cor da câmera para evitar tints esverdeados
 * 
 * @param wb_mode Modo de white balance (0=Auto, 1=Sunny, 2=Cloudy, 3=Office, 4=Home)
 * @param saturation Saturação (-2 a 2)
 * @param gain_level Nível de ganho (0-30)
 * @return esp_err_t ESP_OK se bem-sucedido
 */
esp_err_t camera_adjust_color_settings(int wb_mode, int saturation, int gain_level);

/**
 * @brief Aplica configurações anti-esverdeado baseadas no ambiente
 * 
 * @param is_outdoor true se ambiente externo, false se interno
 * @return esp_err_t ESP_OK se bem-sucedido
 */
esp_err_t camera_apply_anti_green_settings(bool is_outdoor);

/**
 * @brief Realizar captura de warm-up para estabilizar sensor
 * @return ESP_OK se sucesso, ESP_FAIL caso contrário
 */
esp_err_t camera_warmup_capture(void);

/**
 * @brief Detectar automaticamente tint verde em imagem
 * @param fb Frame buffer da imagem
 * @return true se tint verde detectado, false caso contrário
 */
bool detect_green_tint(camera_fb_t *fb);

/**
 * @brief Captura inteligente com correção automática de tint verde
 * @param fb_out Ponteiro para armazenar frame buffer resultante
 * @return ESP_OK se sucesso, ESP_FAIL caso contrário
 */
esp_err_t smart_capture_with_correction(camera_fb_t **fb_out);

/**
 * @brief Aplicar configurações adaptativas baseadas no horário
 */
void apply_time_based_settings(void);

/**
 * @brief Atualizar estatísticas de qualidade da imagem
 * @param had_green_tint true se houve tint verde
 * @param retries número de tentativas necessárias
 */
void update_quality_stats(bool had_green_tint, int retries);

#endif // INIT_HW_H 