#include "compare.h"
#include <stdlib.h>
#include <math.h>
#include <stdio.h>

/**
 * Calcula diferença entre duas imagens JPEG usando algoritmo de 6 etapas
 * 
 * Algoritmo detalhado:
 * 1. Validação rigorosa de entrada (NULL safety)
 * 2. Análise de diferença de tamanho dos arquivos JPEG
 * 3. Amostragem inteligente de 30 pontos distribuídos (início/meio/fim)
 * 4. Detecção de mudanças significativas (threshold > 40)
 * 5. Combinação ponderada: 60% conteúdo + 25% tamanho + 15% mudanças grandes
 * 6. Ajuste de sensibilidade: reduz ruído pequeno, amplifica grandes mudanças
 * 
 * @param img1 Imagem anterior (referência)
 * @param img2 Imagem atual (comparação)
 * @return Diferença normalizada 0.0 (idênticas) a 1.0 (completamente diferentes)
 */
float calculate_image_difference(camera_frame_t *img1, camera_frame_t *img2) {
    // Validações de entrada
    if (!img1 || !img2) {
        return 1.0f;  // Máxima diferença se um dos frames for NULL
    }
    
    if (!img1->buf || !img2->buf) {
        return 1.0f;  // Máxima diferença se buffers forem NULL
    }
    
    if (img1->len == 0 || img2->len == 0) {
        return 1.0f;  // Máxima diferença se comprimentos forem zero
    }
    
    // 1. Verificação básica de tamanho
    float size_ratio = (float)img1->len / img2->len;
    float size_diff = fabs(size_ratio - 1.0f);
    
    // Se diferença de tamanho for extrema, já é mudança significativa
    if (size_ratio < 0.4f || size_ratio > 2.5f) {
        return 0.8f;
    }
    
    // 2. Amostragem melhorada com mais pontos
    size_t sample_points = 30; // Triplicar pontos para melhor detecção
    size_t min_len = (img1->len < img2->len) ? img1->len : img2->len;
    
    // Verificar se temos dados suficientes para análise
    if (min_len < sample_points * 5) {
        return size_diff;
    }
    
    uint64_t diff_sum = 0;
    uint32_t significant_changes = 0;
    size_t valid_samples = 0;
    
    // 3. Amostrar diferentes regiões
    for (size_t i = 0; i < sample_points; i++) {
        size_t pos;
        
        // Distribuir amostras em início, meio e final
        if (i < 10) {
            // Início (cabeçalho JPEG)
            pos = (i * min_len) / 30;
        } else if (i < 20) {
            // Meio (dados principais da imagem)
            pos = min_len/3 + ((i-10) * min_len/3) / 10;
        } else {
            // Final 
            pos = 2*min_len/3 + ((i-20) * min_len/3) / 10;
        }
        
        // Verificar se posição é válida
        if (pos < min_len && pos < img1->len && pos < img2->len) {
            uint8_t byte_diff = abs((int)img1->buf[pos] - (int)img2->buf[pos]);
            diff_sum += byte_diff;
            valid_samples++;
            
            // Contar mudanças significativas (>40 de diferença)
            if (byte_diff > 40) {
                significant_changes++;
            }
        }
    }
    
    // Verificar se tivemos amostras válidas suficientes
    if (valid_samples < sample_points / 2) {
        return size_diff;  // Fallback para diferença de tamanho
    }
    
    // 4. Calcular métricas
    float content_diff = (float)diff_sum / (valid_samples * 255);
    float significant_ratio = (float)significant_changes / valid_samples;
    
    // 5. Combinação inteligente das métricas
    float total_diff = (content_diff * 0.6f) + 
                      (size_diff * 0.25f) + 
                      (significant_ratio * 0.15f);
    
    // 6. Ajuste de sensibilidade
    if (total_diff < 0.08f) {
        total_diff *= 0.7f; // Reduzir ruído pequeno
    } else if (total_diff > 0.4f) {
        total_diff = 0.4f + (total_diff - 0.4f) * 1.3f; // Amplificar mudanças grandes
    }
    
    // Garantir que o resultado esteja no intervalo [0, 1]
    return total_diff > 1.0f ? 1.0f : (total_diff < 0.0f ? 0.0f : total_diff);
} 