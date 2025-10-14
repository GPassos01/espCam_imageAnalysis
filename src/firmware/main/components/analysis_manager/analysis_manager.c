// src/firmware/main/components/analysis_manager/analysis_manager.c

#include "analysis_manager.h"
#include "esp_log.h"
#include "esp_task_wdt.h"
#include "esp_heap_caps.h"
#include "model/ei_inference.h"

static const char *TAG = "ANALYSIS_MANAGER";

esp_err_t analysis_manager_run_inference(camera_fb_t* fb, ei_result_t* out_result) {
    if (!fb || !out_result) {
        return ESP_ERR_INVALID_ARG;
    }

    // Desabilitar watchdog durante inferência para evitar timeout
    ESP_LOGI(TAG, "⏱️ Desabilitando watchdog para inferência...");
    esp_task_wdt_deinit();

    ESP_LOGI(TAG, "💾 Memória antes da inferência - Heap: %lu KB, PSRAM: %lu KB",
             (unsigned long)(esp_get_free_heap_size() / 1024),
             (unsigned long)(heap_caps_get_free_size(MALLOC_CAP_SPIRAM) / 1024));

    esp_err_t err = ei_run_inference(fb, out_result);

    ESP_LOGI(TAG, "💾 Memória após a inferência - Heap: %lu KB, PSRAM: %lu KB",
             (unsigned long)(esp_get_free_heap_size() / 1024),
             (unsigned long)(heap_caps_get_free_size(MALLOC_CAP_SPIRAM) / 1024));

    // Reabilitar watchdog
    ESP_LOGI(TAG, "⏱️ Reabilitando watchdog...");
    esp_task_wdt_config_t wdt_cfg = {
        .timeout_ms = 8000,
        .idle_core_mask = (1 << 0),
        .trigger_panic = true
    };
    esp_task_wdt_init(&wdt_cfg);

    if (err == ESP_OK) {
        ESP_LOGI(TAG, "🧠 Inferência concluída com sucesso (%.2f ms)", out_result->latency_ms);
    } else {
        ESP_LOGE(TAG, "❌ Falha na execução da inferência");
    }

    return err;
}
