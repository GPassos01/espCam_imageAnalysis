#include "init_hw.h"
#include <stdio.h>
#include "esp_camera.h"
#include "esp_log.h"
#include "esp_spiffs.h"
#include "esp_heap_caps.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

static const char *TAG = "INIT_HW";

SemaphoreHandle_t camera_mutex = NULL;

esp_err_t init_camera(camera_config_t *camera_config) {
    ESP_LOGI(TAG, "Inicializando câmera ESP32-CAM...");
    
    size_t psram_size = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
    size_t total_heap = heap_caps_get_free_size(MALLOC_CAP_8BIT);
    size_t internal_heap = heap_caps_get_free_size(MALLOC_CAP_INTERNAL);
    
    ESP_LOGI(TAG, "PSRAM disponível: %zu bytes", psram_size);
    ESP_LOGI(TAG, "Heap total disponível: %zu bytes", total_heap);
    ESP_LOGI(TAG, "Heap interno disponível: %zu bytes", internal_heap);
    
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << 4),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&io_conf);
    
    esp_err_t ret = esp_camera_init(camera_config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Falha na inicialização da câmera: 0x%x", ret);
    } else {
        ESP_LOGI(TAG, "Câmera inicializada com sucesso");
    }
    
    return ret;
}

esp_err_t init_spiffs(void) {
    ESP_LOGI(TAG, "Inicializando SPIFFS...");
    esp_vfs_spiffs_conf_t conf = {
        .base_path = "/spiffs",
        .partition_label = NULL,
        .max_files = 5,
        .format_if_mount_failed = true
    };
    esp_err_t ret = esp_vfs_spiffs_register(&conf);
    if (ret != ESP_OK) return ret;
    size_t total = 0, used = 0;
    ret = esp_spiffs_info(NULL, &total, &used);
    return ret;
}

void create_camera_mutex(void) {
    if (!camera_mutex) camera_mutex = xSemaphoreCreateMutex();
} 