#ifndef CHIP_INFO_H
#define CHIP_INFO_H

#include "esp_chip_info.h"
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Imprimir informações detalhadas do chip ESP32
 */
void print_chip_info(void);

/**
 * @brief Obter modelo do chip
 * @return Modelo do chip
 */
esp_chip_model_t get_chip_model(void);

/**
 * @brief Obter revisão do chip
 * @return Número da revisão
 */
uint8_t get_chip_revision(void);

/**
 * @brief Obter número de cores
 * @return Número de cores
 */
uint8_t get_chip_cores(void);

/**
 * @brief Obter características do chip
 * @return Bitmask das características
 */
uint32_t get_chip_features(void);

/**
 * @brief Obter string do modelo do chip
 * @return String com modelo detalhado
 */
const char* get_chip_model_string(void);

/**
 * @brief Verificar se é uma placa ESP32-CAM
 * @return true se for ESP32-CAM, false caso contrário
 */
bool is_esp32_cam_board(void);

#ifdef __cplusplus
}
#endif

#endif // CHIP_INFO_H 