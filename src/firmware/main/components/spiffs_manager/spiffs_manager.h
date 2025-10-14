// src/firmware/main/components/spiffs_manager/spiffs_manager.h

#ifndef SPIFFS_MANAGER_H
#define SPIFFS_MANAGER_H

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Inicializa o sistema de arquivos SPIFFS.
 *
 * @return esp_err_t ESP_OK em caso de sucesso, erro caso contrário.
 */
esp_err_t spiffs_manager_init(void);

/**
 * @brief Processa todas as imagens de um dataset no SPIFFS.
 *
 * Varre o diretório, encontra arquivos de imagem e os processa um a um,
 * executando a análise de ML e enviando os resultados.
 */
void spiffs_manager_process_dataset(void);

#ifdef __cplusplus
}
#endif

#endif // SPIFFS_MANAGER_H
