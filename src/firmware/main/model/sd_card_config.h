/**
 * @file sd_card_config.h
 * @brief Configurações específicas para SD Card de 8GB no ESP32-CAM
 * 
 * @author Gabriel Passos - UNESP 2025
 */

#ifndef SD_CARD_CONFIG_H
#define SD_CARD_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "driver/gpio.h"
#include "driver/sdmmc_host.h"
#include "esp_vfs_fat.h"

// Configurações dos pinos para ESP32-CAM
#define SD_MMC_CMD      GPIO_NUM_15
#define SD_MMC_CLK      GPIO_NUM_14
#define SD_MMC_D0       GPIO_NUM_2
#define SD_MMC_D1       GPIO_NUM_4
#define SD_MMC_D2       GPIO_NUM_12
#define SD_MMC_D3       GPIO_NUM_13

// Configurações do SD Card
#define SD_MMC_WIDTH    4
#define SD_MMC_FREQ     SDMMC_FREQ_DEFAULT
#define SD_MMC_MAX_FILES 5
#define SD_MMC_ALLOC_UNIT 16 * 1024

// Configurações de montagem
#define SD_MOUNT_POINT "/sdcard"
#define SD_MAX_FILES 5
#define SD_ALLOCATION_UNIT_SIZE 16 * 1024

// Funções de configuração
esp_err_t sd_card_init(void);
esp_err_t sd_card_mount(void);
esp_err_t sd_card_unmount(void);
bool sd_card_is_available(void);

#ifdef __cplusplus
}
#endif

#endif // SD_CARD_CONFIG_H
