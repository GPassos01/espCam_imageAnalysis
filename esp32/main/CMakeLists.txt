# CMakeLists.txt para sistema de monitoramento com comparação de imagens + WiFi sniffer
idf_component_register(
    SRCS 
        "main.c"
        "model/compare.c"
        "model/mqtt_send.c"
        "model/init_net.c"
        "model/init_hw.c"
        "model/wifi_sniffer.c"
    INCLUDE_DIRS 
        "."
        "model"
    REQUIRES 
        nvs_flash 
        esp_wifi 
        esp_event 
        esp_netif 
        mqtt 
        driver
        esp32-camera
        esp_psram
        esp_timer
        spiffs
    PRIV_REQUIRES json
)
