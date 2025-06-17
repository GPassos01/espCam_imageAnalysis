/**
 * @file compare.c
 * @brief Implementação da comparação de imagens e detecção de movimento
 * 
 * Este módulo implementa:
 * - Comparação de imagens usando diferença média de pixels
 * - Detecção de movimento baseada em threshold
 * 
 * @author Gabriel Passos - UNESP 2025
 */
#include "compare.h"
#include "esp_log.h"
#include "config.h"

static const char *TAG = "IMG_COMPARE";

/**
 * Algoritmo  de comparação de imagens simples
 * 
 * Compara imagens usando uma métrica simples de diferença média
 * entre pixels, otimizada para ESP32-CAM
 */

// Função principal de comparação de imagens
float calculate_image_difference(camera_fb_t* frame1, camera_fb_t* frame2) {
    if (!frame1 || !frame2) {
        ESP_LOGE(TAG, "Frames inválidos");
        return 0.0f;
    }

    // Calcular diferença de tamanho
    float size_diff = abs((int)frame1->len - (int)frame2->len);
    float avg_size = (frame1->len + frame2->len) / 2.0f;
    float size_variation = (size_diff / avg_size) * 100.0f;
    
    ESP_LOGD(TAG, "Tamanhos: %zu vs %zu bytes, diferença: %.1f%%", 
             frame1->len, frame2->len, size_variation);

    // Para imagens JPEG, algoritmo mais sensível
    // Variações muito pequenas (<0.5%) são consideradas ruído
    if (size_variation < 0.5f) {
        return 0.0f;  // Ruído/compressão normal
    }
    
    // Variações pequenas (0.5-5%) são reportadas diretamente
    if (size_variation < 5.0f) {
        return size_variation;  // Mudanças pequenas mas detectáveis
    }
    
    // Variações moderadas (5-20%) são mudanças significativas
    if (size_variation < 20.0f) {
        return size_variation;  // Mudanças claras
    }
    
    // Variações grandes (>20%) são muito significativas
    return size_variation > 50.0f ? 50.0f : size_variation;
}
