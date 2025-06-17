/**
 * @file mqtt_send.h
 * @brief Interface para envio de dados via MQTT
 * 
 * Este módulo fornece funções para:
 * - Envio de imagens em chunks
 * - Envio de dados de monitoramento
 * - Envio de alertas
 * - Envio de estatísticas
 * 
 * @author Gabriel Passos - UNESP 2025
 */
#ifndef MQTT_SEND_H
#define MQTT_SEND_H

#include "esp_camera.h"
#include "esp_err.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Envia uma imagem via MQTT.
 * A imagem é enviada em chunks para o tópico definido em `MQTT_TOPIC_IMAGE`.
 * 
 * @param fb Ponteiro para o frame buffer da câmera.
 * @param topic Tópico MQTT para publicar a imagem.
 * @return esp_err_t 
 */
esp_err_t mqtt_send_image(camera_fb_t* fb, const char* topic);

/**
 * @brief Envia dados de monitoramento (heap, psram, uptime).
 * 
 * @param free_heap Heap livre.
 * @param free_psram PSRAM livre.
 * @param uptime Uptime em segundos.
 * @return esp_err_t 
 */
esp_err_t mqtt_send_monitoring(uint32_t free_heap, uint32_t free_psram, uint32_t uptime);

/**
 * @brief Envia um alerta de detecção de mudança.
 * 
 * @param difference_percent Percentual de diferença detectado.
 * @param frame Frame buffer da imagem que gerou o alerta (pode ser NULL).
 * @return esp_err_t 
 */
esp_err_t mqtt_send_alert(float difference_percent, camera_fb_t* frame);

/**
 * @brief Envia estatísticas do WiFi sniffer.
 * 
 * @param total_packets Total de pacotes capturados.
 * @param total_bytes Total de bytes capturados.
 * @param mqtt_packets Pacotes MQTT capturados.
 * @param mqtt_bytes Bytes MQTT capturados.
 * @return esp_err_t 
 */
esp_err_t mqtt_send_sniffer_stats(uint32_t total_packets, uint32_t total_bytes,
                                 uint32_t mqtt_packets, uint32_t mqtt_bytes);

/**
 * @brief Envia dados detalhados de monitoramento de imagem.
 * 
 * @param difference Diferença percentual entre imagens.
 * @param image_size Tamanho da imagem em bytes.
 * @param width Largura da imagem.
 * @param height Altura da imagem.
 * @param format Formato da imagem.
 * @param device_id ID do dispositivo.
 * @return esp_err_t 
 */
esp_err_t mqtt_send_monitoring_data(float difference, uint32_t image_size, 
                                   uint16_t width, uint16_t height, 
                                   uint8_t format, const char* device_id);

#ifdef __cplusplus
}
#endif

#endif // MQTT_SEND_H 