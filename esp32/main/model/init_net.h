/**
 * @file init_net.h
 * @brief Interface para inicialização de rede e MQTT
 * 
 * Este módulo fornece funções para:
 * - Inicialização do WiFi
 * - Inicialização do MQTT
 * - Gerenciamento de conexão
 * 
 * @author Gabriel Passos - UNESP 2025
 */
#ifndef INIT_NET_H
#define INIT_NET_H

#include "esp_err.h"
#include "esp_event.h"
#include "mqtt_client.h"

// Handle do cliente MQTT, acessível externamente
extern esp_mqtt_client_handle_t mqtt_client;

/**
 * @brief Inicializa o WiFi em modo station
 * 
 * @return esp_err_t ESP_OK em caso de sucesso
 */
esp_err_t wifi_init_sta(void);

/**
 * @brief Inicializa o cliente MQTT
 * 
 * @return esp_err_t ESP_OK em caso de sucesso
 */
esp_err_t mqtt_init(void);

/**
 * @brief Verifica se o WiFi está conectado
 * 
 * @return true Se conectado
 * @return false Se desconectado
 */
bool wifi_is_connected(void);

/**
 * @brief Verifica se o MQTT está conectado
 * 
 * @return true Se conectado
 * @return false Se desconectado
 */
bool mqtt_is_connected(void);

/**
 * @brief Aguarda conexão WiFi com timeout
 * 
 * @param timeout_ms Timeout em milissegundos
 * @return esp_err_t ESP_OK em caso de sucesso
 */
esp_err_t wifi_wait_connected(uint32_t timeout_ms);

/**
 * @brief Aguarda conexão MQTT com timeout
 * 
 * @param timeout_ms Timeout em milissegundos
 * @return esp_err_t ESP_OK em caso de sucesso
 */
esp_err_t mqtt_wait_connected(uint32_t timeout_ms);

#endif // INIT_NET_H 