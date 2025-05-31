idf_component_register(SRCS "main.c"
                    INCLUDE_DIRS "."
                    REQUIRES nvs_flash wifi_provisioning protocomm mqtt esp_timer freertos driver)
