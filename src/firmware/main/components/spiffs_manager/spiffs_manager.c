// src/firmware/main/components/spiffs_manager/spiffs_manager.c

#include "spiffs_manager.h"
#include "esp_spiffs.h"
#include "esp_log.h"
#include <string.h>
#include <dirent.h>
#include "esp_heap_caps.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "components/analysis_manager/analysis_manager.h"
#include "components/network_manager/network_manager.h"

static const char *TAG = "SPIFFS_MANAGER";

esp_err_t spiffs_manager_init(void) {
    ESP_LOGI(TAG, "Inicializando SPIFFS");

    esp_vfs_spiffs_conf_t conf = {
      .base_path = "/spiffs",
      .partition_label = "storage",
      .max_files = 5,
      .format_if_mount_failed = true
    };

    esp_err_t ret = esp_vfs_spiffs_register(&conf);

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Falha ao montar ou formatar o sistema de arquivos");
        } else if (ret == ESP_ERR_NOT_FOUND) {
            ESP_LOGE(TAG, "Falha ao encontrar a partição SPIFFS");
        } else {
            ESP_LOGE(TAG, "Falha ao inicializar o SPIFFS (%s)", esp_err_to_name(ret));
        }
        return ret;
    }

    size_t total = 0, used = 0;
    ret = esp_spiffs_info(conf.partition_label, &total, &used);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Falha ao obter informações da partição SPIFFS (%s)", esp_err_to_name(ret));
    } else {
        ESP_LOGI(TAG, "Partição SPIFFS: total: %d, usado: %d", total, used);
    }
    
    return ESP_OK;
}

static bool process_single_image(const char* filepath, const char* filename) {
    ESP_LOGI(TAG, "Processando imagem: %s", filename);

    FILE* file = fopen(filepath, "rb");
    if (!file) {
        ESP_LOGE(TAG, "Falha ao abrir arquivo: %s", filepath);
        return false;
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    if (file_size <= 0) {
        ESP_LOGE(TAG, "Arquivo vazio ou inválido: %s", filepath);
        fclose(file);
        return false;
    }
    ESP_LOGI(TAG, "Tamanho do arquivo: %ld bytes", file_size);

    uint8_t* image_buffer = (uint8_t*)heap_caps_malloc(file_size, MALLOC_CAP_SPIRAM);
    if (!image_buffer) {
        ESP_LOGE(TAG, "Falha ao alocar buffer para imagem");
        fclose(file);
        return false;
    }

    size_t bytes_read = fread(image_buffer, 1, file_size, file);
    fclose(file);

    if (bytes_read != file_size) {
        ESP_LOGE(TAG, "Erro na leitura: %zu/%ld bytes", bytes_read, file_size);
        heap_caps_free(image_buffer);
        return false;
    }

    camera_fb_t fb = {
        .buf = image_buffer,
        .len = file_size,
        .width = 640,
        .height = 480,
        .format = PIXFORMAT_JPEG,
        .timestamp = {0}
    };

    ei_result_t ei_result = {0};
    bool success = false;
    if (analysis_manager_run_inference(&fb, &ei_result) == ESP_OK) {
        network_manager_send_analysis(filename, &fb, &ei_result);
        success = true;
    } else {
        ESP_LOGW(TAG, "Análise de ML falhou para %s", filename);
        network_manager_send_analysis(filename, &fb, NULL);
    }
    
    ei_free_result(&ei_result);
    heap_caps_free(image_buffer);
    
    ESP_LOGI(TAG, "Processamento concluído para %s", filename);
    return success;
}

void spiffs_manager_process_dataset(void) {
    ESP_LOGI(TAG, "Processando dataset de imagens no SPIFFS...");

    DIR* dir = opendir("/spiffs");
    if (!dir) {
        ESP_LOGE(TAG, "Falha ao abrir diretório SPIFFS");
        return;
    }

    struct dirent* entry;
    int image_count = 0;
    int processed_count = 0;

    while ((entry = readdir(dir)) != NULL) {
        if (strstr(entry->d_name, ".jpg") || strstr(entry->d_name, ".jpeg")) {
            image_count++;
        }
    }

    if (image_count == 0) {
        ESP_LOGW(TAG, "Nenhuma imagem encontrada no SPIFFS");
        closedir(dir);
        return;
    }
    ESP_LOGI(TAG, "Encontradas %d imagens no dataset", image_count);
    rewinddir(dir);

    while ((entry = readdir(dir)) != NULL) {
        if (strstr(entry->d_name, ".jpg") || strstr(entry->d_name, ".jpeg")) {
            char filepath[512];
            snprintf(filepath, sizeof(filepath), "/spiffs/%s", entry->d_name);
            if (process_single_image(filepath, entry->d_name)) {
                processed_count++;
            }
            vTaskDelay(pdMS_TO_TICKS(100));
        }
    }
    closedir(dir);

    ESP_LOGI(TAG, "Dataset processado: %d/%d imagens com sucesso", processed_count, image_count);
    network_manager_send_dataset_stats(processed_count, image_count);
}
