#include "image_processing.h"
#include "esp_log.h"
#include <math.h>
#include <string.h>

static const char *TAG = "IMG_PROC";

// Configurações de processamento
#define WATER_THRESHOLD_LOW   80    // Limiar inferior para água
#define WATER_THRESHOLD_HIGH  140   // Limiar superior para água
#define MIN_WATER_PIXELS      50    // Mínimo de pixels para considerar água válida

static bool convert_jpeg_to_grayscale_roi(camera_fb_t *fb, uint8_t *gray_buffer, 
                                        int roi_x, int roi_y, int roi_w, int roi_h) {
    // Conversão simplificada JPEG -> Grayscale para ROI
    // Para produção, usar biblioteca de decodificação JPEG adequada
    
    if (!fb || !gray_buffer || fb->format != PIXFORMAT_JPEG) {
        ESP_LOGE(TAG, "Parâmetros inválidos para conversão");
        return false;
    }
    
    // Implementação simplificada - assumindo que podemos acessar dados JPEG
    // Na prática, seria necessário decodificar o JPEG primeiro
    size_t roi_size = roi_w * roi_h;
    
    // Simulação de conversão - em implementação real usar decodificador JPEG
    // Aqui fazemos uma estimativa baseada nos bytes JPEG disponíveis
    for (int i = 0; i < roi_size && i < fb->len; i++) {
        // Amostragem distribuída dos bytes JPEG para estimar tons de cinza
        size_t jpeg_pos = (i * fb->len) / roi_size;
        gray_buffer[i] = fb->buf[jpeg_pos];
    }
    
    return true;
}

static int apply_threshold(uint8_t *gray_buffer, uint8_t *binary_buffer, 
                          int width, int height) {
    int water_pixels = 0;
    
    for (int i = 0; i < width * height; i++) {
        // Thresholding adaptativo para água
        if (gray_buffer[i] >= WATER_THRESHOLD_LOW && 
            gray_buffer[i] <= WATER_THRESHOLD_HIGH) {
            binary_buffer[i] = 255; // Água detectada
            water_pixels++;
        } else {
            binary_buffer[i] = 0;   // Não é água
        }
    }
    
    return water_pixels;
}

static float find_water_level_line(uint8_t *binary_buffer, int width, int height) {
    int water_line_y = -1;
    int max_water_in_row = 0;
    
    // Procurar a linha com mais pixels de água (linha da superfície)
    for (int y = 0; y < height; y++) {
        int water_count = 0;
        for (int x = 0; x < width; x++) {
            if (binary_buffer[y * width + x] == 255) {
                water_count++;
            }
        }
        
        // Se há água suficiente nesta linha
        if (water_count > max_water_in_row && water_count > width / 4) {
            max_water_in_row = water_count;
            water_line_y = y;
        }
    }
    
    if (water_line_y >= 0) {
        // Retornar nível como percentual da altura (0-100%)
        return (float)(height - water_line_y) / height * 100.0f;
    }
    
    return -1.0f; // Não encontrou linha d'água clara
}

float process_image_for_water_level(camera_fb_t *fb) {
    if (!fb || !fb->buf || fb->len == 0) {
        ESP_LOGE(TAG, "Frame inválido");
        return -1.0f;
    }
    
    ESP_LOGI(TAG, "Processando imagem: %dx%d, %zu bytes", 
             fb->width, fb->height, fb->len);
    
    // Definir ROI (Region of Interest) para reduzir processamento
    int roi_w = fb->width / 2;   // Processar apenas metade central
    int roi_h = fb->height;
    int roi_x = fb->width / 4;   // Começar no quarto da largura
    int roi_y = 0;
    
    // Alocar buffers de trabalho
    uint8_t *gray_buffer = heap_caps_malloc(roi_w * roi_h, MALLOC_CAP_8BIT);
    uint8_t *binary_buffer = heap_caps_malloc(roi_w * roi_h, MALLOC_CAP_8BIT);
    
    if (!gray_buffer || !binary_buffer) {
        ESP_LOGE(TAG, "Falha ao alocar memória para processamento");
        if (gray_buffer) free(gray_buffer);
        if (binary_buffer) free(binary_buffer);
        return -1.0f;
    }
    
    float water_level = -1.0f;
    
    // 1. Converter JPEG para tons de cinza (ROI)
    if (!convert_jpeg_to_grayscale_roi(fb, gray_buffer, roi_x, roi_y, roi_w, roi_h)) {
        ESP_LOGE(TAG, "Falha na conversão para tons de cinza");
        goto cleanup;
    }
    
    // 2. Aplicar limiarização para detectar água
    int water_pixels = apply_threshold(gray_buffer, binary_buffer, roi_w, roi_h);
    
    ESP_LOGI(TAG, "Pixels de água detectados: %d de %d (%.1f%%)", 
             water_pixels, roi_w * roi_h, 
             (float)water_pixels / (roi_w * roi_h) * 100.0f);
    
    // 3. Verificar se há água suficiente para análise válida
    if (water_pixels < MIN_WATER_PIXELS) {
        ESP_LOGW(TAG, "Poucos pixels de água detectados - possível condição seca");
        water_level = 0.0f;
        goto cleanup;
    }
    
    // 4. Encontrar linha do nível d'água
    water_level = find_water_level_line(binary_buffer, roi_w, roi_h);
    
    if (water_level >= 0) {
        ESP_LOGI(TAG, "Nível d'água detectado: %.1f%%", water_level);
    } else {
        ESP_LOGW(TAG, "Não foi possível determinar nível d'água preciso");
        // Estimar baseado na quantidade de pixels de água
        water_level = (float)water_pixels / (roi_w * roi_h) * 100.0f;
        ESP_LOGI(TAG, "Estimativa baseada em densidade: %.1f%%", water_level);
    }
    
cleanup:
    free(gray_buffer);
    free(binary_buffer);
    
    return water_level;
}

water_analysis_result_t analyze_water_level_advanced(camera_fb_t *fb, float previous_level) {
    water_analysis_result_t result = {0};
    
    result.image_level = process_image_for_water_level(fb);
    result.confidence = 0.0f;
    result.is_valid = false;
    
    if (result.image_level >= 0) {
        result.is_valid = true;
        
        // Calcular confiança baseada em consistência com leitura anterior
        if (previous_level >= 0) {
            float diff = fabs(result.image_level - previous_level);
            // Confiança alta se mudança for gradual (<10%)
            if (diff < 10.0f) {
                result.confidence = 0.9f;
            } else if (diff < 20.0f) {
                result.confidence = 0.7f;
            } else {
                result.confidence = 0.5f; // Mudança abrupta, baixa confiança
            }
        } else {
            result.confidence = 0.8f; // Primeira leitura
        }
        
        // Classificar nível
        if (result.image_level < 20.0f) {
            result.level_status = LEVEL_LOW;
        } else if (result.image_level < 80.0f) {
            result.level_status = LEVEL_NORMAL;
        } else {
            result.level_status = LEVEL_HIGH;
        }
    }
    
    return result;
} 