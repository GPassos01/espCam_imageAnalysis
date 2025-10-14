/**
 * Sistema de Monitoramento ESP32-CAM - Versão Inteligente
 * 
 * Funcionalidades avançadas:
 * - Captura fotos a cada 15 segundos (HVGA 480x320)
 * - Análise de comparação RGB565 por blocos 32x32
 * - Envia APENAS imagens com mudanças significativas (>8%)
 * - Sistema de referência estática para estabilidade
 * - Economia de dados ~90% vs versão simples
 * - WiFi sniffer para monitoramento de tráfego
 * 
 * @version 2.0 - Versão inteligente principal
 * @author Gabriel Passos - UNESP 2025
 */

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "nvs_flash.h"

// Módulos do projeto
#include "model/chip_info.h"
#include "model/ei_inference.h"
#include "components/spiffs_manager/spiffs_manager.h"
#include "components/network_manager/network_manager.h"

static const char *TAG = "APP_MAIN";

void app_main(void)
{
    ESP_LOGI(TAG, "🚀 Iniciando Sistema de Análise de Imagem com ML");
    
    // Inicializar NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    
    // Imprimir informações do chip
    print_chip_info();
    
    // Inicializar rede (WiFi + MQTT)
    if (network_manager_init() != ESP_OK) {
        ESP_LOGE(TAG, "❌ Falha na inicialização da rede. O sistema será reiniciado.");
        vTaskDelay(pdMS_TO_TICKS(5000));
        esp_restart();
    }
    
    // Inicializar Edge Impulse
    if (ei_init() != ESP_OK) {
        ESP_LOGE(TAG, "❌ Falha ao inicializar o Edge Impulse.");
    } else {
        ESP_LOGI(TAG, "🧠 Edge Impulse inicializado com sucesso.");
    }

    // Inicializar SPIFFS
    if (spiffs_manager_init() != ESP_OK) {
        ESP_LOGE(TAG, "❌ Falha na inicialização do SPIFFS. Não é possível processar o dataset.");
    } else {
        ESP_LOGI(TAG, "📁 SPIFFS inicializado - Dataset disponível.");
        
        // Pausar rede para economizar memória durante o processamento
        network_manager_stop();

        // Processar todas as imagens do dataset
        spiffs_manager_process_dataset();

        // Retomar a rede para enviar os últimos dados
        network_manager_start();
    }
    
    ESP_LOGI(TAG, "✅ Sequência de inicialização e processamento concluída.");
    ESP_LOGI(TAG, "💤 Sistema em modo de espera...");
    
    // Loop infinito para manter a task principal viva
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(60000)); // Aguarda 1 minuto
    }
} 