#include "portable/portable_preprocess.h"

#include <math.h>
#include <stdbool.h>

static inline uint8_t clamp_u8(int v) {
    if (v < 0) return 0;
    if (v > 255) return 255;
    return (uint8_t)v;
}

bool pp_fit_shortest_center_rgb888_to_u32float_signal(
    const uint8_t *rgb_src,
    int src_w,
    int src_h,
    int dst_w,
    int dst_h,
    int dst_channels,
    float *signal_out)
{
    if (!rgb_src || !signal_out || src_w <= 0 || src_h <= 0 || dst_w <= 0 || dst_h <= 0) {
        return false;
    }

    // Escala (fit shortest)
    float scale_w = (float)dst_w / (float)src_w;
    float scale_h = (float)dst_h / (float)src_h;
    float S = scale_w > scale_h ? scale_w : scale_h;
    float scaled_w = (float)src_w * S;
    float scaled_h = (float)src_h * S;
    float off_x = (scaled_w - (float)dst_w) * 0.5f;
    float off_y = (scaled_h - (float)dst_h) * 0.5f;

    for (int y = 0; y < dst_h; y++) {
        float Ys = ((float)y + off_y) / S;
        int sy = (int)lrintf(Ys);
        if (sy < 0) sy = 0;
        if (sy >= src_h) sy = src_h - 1;
        for (int x = 0; x < dst_w; x++) {
            float Xs = ((float)x + off_x) / S;
            int sx = (int)lrintf(Xs);
            if (sx < 0) sx = 0;
            if (sx >= src_w) sx = src_w - 1;
            size_t idx = ((size_t)sy * (size_t)src_w + (size_t)sx) * 3;
            uint8_t r = rgb_src[idx + 0];
            uint8_t g = rgb_src[idx + 1];
            uint8_t b = rgb_src[idx + 2];
            if (dst_channels == 1) {
                uint8_t yv = (uint8_t)(0.299f * r + 0.587f * g + 0.114f * b);
                r = g = b = yv;
            }
            uint32_t pix = ((uint32_t)r << 16) | ((uint32_t)g << 8) | (uint32_t)b;
            signal_out[(size_t)y * (size_t)dst_w + (size_t)x] = (float)pix;
        }
    }
    return true;
}


