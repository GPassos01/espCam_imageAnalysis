#ifndef IMAGE_PROCESSING_H
#define IMAGE_PROCESSING_H
#include "esp_camera.h"
#include <stdbool.h>

typedef enum {
    LEVEL_LOW = 0,
    LEVEL_NORMAL,
    LEVEL_HIGH
} water_level_status_t;

typedef struct {
    float image_level;          // Nível detectado pela câmera (0-100%)
    float confidence;           // Confiança na leitura (0-1)
    bool is_valid;              // Se a leitura é válida
    water_level_status_t level_status; // Classificação do nível
} water_analysis_result_t;

// Função principal de processamento de imagem para IC
float process_image_for_water_level(camera_fb_t *fb);

// Análise avançada com confiança
water_analysis_result_t analyze_water_level_advanced(camera_fb_t *fb, float previous_level);

#endif // IMAGE_PROCESSING_H 