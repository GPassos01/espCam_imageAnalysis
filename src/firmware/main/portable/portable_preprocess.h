#ifndef PORTABLE_PREPROCESS_H
#define PORTABLE_PREPROCESS_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Aplica "fit shortest + crop central" em imagem RGB888
// Converte para formato esperado pelo EI: cada pixel como float contendo 0xRRGGBB
// Se dst_channels == 1, converte para escala de cinza antes de empacotar
// Retorna true em sucesso
bool pp_fit_shortest_center_rgb888_to_u32float_signal(
    const uint8_t *rgb_src,
    int src_w,
    int src_h,
    int dst_w,
    int dst_h,
    int dst_channels,
    float *signal_out);

#ifdef __cplusplus
}
#endif

#endif // PORTABLE_PREPROCESS_H


