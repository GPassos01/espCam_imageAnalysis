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
#include <time.h>
#include <sys/time.h>
#include <inttypes.h>

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
    
    // Configura parâmetros do sensor - OTIMIZADO PARA CORES NATURAIS
    s->set_brightness(s, 0);     // -2 a 2 (0=neutro)
    s->set_contrast(s, 0);       // -2 a 2 (0=neutro)
    s->set_saturation(s, -1);    // -2 a 2 (reduzir saturação para cores mais naturais)
    s->set_special_effect(s, 0); // 0=sem efeito (CRÍTICO: evita tints de cor)
    
    // CONFIGURAÇÕES ANTI-ESVERDEADO
    s->set_whitebal(s, 1);       // Habilitar auto white balance
    s->set_awb_gain(s, 1);       // Habilitar ganho AWB
    s->set_wb_mode(s, 1);        // 1=Sunny (melhor para ambientes externos)
    
    // EXPOSIÇÃO E GANHO OTIMIZADOS
    s->set_exposure_ctrl(s, 1);  // Habilitar controle automático de exposição
    s->set_aec2(s, 0);           // AEC2 desabilitado (pode causar artefatos)
    s->set_ae_level(s, 0);       // Nível de exposição neutro
    s->set_gain_ctrl(s, 1);      // Habilitar controle de ganho
    s->set_agc_gain(s, 4);       // Ganho moderado (0-30, evitar 0 que pode causar tints)
    s->set_gainceiling(s, (gainceiling_t)2);  // Limite de ganho moderado
    
    // CORREÇÕES DE IMAGEM
    s->set_bpc(s, 1);            // Habilitar correção de pixel ruim
    s->set_wpc(s, 1);            // Habilitar correção de pixel branco
    s->set_raw_gma(s, 1);        // Habilitar correção gamma
    s->set_lenc(s, 1);           // Habilitar correção de lente
    
    // ORIENTAÇÃO
    s->set_hmirror(s, 0);        // Sem espelhamento horizontal
    s->set_vflip(s, 0);          // Sem inversão vertical
    s->set_dcw(s, 1);            // Habilitar downsize
    s->set_colorbar(s, 0);       // Desabilitar barra de cores de teste
    
    ESP_LOGI(TAG, "✅ Configurações anti-esverdeado aplicadas");
    
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

esp_err_t camera_adjust_color_settings(int wb_mode, int saturation, int gain_level) {
    sensor_t *s = esp_camera_sensor_get();
    if (!s) {
        ESP_LOGE(TAG, "Falha ao obter sensor da câmera");
        return ESP_FAIL;
    }
    
    // Validar parâmetros
    if (wb_mode < 0 || wb_mode > 4) wb_mode = 1; // Default: Sunny
    if (saturation < -2 || saturation > 2) saturation = -1; // Default: reduzido
    if (gain_level < 0 || gain_level > 30) gain_level = 4; // Default: moderado
    
    ESP_LOGI(TAG, "🎨 Ajustando configurações de cor: WB=%d, Sat=%d, Gain=%d", 
             wb_mode, saturation, gain_level);
    
    // Aplicar configurações
    s->set_wb_mode(s, wb_mode);
    s->set_saturation(s, saturation);
    s->set_agc_gain(s, gain_level);
    
    // Forçar recalibração do AWB
    s->set_whitebal(s, 0);
    vTaskDelay(pdMS_TO_TICKS(100));
    s->set_whitebal(s, 1);
    
    ESP_LOGI(TAG, "✅ Configurações de cor aplicadas");
    return ESP_OK;
}

esp_err_t camera_apply_anti_green_settings(bool is_outdoor) {
    sensor_t *s = esp_camera_sensor_get();
    if (!s) {
        ESP_LOGE(TAG, "Falha ao obter sensor da câmera");
        return ESP_FAIL;
    }
    
    ESP_LOGI(TAG, "🌿 Aplicando configurações anti-esverdeado (%s)", 
             is_outdoor ? "externo" : "interno");
    
    if (is_outdoor) {
        // Configurações para ambiente externo
        s->set_wb_mode(s, 1);        // Sunny
        s->set_saturation(s, -1);    // Saturação reduzida
        s->set_agc_gain(s, 3);       // Ganho baixo-moderado
        s->set_ae_level(s, 0);       // Exposição neutra
    } else {
        // Configurações para ambiente interno
        s->set_wb_mode(s, 3);        // Office
        s->set_saturation(s, -2);    // Saturação mais reduzida
        s->set_agc_gain(s, 6);       // Ganho moderado
        s->set_ae_level(s, 1);       // Exposição ligeiramente aumentada
    }
    
    // Configurações comuns anti-esverdeado
    s->set_special_effect(s, 0);     // CRÍTICO: sem efeitos
    s->set_whitebal(s, 1);           // AWB habilitado
    s->set_awb_gain(s, 1);           // Ganho AWB habilitado
    s->set_bpc(s, 1);                // Correção de pixel ruim
    s->set_wpc(s, 1);                // Correção de pixel branco
    s->set_raw_gma(s, 1);            // Correção gamma
    
    ESP_LOGI(TAG, "✅ Configurações anti-esverdeado aplicadas");
    return ESP_OK;
}

esp_err_t camera_warmup_capture(void) {
    ESP_LOGI(TAG, "🔥 Realizando captura de warm-up...");
    
    // Captura descartável para estabilizar sensor
    camera_fb_t *fb = esp_camera_fb_get();
    if (fb) {
        esp_camera_fb_return(fb);
        ESP_LOGI(TAG, "✅ Warm-up concluído");
        return ESP_OK;
    }
    
    ESP_LOGW(TAG, "⚠️ Falha na captura de warm-up");
    return ESP_FAIL;
}

bool detect_green_tint(camera_fb_t *fb) {
    if (!fb || fb->format != PIXFORMAT_JPEG) {
        return false;
    }
    
    // Análise simples baseada no tamanho da imagem
    // Imagens com tint verde tendem a ter compressão diferente
    static uint32_t avg_size = 0;
    static uint32_t sample_count = 0;
    
    if (sample_count < 10) {
        // Construir baseline das primeiras 10 imagens
        avg_size = (avg_size * sample_count + fb->len) / (sample_count + 1);
        sample_count++;
        return false; // Não detectar durante calibração inicial
    }
    
    // Verificar se o tamanho está muito fora do padrão
    // Imagens com tint verde podem ter compressão anômala
    float size_ratio = (float)fb->len / avg_size;
    
    if (size_ratio < 0.7 || size_ratio > 1.4) {
        ESP_LOGD(TAG, "🔍 Possível tint detectado - Tamanho anômalo: %d vs %d (ratio: %.2f)", 
                 fb->len, avg_size, size_ratio);
        return true;
    }
    
    return false;
}

esp_err_t smart_capture_with_correction(camera_fb_t **fb_out) {
    const int MAX_RETRIES = 3;
    bool had_green_tint = false;
    
    for (int retry = 0; retry < MAX_RETRIES; retry++) {
        // Warm-up se for primeira tentativa
        if (retry == 0) {
            camera_warmup_capture();
            vTaskDelay(pdMS_TO_TICKS(200));
        }
        
        camera_fb_t *fb = esp_camera_fb_get();
        if (!fb) {
            ESP_LOGW(TAG, "⚠️ Falha na captura, tentativa %d", retry + 1);
            continue;
        }
        
        // Verificar se há tint verde
        if (!detect_green_tint(fb)) {
            *fb_out = fb;
            ESP_LOGI(TAG, "✅ Captura OK na tentativa %d", retry + 1);
            update_quality_stats(had_green_tint, retry);
            return ESP_OK;
        }
        
        // Se detectou tint, descartar e reconfigurar
        had_green_tint = true;
        esp_camera_fb_return(fb);
        ESP_LOGW(TAG, "🌿 Tint verde detectado, reconfigurando... (tentativa %d)", retry + 1);
        
        // Recalibrar AWB
        sensor_t *s = esp_camera_sensor_get();
        if (s) {
            s->set_whitebal(s, 0);
            vTaskDelay(pdMS_TO_TICKS(100));
            s->set_whitebal(s, 1);
            vTaskDelay(pdMS_TO_TICKS(300));
        }
    }
    
    ESP_LOGE(TAG, "❌ Falha ao obter imagem sem tint após %d tentativas", MAX_RETRIES);
    update_quality_stats(had_green_tint, MAX_RETRIES);
    return ESP_FAIL;
}

void apply_time_based_settings(void) {
    time_t now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);
    
    int hour = timeinfo.tm_hour;
    sensor_t *s = esp_camera_sensor_get();
    
    if (!s) {
        ESP_LOGW(TAG, "⚠️ Sensor não disponível para configurações por horário");
        return;
    }
    
    if (hour >= 6 && hour <= 18) {
        // Período diurno - configurações para luz natural
        s->set_wb_mode(s, 1);      // Sunny
        s->set_saturation(s, -1);  // Saturação reduzida
        s->set_agc_gain(s, 3);     // Ganho baixo
        ESP_LOGI(TAG, "☀️ Configurações diurnas aplicadas (hora: %d)", hour);
    } else {
        // Período noturno - configurações para luz artificial
        s->set_wb_mode(s, 3);      // Office
        s->set_saturation(s, -2);  // Saturação mais reduzida
        s->set_agc_gain(s, 8);     // Ganho aumentado
        ESP_LOGI(TAG, "🌙 Configurações noturnas aplicadas (hora: %d)", hour);
    }
}

void update_quality_stats(bool had_green_tint, int retries) {
    static struct {
        uint32_t total_captures;
        uint32_t green_tint_detected;
        uint32_t retries_needed;
        uint32_t warmup_used;
        float success_rate;
    } stats = {0};
    
    stats.total_captures++;
    if (had_green_tint) stats.green_tint_detected++;
    if (retries > 0) stats.retries_needed++;
    
    stats.success_rate = ((float)(stats.total_captures - stats.green_tint_detected) / stats.total_captures) * 100.0f;
    
    // Log estatísticas a cada 50 capturas
    if (stats.total_captures % 50 == 0) {
        ESP_LOGI(TAG, "📊 Qualidade de Imagem - Taxa de Sucesso: %.1f%% (%" PRIu32 "/%" PRIu32 ")",
                 stats.success_rate, stats.total_captures - stats.green_tint_detected, stats.total_captures);
        ESP_LOGI(TAG, "📊 Estatísticas: Tint Verde: %" PRIu32 ", Retries: %" PRIu32 ", Total: %" PRIu32,
                 stats.green_tint_detected, stats.retries_needed, stats.total_captures);
    }
} 