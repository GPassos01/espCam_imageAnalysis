// src/firmware/main/components/network_manager/network_manager.h

#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

#include "esp_err.h"
#include "model/ei_inference.h"
#include "esp_camera.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Inicializa os serviços de rede (WiFi e MQTT).
 *
 * @return esp_err_t ESP_OK em caso de sucesso, erro caso contrário.
 */
esp_err_t network_manager_init(void);

/**
 * @brief Para os serviços de rede.
 */
void network_manager_stop(void);

/**
 * @brief Inicia/reinicia os serviços de rede.
 */
void network_manager_start(void);

/**
 * @brief Envia os resultados da análise de imagem via MQTT.
 *
 * @param filename Nome do arquivo analisado.
 * @param fb Framebuffer da imagem.
 * @param ei_result Resultados da inferência do Edge Impulse.
 */
void network_manager_send_analysis(const char* filename, camera_fb_t* fb, ei_result_t* ei_result);

/**
 * @brief Envia as estatísticas finais do processamento do dataset via MQTT.
 *
 * @param processed Número de imagens processadas com sucesso.
 * @param total Número total de imagens no dataset.
 */
void network_manager_send_dataset_stats(int processed, int total);


#ifdef __cplusplus
}
#endif

#endif // NETWORK_MANAGER_H
