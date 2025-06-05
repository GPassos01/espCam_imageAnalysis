idf_component_register(
    SRCS "main.c"
    INCLUDE_DIRS "."
    REQUIRES 
        nvs_flash 
        esp_wifi 
        esp_event 
        esp_netif 
        mqtt 
        spiffs 
        driver
        esp32-camera
        esp_psram
        esp_timer
)

# Todos os módulos modularizados para IC
set(SOURCES main.c 
    model/compare.c 
    model/init_hw.c 
    model/init_net.c 
    model/sensor.c
    model/image_processing.c
    model/mqtt_send.c
)
