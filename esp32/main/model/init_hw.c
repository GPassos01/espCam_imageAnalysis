/**
 * @file init_hw.c
 * @brief Implementação da inicialização de hardware
 * 
 * @author Gabriel Passos - UNESP 2025
 */
#include "init_hw.h"
#include "config.h"
#include "esp_log.h"
#include "esp_system.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "esp_spiffs.h"

static const char *TAG = "INIT_HW";

// Mutex para acesso à câmera
SemaphoreHandle_t camera_mutex = NULL;

esp_err_t camera_init(void) {
    ESP_LOGI(TAG, "Inicializando câmera...");
    
    // Cria mutex para acesso à câmera
    camera_mutex = xSemaphoreCreateMutex();
    if (!camera_mutex) {
        ESP_LOGE(TAG, "Falha ao criar mutex da câmera");
        return ESP_FAIL;
    }
    
    // Obtém configuração da câmera
    camera_config_t camera_config = get_camera_config();
    
    // Inicializa a câmera
    esp_err_t err = esp_camera_init(&camera_config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Falha ao inicializar câmera: %d", err);
        return err;
    }
    
    // Verifica se a câmera está funcionando
    sensor_t *s = esp_camera_sensor_get();
    if (!s) {
        ESP_LOGE(TAG, "Falha ao obter sensor da câmera");
        return ESP_FAIL;
    }
    
    // Configura parâmetros do sensor
    s->set_brightness(s, 0);     // -2,2
    s->set_contrast(s, 0);       // -2,2
    s->set_saturation(s, 0);     // -2,2
    s->set_special_effect(s, 0); // 0,6
    s->set_whitebal(s, 1);       // 0,1
    s->set_awb_gain(s, 1);       // 0,1
    s->set_wb_mode(s, 0);        // 0,4
    s->set_exposure_ctrl(s, 1);  // 0,1
    s->set_aec2(s, 0);           // 0,1
    s->set_gain_ctrl(s, 1);      // 0,1
    s->set_agc_gain(s, 0);       // 0,30
    s->set_gainceiling(s, (gainceiling_t)0);  // 0,6
    s->set_bpc(s, 0);            // 0,1
    s->set_wpc(s, 1);            // 0,1
    s->set_raw_gma(s, 1);        // 0,1
    s->set_lenc(s, 1);           // 0,1
    s->set_hmirror(s, 0);        // 0,1
    s->set_vflip(s, 0);          // 0,1
    s->set_dcw(s, 1);            // 0,1
    s->set_colorbar(s, 0);       // 0,1
    
    ESP_LOGI(TAG, "Câmera inicializada com sucesso");
    return ESP_OK;
}

esp_err_t gpio_init(void) {
    ESP_LOGI(TAG, "Configurando GPIOs...");
    
    // Configura GPIO do flash como saída
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << CAM_PIN_FLASH),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    
    esp_err_t err = gpio_config(&io_conf);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Falha ao configurar GPIO do flash: %d", err);
        return err;
    }
    
    // Desliga o flash inicialmente
    gpio_set_level(CAM_PIN_FLASH, 0);
    
    ESP_LOGI(TAG, "GPIOs configurados com sucesso");
    return ESP_OK;
}

esp_err_t peripherals_init(void) {
    ESP_LOGI(TAG, "Inicializando periféricos...");
    
    // Inicializa GPIOs
    esp_err_t err = gpio_init();
    if (err != ESP_OK) {
        return err;
    }
    
    // Inicializa câmera
    err = camera_init();
    if (err != ESP_OK) {
        return err;
    }
    
    ESP_LOGI(TAG, "Periféricos inicializados com sucesso");
    return ESP_OK;
}

camera_config_t get_camera_config(void) {
    camera_config_t config = {
        .pin_pwdn = CAM_PIN_PWDN,
        .pin_reset = CAM_PIN_RESET,
        .pin_xclk = CAM_PIN_XCLK,
        .pin_sccb_sda = CAM_PIN_SIOD,
        .pin_sccb_scl = CAM_PIN_SIOC,
        
        .pin_d7 = CAM_PIN_D7,
        .pin_d6 = CAM_PIN_D6,
        .pin_d5 = CAM_PIN_D5,
        .pin_d4 = CAM_PIN_D4,
        .pin_d3 = CAM_PIN_D3,
        .pin_d2 = CAM_PIN_D2,
        .pin_d1 = CAM_PIN_D1,
        .pin_d0 = CAM_PIN_D0,
        .pin_vsync = CAM_PIN_VSYNC,
        .pin_href = CAM_PIN_HREF,
        .pin_pclk = CAM_PIN_PCLK,
        
        .xclk_freq_hz = 20000000,
        .ledc_timer = LEDC_TIMER_0,
        .ledc_channel = LEDC_CHANNEL_0,
        
        .pixel_format = PIXEL_FORMAT,
        .frame_size = FRAMESIZE,
        
        .jpeg_quality = JPEG_QUALITY,
        .fb_count = 2,
        .grab_mode = CAMERA_GRAB_WHEN_EMPTY
    };
    
    return config;
}

void create_camera_mutex(void) {
    if (!camera_mutex) camera_mutex = xSemaphoreCreateMutex();
} 