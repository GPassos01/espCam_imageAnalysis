#include "init_net.h"
#include <stdio.h>
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "mqtt_client.h"
#include "freertos/event_groups.h"

static const char *TAG = "INIT_NET";

EventGroupHandle_t wifi_event_group = NULL;
esp_mqtt_client_handle_t mqtt_client = NULL;

void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        esp_wifi_connect();
        xEventGroupClearBits(wifi_event_group, BIT0);
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        xEventGroupSetBits(wifi_event_group, BIT0);
    }
}

void wifi_init_sta(const char* ssid, const char* pass) {
    wifi_event_group = xEventGroupCreate();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);
    esp_event_handler_instance_t instance_any_id, instance_got_ip;
    esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, &instance_any_id);
    esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, &instance_got_ip);
    wifi_config_t wifi_config = {0};
    snprintf((char*)wifi_config.sta.ssid, sizeof(wifi_config.sta.ssid), "%s", ssid);
    snprintf((char*)wifi_config.sta.password, sizeof(wifi_config.sta.password), "%s", pass);
    wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
    esp_wifi_start();
    xEventGroupWaitBits(wifi_event_group, BIT0, pdFALSE, pdFALSE, portMAX_DELAY);
}

void mqtt_init(const char* broker_uri, const char* username, const char* password) {
    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = broker_uri,
        .credentials.username = username,
        .credentials.authentication.password = password,
    };
    mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_start(mqtt_client);
} 