#ifndef WIFI_SNIFFER_H
#define WIFI_SNIFFER_H

#include <stdint.h>
#include <stdbool.h>
#include "esp_wifi.h"
#include "esp_err.h"
#include "mqtt_client.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Estrutura para estatísticas de tráfego WiFi
 */
typedef struct {
    uint32_t total_packets;          // Total de pacotes capturados
    uint32_t mqtt_packets;           // Pacotes MQTT identificados
    uint64_t total_bytes;            // Total de bytes trafegados
    uint64_t mqtt_bytes;             // Bytes MQTT específicos
    uint32_t image_packets;          // Pacotes de envio de imagem
    uint64_t image_bytes;            // Bytes de envio de imagem
    uint32_t start_time;             // Timestamp de início
    bool active;                     // Status do sniffer
} wifi_traffic_stats_t;

/**
 * Callback para processamento de pacotes capturados
 */
typedef void (*packet_callback_t)(const uint8_t *buffer, uint32_t len, const wifi_promiscuous_pkt_t *pkt);

/**
 * Inicializar sistema de WiFi sniffing
 * @param channel Canal WiFi para monitorar (1-13)
 * @return ESP_OK se inicializado com sucesso
 */
esp_err_t wifi_sniffer_init(uint8_t channel);

/**
 * Iniciar captura de pacotes WiFi
 * @return ESP_OK se iniciado com sucesso
 */
esp_err_t wifi_sniffer_start(void);

/**
 * Parar captura de pacotes WiFi
 * @return ESP_OK se parado com sucesso
 */
esp_err_t wifi_sniffer_stop(void);

/**
 * Deinicializar sistema de sniffing
 * @return ESP_OK se deinicializado com sucesso
 */
esp_err_t wifi_sniffer_deinit(void);

/**
 * Marcar início de transmissão de imagem (para medição específica)
 */
void wifi_sniffer_mark_image_start(void);

/**
 * Marcar fim de transmissão de imagem
 */
void wifi_sniffer_mark_image_end(void);

/**
 * Obter estatísticas atuais de tráfego
 * @param stats Ponteiro para estrutura de estatísticas
 */
void wifi_sniffer_get_stats(wifi_traffic_stats_t *stats);

/**
 * Resetar contadores de estatísticas
 */
void wifi_sniffer_reset_stats(void);

/**
 * Imprimir estatísticas de tráfego
 */
void wifi_sniffer_print_stats(void);

/**
 * Definir callback personalizado para processamento de pacotes
 * @param callback Função callback
 */
void wifi_sniffer_set_callback(packet_callback_t callback);

/**
 * Verificar se o sniffer está ativo
 * @return true se ativo, false caso contrário
 */
bool wifi_sniffer_is_active(void);

/**
 * Enviar estatísticas do sniffer via MQTT
 * @param mqtt_client Cliente MQTT ativo
 * @param device_id ID do dispositivo
 * @return ESP_OK se enviado com sucesso
 */
esp_err_t wifi_sniffer_send_mqtt_stats(esp_mqtt_client_handle_t mqtt_client, const char* device_id);

#ifdef __cplusplus
}
#endif

#endif // WIFI_SNIFFER_H 