#ifndef INIT_NET_H
#define INIT_NET_H
#include "freertos/event_groups.h"
#include "mqtt_client.h"

extern EventGroupHandle_t wifi_event_group;

void wifi_init_sta(const char* ssid, const char* pass);
void mqtt_init(const char* broker_uri, const char* username, const char* password);

#endif // INIT_NET_H 