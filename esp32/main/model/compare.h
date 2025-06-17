/**
 * @file compare.h
 * @brief Interface para comparação de imagens e detecção de movimento
 * 
 * Este módulo fornece funções para:
 * - Comparação de imagens usando diferença média de pixels
 * - Detecção de movimento baseada em threshold
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
 * @return float Percentual de diferença entre as imagens (0.0 a 1.0)
 */
float calculate_image_difference(camera_fb_t* frame1, camera_fb_t* frame2);

/**
 * @brief Detecta movimento entre duas imagens
 * 
 * @param frame1 Primeira imagem
 * @param frame2 Segunda imagem
 * @param threshold Threshold para detecção (0.0 a 1.0)
 */
bool detect_motion(camera_fb_t* frame1, camera_fb_t* frame2, float threshold);

#endif // COMPARE_H 