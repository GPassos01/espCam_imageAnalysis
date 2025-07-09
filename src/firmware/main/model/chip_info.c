#include "chip_info.h"
#include "esp_system.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_log.h"
#include "esp_mac.h"
#include "esp_psram.h"
#include "soc/soc.h"
#include "soc/rtc.h"
#include <inttypes.h>

static const char *TAG = "CHIP_INFO";

void print_chip_info(void) {
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    
    uint32_t flash_size;
    esp_flash_get_size(NULL, &flash_size);
    
    ESP_LOGI(TAG, "==========================================");
    ESP_LOGI(TAG, "📋 INFORMAÇÕES DETALHADAS DO CHIP ESP32");
    ESP_LOGI(TAG, "==========================================");
    
    // Modelo do chip
    ESP_LOGI(TAG, "🔧 Modelo: %s", 
             (chip_info.model == CHIP_ESP32) ? "ESP32" : "ESP32-S2/S3/C3");
    
    // Revisão do chip
    ESP_LOGI(TAG, "📦 Revisão: %u", chip_info.revision);
    
    // Número de cores
    ESP_LOGI(TAG, "⚙️  Cores: %u", chip_info.cores);
    
    // Frequência da CPU
    rtc_cpu_freq_config_t freq_config;
    rtc_clk_cpu_freq_get_config(&freq_config);
    ESP_LOGI(TAG, "🚀 Frequência CPU: %" PRIu32 " MHz", freq_config.freq_mhz);
    
    // Características do chip
    ESP_LOGI(TAG, "🔋 Características:");
    ESP_LOGI(TAG, "   WiFi: %s", (chip_info.features & CHIP_FEATURE_WIFI_BGN) ? "✓" : "✗");
    ESP_LOGI(TAG, "   Bluetooth: %s", (chip_info.features & CHIP_FEATURE_BT) ? "✓" : "✗");
    ESP_LOGI(TAG, "   BLE: %s", (chip_info.features & CHIP_FEATURE_BLE) ? "✓" : "✗");
    ESP_LOGI(TAG, "   IEEE 802.15.4: %s", (chip_info.features & CHIP_FEATURE_IEEE802154) ? "✓" : "✗");
    ESP_LOGI(TAG, "   Embedded Flash: %s", (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "✓" : "✗");
    ESP_LOGI(TAG, "   Embedded PSRAM: %s", (chip_info.features & CHIP_FEATURE_EMB_PSRAM) ? "✓" : "✗");
    
    // Informações de memória
    ESP_LOGI(TAG, "💾 Memória:");
    ESP_LOGI(TAG, "   Flash: %" PRIu32 " MB", flash_size / (1024 * 1024));
    ESP_LOGI(TAG, "   PSRAM: %s", 
             esp_psram_is_initialized() ? "Inicializado" : "Não disponível");
    
    if (esp_psram_is_initialized()) {
        ESP_LOGI(TAG, "   PSRAM Tamanho: %" PRIu32 " KB", (uint32_t)(esp_psram_get_size() / 1024));
    }
    
    // Heap
    ESP_LOGI(TAG, "   Heap Total: ~%" PRIu32 " KB", (uint32_t)(esp_get_free_heap_size() / 1024 + 100)); // Estimativa
    ESP_LOGI(TAG, "   Heap Livre: %" PRIu32 " KB", (uint32_t)(esp_get_free_heap_size() / 1024));
    ESP_LOGI(TAG, "   Heap Mínimo: %" PRIu32 " KB", (uint32_t)(esp_get_minimum_free_heap_size() / 1024));
    
    // MAC Address
    uint8_t mac[6];
    esp_read_mac(mac, ESP_MAC_WIFI_STA);
    ESP_LOGI(TAG, "🌐 MAC WiFi: %02x:%02x:%02x:%02x:%02x:%02x",
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    
    // Identificação única
    uint64_t chip_id = 0;
    for(int i=0; i<17; i=i+8) {
        chip_id |= ((uint64_t)((uint32_t*)0x3ff5a000)[i/8] << (i * 4));
    }
    ESP_LOGI(TAG, "🆔 Chip ID: %016llx", chip_id);
    
    // Informações do SDK
    ESP_LOGI(TAG, "📚 SDK: %s", esp_get_idf_version());
    
    ESP_LOGI(TAG, "==========================================");
}

esp_chip_model_t get_chip_model(void) {
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    return chip_info.model;
}

uint8_t get_chip_revision(void) {
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    return chip_info.revision;
}

uint8_t get_chip_cores(void) {
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    return chip_info.cores;
}

uint32_t get_chip_features(void) {
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    return chip_info.features;
}

const char* get_chip_model_string(void) {
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    
    switch(chip_info.model) {
        case CHIP_ESP32:
            // Determinar variante baseada na revisão
            if (chip_info.revision >= 3) {
                return "ESP32-D0WD-V3";
            } else if (chip_info.revision >= 1) {
                return "ESP32-D0WD";
            } else {
                return "ESP32-D0WDQ6";
            }
        case CHIP_ESP32S2:
            return "ESP32-S2";
        case CHIP_ESP32S3:
            return "ESP32-S3";
        case CHIP_ESP32C3:
            return "ESP32-C3";
        case CHIP_ESP32H2:
            return "ESP32-H2";
        case CHIP_ESP32C2:
            return "ESP32-C2";
        default:
            return "ESP32-Unknown";
    }
}

bool is_esp32_cam_board(void) {
    // Verificar se é uma placa ESP32-CAM baseada em características típicas
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    
    // ESP32-CAM usa ESP32 original com PSRAM
    if (chip_info.model != CHIP_ESP32) {
        return false;
    }
    
    // Verificar se tem PSRAM (essencial para ESP32-CAM)
    if (!esp_psram_is_initialized()) {
        return false;
    }
    
    // Verificar tamanho da PSRAM (ESP32-CAM típico tem 4MB)
    size_t psram_size = esp_psram_get_size();
    if (psram_size >= 4 * 1024 * 1024) {
        return true;
    }
    
    return false;
} 