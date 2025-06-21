/**
 * @file init_net.c
 * @brief Implementação da inicialização de rede e MQTT
 * 
 * @author Gabriel Passos - UNESP 2025
 */
#include <string.h>
#include <stdint.h>
#include <stddef.h>

// FreeRTOS
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/semphr.h"

// ESP-IDF
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

// LwIP
#include "lwip/err.h"
#include "lwip/sys.h"

// MQTT
#include "mqtt_client.h"

// Módulo
#include "init_net.h"
#include "config.h"

#define WIFI_MAXIMUM_RETRY 5
#define MQTT_MAXIMUM_RETRY 5

static const char *TAG = "INIT_NET";

// Event group para sinalizar eventos de Wi-Fi
static EventGroupHandle_t s_wifi_event_group;
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

static int s_retry_num = 0;
static int mqtt_retry_num = 0;

// Handle do cliente MQTT, acessível externamente por outros módulos
esp_mqtt_client_handle_t mqtt_client = NULL;

// Event group para MQTT
static EventGroupHandle_t s_mqtt_event_group;
#define MQTT_CONNECTED_BIT BIT0
#define MQTT_FAIL_BIT      BIT1

static void event_handler(void* arg, esp_event_base_t event_base,
                          int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < WIFI_MAXIMUM_RETRY) {
        esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "Tentando reconectar ao Wi-Fi (tentativa %d)", s_retry_num);
        } else {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
            ESP_LOGE(TAG, "Falha ao conectar ao Wi-Fi após %d tentativas", WIFI_MAXIMUM_RETRY);
        }
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "IP obtido: " IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;

    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT Conectado");
            mqtt_retry_num = 0;
            xEventGroupSetBits(s_mqtt_event_group, MQTT_CONNECTED_BIT);
            break;
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "MQTT Desconectado");
            xEventGroupClearBits(s_mqtt_event_group, MQTT_CONNECTED_BIT);
            break;
        case MQTT_EVENT_ERROR:
            ESP_LOGE(TAG, "MQTT Erro");
            if (mqtt_retry_num < MQTT_MAXIMUM_RETRY) {
                ESP_LOGI(TAG, "Tentando reconectar MQTT (tentativa %d)", mqtt_retry_num + 1);
                mqtt_retry_num++;
                esp_mqtt_client_reconnect(client);
            } else {
                xEventGroupSetBits(s_mqtt_event_group, MQTT_FAIL_BIT);
                ESP_LOGE(TAG, "Falha ao conectar MQTT após %d tentativas", MQTT_MAXIMUM_RETRY);
            }
            break;
        default:
            ESP_LOGI(TAG, "Outro evento MQTT: %d", event->event_id);
            break;
    }
}

esp_err_t wifi_init_sta(void) {
    s_wifi_event_group = xEventGroupCreate();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL));
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
            .pmf_cfg = {
                .capable = true,
                .required = false
            },
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_LOGI(TAG, "wifi_init_sta finalizado. Aguardando conexão...");
    return ESP_OK;
}

esp_err_t mqtt_init(void) {
    s_mqtt_event_group = xEventGroupCreate();
    esp_mqtt_client_config_t mqtt_cfg = {
        .broker = {
            .address = {
                .uri = MQTT_BROKER_URI
            }
        },
        .credentials = {
            .username = MQTT_USERNAME,
            .authentication = {
                .password = MQTT_PASSWORD
            }
        }
    };
    mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
    if (!mqtt_client) {
        ESP_LOGE(TAG, "Falha ao inicializar cliente MQTT");
        return ESP_FAIL;
    }
    ESP_ERROR_CHECK(esp_mqtt_client_register_event(mqtt_client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL));
    ESP_ERROR_CHECK(esp_mqtt_client_start(mqtt_client));
    return ESP_OK;
}

bool wifi_is_connected(void) {
    return (xEventGroupGetBits(s_wifi_event_group) & WIFI_CONNECTED_BIT) != 0;
}

bool mqtt_is_connected(void) {
    return (xEventGroupGetBits(s_mqtt_event_group) & MQTT_CONNECTED_BIT) != 0;
}

esp_err_t wifi_wait_connected(uint32_t timeout_ms) {
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
            WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
            pdFALSE,
            pdFALSE,
                                          pdMS_TO_TICKS(timeout_ms));

    if (bits & WIFI_CONNECTED_BIT) {
        return ESP_OK;
    } else if (bits & WIFI_FAIL_BIT) {
        return ESP_FAIL;
    } else {
        return ESP_ERR_TIMEOUT;
    }
}

esp_err_t mqtt_wait_connected(uint32_t timeout_ms) {
    EventBits_t bits = xEventGroupWaitBits(s_mqtt_event_group,
                                          MQTT_CONNECTED_BIT | MQTT_FAIL_BIT,
                                          pdFALSE,
                                          pdFALSE,
                                          pdMS_TO_TICKS(timeout_ms));
    
    if (bits & MQTT_CONNECTED_BIT) {
        return ESP_OK;
    } else if (bits & MQTT_FAIL_BIT) {
        return ESP_FAIL;
    } else {
        return ESP_ERR_TIMEOUT;
    }
} 