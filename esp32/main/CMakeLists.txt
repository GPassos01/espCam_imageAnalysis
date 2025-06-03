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
