/**
 * @file ei_inference.h
 * @brief Integração com Edge Impulse - Interface de inferência
 *
 * Este header define a API mínima para executar inferência local usando
 * um modelo do Edge Impulse em imagens capturadas pela ESP32-CAM.
 * A implementação concreta pode ser baseada em C++/C gerado pelo EI Studio
 * (sdk-c++-inference) ou no WebAssembly/impulse runner quando disponível.
 */

#ifndef EI_INFERENCE_H
#define EI_INFERENCE_H

#include <stdbool.h>
#include <stdint.h>
#include "esp_err.h"
#include "esp_camera.h"

#ifdef __cplusplus
extern "C" {
#endif

// Resultado genérico de inferência (multiclasse)
typedef struct {
    const char* label;      // Nome da classe
    float score;            // Confiança 0..1
} ei_prediction_t;

typedef struct {
    ei_prediction_t* predictions; // Array dinâmico de predições
    int count;                    // Quantidade de predições
    float latency_ms;             // Tempo de inferência em ms
} ei_result_t;

/**
 * Inicializa o mecanismo de inferência do Edge Impulse.
 * Retorna ESP_OK se o modelo foi carregado/configurado com sucesso.
 */
esp_err_t ei_init(void);

/**
 * Executa inferência em um frame JPEG da câmera.
 * Implementações comuns convertem para RGB888/GRAYSCALE conforme o modelo.
 *
 * @param fb Frame buffer JPEG da câmera
 * @param out_result Resultado com classes e scores (alocado internamente)
 * @return ESP_OK em sucesso. Use ei_free_result para liberar.
 */
esp_err_t ei_run_inference(camera_fb_t* fb, ei_result_t* out_result);

/**
 * Libera memória alocada em ei_result_t
 */
void ei_free_result(ei_result_t* result);

#ifdef __cplusplus
}
#endif

#endif // EI_INFERENCE_H


