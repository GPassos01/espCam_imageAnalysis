/**
 * @file sd_card_config.c
 * @brief Implementação das funções de configuração do SD Card
 * 
 * @author Gabriel Passos - UNESP 2025
 */

#include "sd_card_config.h"
#include "esp_log.h"
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include "driver/sdmmc_host.h"
#include "driver/gpio.h"

static const char* TAG = "SD_CARD";
static bool sd_card_mounted = false;
static sdmmc_card_t* sd_card = NULL;

esp_err_t sd_card_init(void) {
    ESP_LOGI(TAG, "🔧 Inicializando SD Card de 8GB...");
    
    // Verificar se os pinos estão disponíveis
    ESP_LOGI(TAG, "🔍 Verificando pinos disponíveis...");
    ESP_LOGI(TAG, "   - CMD: GPIO %d", SD_MMC_CMD);
    ESP_LOGI(TAG, "   - CLK: GPIO %d", SD_MMC_CLK);
    ESP_LOGI(TAG, "   - D0:  GPIO %d", SD_MMC_D0);
    ESP_LOGI(TAG, "   - D1:  GPIO %d", SD_MMC_D1);
    ESP_LOGI(TAG, "   - D2:  GPIO %d", SD_MMC_D2);
    ESP_LOGI(TAG, "   - D3:  GPIO %d", SD_MMC_D3);
    
    // Configurar host SDMMC
    sdmmc_host_t host = SDMMC_HOST_DEFAULT();
    host.max_freq_khz = SD_MMC_FREQ;
    
    // Configurar slot SDMMC usando GPIO matrix
    sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();
    
    // Usar GPIO matrix para configurar pinos
    slot_config.gpio_cmd = SD_MMC_CMD;
    slot_config.gpio_clk = SD_MMC_CLK;
    slot_config.gpio_d0 = SD_MMC_D0;
    slot_config.gpio_d1 = SD_MMC_D1;
    slot_config.gpio_d2 = SD_MMC_D2;
    slot_config.gpio_d3 = SD_MMC_D3;
    slot_config.width = SD_MMC_WIDTH;
    
    // Configurar montagem
    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .max_files = SD_MAX_FILES,
        .allocation_unit_size = SD_ALLOCATION_UNIT_SIZE
    };
    
    ESP_LOGI(TAG, "🚀 Tentando montar SD Card...");
    
    // Montar SD Card
    esp_err_t ret = esp_vfs_fat_sdmmc_mount(SD_MOUNT_POINT, &host, &slot_config, &mount_config, &sd_card);
    
    if (ret == ESP_OK) {
        sd_card_mounted = true;
        ESP_LOGI(TAG, "✅ SD Card inicializado com sucesso!");
        ESP_LOGI(TAG, "📊 Capacidade: %llu MB", 
                 ((uint64_t)sd_card->csd.capacity) * sd_card->csd.sector_size / (1024 * 1024));
        ESP_LOGI(TAG, "🔧 Tipo: SD Card");
        ESP_LOGI(TAG, "📁 Montado em: %s", SD_MOUNT_POINT);
        return ESP_OK;
    } else {
        ESP_LOGW(TAG, "⚠️ Falha ao inicializar SD Card: %s", esp_err_to_name(ret));
        ESP_LOGW(TAG, "💡 Verifique se o SD Card está inserido e os pinos estão corretos");
        sd_card_mounted = false;
        return ret;
    }
}

esp_err_t sd_card_mount(void) {
    if (sd_card_mounted) {
        ESP_LOGW(TAG, "⚠️ SD Card já está montado");
        return ESP_OK;
    }
    
    return sd_card_init();
}

esp_err_t sd_card_unmount(void) {
    if (!sd_card_mounted) {
        ESP_LOGW(TAG, "⚠️ SD Card não está montado");
        return ESP_OK;
    }
    
    esp_err_t ret = esp_vfs_fat_sdmmc_unmount();
    if (ret == ESP_OK) {
        sd_card_mounted = false;
        sd_card = NULL;
        ESP_LOGI(TAG, "✅ SD Card desmontado com sucesso");
    } else {
        ESP_LOGE(TAG, "❌ Falha ao desmontar SD Card: %s", esp_err_to_name(ret));
    }
    
    return ret;
}

bool sd_card_is_available(void) {
    return sd_card_mounted;
}

// Função para obter informações do SD Card
void sd_card_print_info(void) {
    if (!sd_card_mounted || !sd_card) {
        ESP_LOGW(TAG, "⚠️ SD Card não disponível para informações");
        return;
    }
    
    ESP_LOGI(TAG, "📊 Informações do SD Card:");
    ESP_LOGI(TAG, "   - Capacidade: %llu MB", 
             ((uint64_t)sd_card->csd.capacity) * sd_card->csd.sector_size / (1024 * 1024));
    ESP_LOGI(TAG, "   - Tipo: SD Card");
    ESP_LOGI(TAG, "   - Frequência: %d kHz", sd_card->max_freq_khz);
    ESP_LOGI(TAG, "   - Largura: %d bits", sd_card->log_bus_width);
    ESP_LOGI(TAG, "   - Montado em: %s", SD_MOUNT_POINT);
}
