/**
 * @file compare.c
 * @brief Implementação da comparação de imagens otimizada para HVGA
 * 
 * Este módulo implementa:
 * - Comparação de imagens usando RGB565 para eficiência
 * - Análise por blocos 16x16 com amostragem
 * - Algoritmo otimizado para resolução HVGA (480x320)
 * 
 * @author Gabriel Passos - UNESP 2025
 */
#include "compare.h"
#include "esp_log.h"
#include "config.h"
#include "esp_camera.h"
#include "img_converters.h"
#include "esp_heap_caps.h"
#include <stdlib.h>
#include <string.h>

static const char *TAG = "IMG_COMPARE";

/**
 * Algoritmo principal de comparação de imagens
 * Otimizado para HVGA (480x320) com qualidade JPEG 5
 */
float calculate_image_difference(camera_fb_t* frame1, camera_fb_t* frame2) {
    if (!frame1 || !frame2) {
        ESP_LOGE(TAG, "Frames inválidos");
        return 0.0f;
    }
    
    // Verificar se as imagens têm o mesmo tamanho
    if (frame1->width != frame2->width || frame1->height != frame2->height) {
        ESP_LOGE(TAG, "Imagens com tamanhos diferentes: %dx%d vs %dx%d",
                 frame1->width, frame1->height, frame2->width, frame2->height);
        return 50.0f; // Retorna diferença máxima
    }
    
    // Alocar buffers RGB565 (mais eficiente que RGB888)
    size_t rgb565_size = frame1->width * frame1->height * 2;
    uint8_t *rgb565_buf1 = (uint8_t *)heap_caps_malloc(rgb565_size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    uint8_t *rgb565_buf2 = (uint8_t *)heap_caps_malloc(rgb565_size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    
    if (!rgb565_buf1 || !rgb565_buf2) {
        ESP_LOGE(TAG, "Falha ao alocar buffers RGB565");
        if (rgb565_buf1) free(rgb565_buf1);
        if (rgb565_buf2) free(rgb565_buf2);
        
        // Fallback para comparação por tamanho
        float size_diff = abs((int)frame1->len - (int)frame2->len);
        float avg_size = (frame1->len + frame2->len) / 2.0f;
        return (size_diff / avg_size) * 100.0f;
    }
    
    // Decodificar JPEG para RGB565
    bool decoded1 = jpg2rgb565(frame1->buf, frame1->len, rgb565_buf1, JPG_SCALE_NONE);
    bool decoded2 = jpg2rgb565(frame2->buf, frame2->len, rgb565_buf2, JPG_SCALE_NONE);
    
    if (!decoded1 || !decoded2) {
        ESP_LOGE(TAG, "Falha ao decodificar JPEG");
        free(rgb565_buf1);
        free(rgb565_buf2);
        return 0.0f;
    }
    
    // Configurações otimizadas para HVGA
    const int BLOCK_SIZE = 16;           // Blocos de 16x16 pixels
    const int SAMPLE_RATE = 4;           // Analisar 1 a cada 4 pixels
    const int BLOCK_DIFF_THRESHOLD = 40; // Threshold ajustado para HVGA
    
    int blocks_x = frame1->width / BLOCK_SIZE;
    int blocks_y = frame1->height / BLOCK_SIZE;
    int total_blocks = blocks_x * blocks_y;
    int changed_blocks = 0;
    
    // Analisar cada bloco
    for (int by = 0; by < blocks_y; by++) {
        for (int bx = 0; bx < blocks_x; bx++) {
            int block_diff_sum = 0;
            int pixels_compared = 0;
            
            // Comparar pixels dentro do bloco (com amostragem)
            for (int y = 0; y < BLOCK_SIZE; y += SAMPLE_RATE) {
                for (int x = 0; x < BLOCK_SIZE; x += SAMPLE_RATE) {
                    int px = bx * BLOCK_SIZE + x;
                    int py = by * BLOCK_SIZE + y;
                    
                    if (px < frame1->width && py < frame1->height) {
                        int idx = (py * frame1->width + px) * 2; // RGB565 = 2 bytes per pixel
                        
                        // Extrair componentes RGB565 e converter para luminância
                        uint16_t pixel1 = ((uint16_t)rgb565_buf1[idx] << 8) | rgb565_buf1[idx + 1];
                        uint16_t pixel2 = ((uint16_t)rgb565_buf2[idx] << 8) | rgb565_buf2[idx + 1];
                        
                        // RGB565: RRRRRGGGGGGBBBBB
                        int r1 = (pixel1 >> 11) & 0x1F;
                        int g1 = (pixel1 >> 5) & 0x3F;
                        int b1 = pixel1 & 0x1F;
                        
                        int r2 = (pixel2 >> 11) & 0x1F;
                        int g2 = (pixel2 >> 5) & 0x3F;
                        int b2 = pixel2 & 0x1F;
                        
                        // Converter para escala 0-255 e calcular luminância
                        r1 = (r1 << 3) | (r1 >> 2);
                        g1 = (g1 << 2) | (g1 >> 4);
                        b1 = (b1 << 3) | (b1 >> 2);
                        
                        r2 = (r2 << 3) | (r2 >> 2);
                        g2 = (g2 << 2) | (g2 >> 4);
                        b2 = (b2 << 3) | (b2 >> 2);
                        
                        int lum1 = (r1 * 77 + g1 * 150 + b1 * 29) >> 8;
                        int lum2 = (r2 * 77 + g2 * 150 + b2 * 29) >> 8;
                        
                        block_diff_sum += abs(lum1 - lum2);
                        pixels_compared++;
                    }
                }
            }
            
            // Calcular diferença média do bloco
            if (pixels_compared > 0) {
                int avg_diff = block_diff_sum / pixels_compared;
                if (avg_diff > BLOCK_DIFF_THRESHOLD) {
                    changed_blocks++;
                }
            }
        }
    }
    
    // Liberar buffers
    free(rgb565_buf1);
    free(rgb565_buf2);
    
    // Calcular porcentagem de mudança
    float change_percentage = (float)changed_blocks / (float)total_blocks * 100.0f;
    
    ESP_LOGD(TAG, "Blocos analisados: %d, mudados: %d, mudança: %.1f%%",
             total_blocks, changed_blocks, change_percentage);
    
    // Aplicar filtro de ruído (ajustado para HVGA)
    if (change_percentage < 2.0f) {
        return 0.0f; // Ignorar mudanças menores que 2%
    }
    
    return change_percentage;
}

/**
 * Função obsoleta mantida para compatibilidade
 * TODO: Remover quando não for mais necessária
 */
void compare_free_buffers(void) {
    ESP_LOGD(TAG, "compare_free_buffers() - função obsoleta");
}
