/**
 * @file compare.h
 * @brief Interface para comparação de imagens e detecção de movimento
 * 
 * Este módulo fornece funções para:
 * - Comparação de imagens pixel a pixel com decodificação JPEG
 * - Análise por blocos para otimização de performance
 * - Algoritmo otimizado para HVGA (480x320)
 * 
 * @author Gabriel Passos - UNESP 2025
 */
#ifndef COMPARE_H
#define COMPARE_H

#include "esp_camera.h"

/**
 * @brief Calcula a diferença percentual entre duas imagens
 * 
 * @param frame1 Primeira imagem para comparação
 * @param frame2 Segunda imagem para comparação
 * @return float Percentual de diferença entre as imagens (0.0 a 100.0)
 */
float calculate_image_difference(camera_fb_t* frame1, camera_fb_t* frame2);

/**
 * @brief Libera os buffers de decodificação usados na comparação
 * 
 * Deve ser chamada quando o sistema precisa liberar memória
 * ou ao finalizar o uso do módulo de comparação
 */
void compare_free_buffers(void);

#endif // COMPARE_H 