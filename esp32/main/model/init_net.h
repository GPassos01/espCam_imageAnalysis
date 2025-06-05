#ifndef INIT_NET_H
#define INIT_NET_H
#include "freertos/event_groups.h"
#include "mqtt_client.h"

extern EventGroupHandle_t wifi_event_group;
extern esp_mqtt_client_handle_t mqtt_client;

void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);
void wifi_init_sta(const char* ssid, const char* pass);
void mqtt_init(const char* broker_uri, const char* username, const char* password);

#endif // INIT_NET_H 