/**
 * @file test_dataset.h
 * @brief Header para sistema de teste com dataset local
 */

#ifndef TEST_DATASET_H
#define TEST_DATASET_H

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Inicializa SPIFFS para leitura de imagens
 */
esp_err_t init_spiffs(void);

/**
 * Testa múltiplas imagens do dataset
 */
void test_dataset_images(void);

/**
 * Testa uma imagem específica
 * @param category Categoria esperada (flood, high, low, medium)
 * @param filename Nome do arquivo na SPIFFS
 */
void test_single_image(const char* category, const char* filename);

#ifdef __cplusplus
}
#endif

#endif // TEST_DATASET_H
