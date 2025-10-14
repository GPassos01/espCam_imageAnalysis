// src/firmware/main/components/analysis_manager/analysis_manager.h

#ifndef ANALYSIS_MANAGER_H
#define ANALYSIS_MANAGER_H

#include "esp_err.h"
#include "model/ei_inference.h"
#include "esp_camera.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Executa a inferência de imagem usando Edge Impulse.
 * 
 * Esta função recebe um buffer de imagem, executa o modelo de ML e retorna
 * os resultados da classificação.
 *
 * @param fb Ponteiro para o frame buffer da câmera contendo a imagem.
 * @param out_result Ponteiro para a estrutura onde os resultados serão armazenados.
 * @return esp_err_t ESP_OK se a inferência for bem-sucedida, erro caso contrário.
 */
esp_err_t analysis_manager_run_inference(camera_fb_t* fb, ei_result_t* out_result);

#ifdef __cplusplus
}
#endif

#endif // ANALYSIS_MANAGER_H
