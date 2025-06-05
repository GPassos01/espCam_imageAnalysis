#include "compare.h"
#include <stdlib.h>
#include <math.h>
#include <stdio.h>

float calculate_image_difference(camera_frame_t *img1, camera_frame_t *img2) {
    if (!img1 || !img2 || !img1->buf || !img2->buf) {
        return 1.0f;
    }
    float size_ratio = (float)img1->len / img2->len;
    if (size_ratio < 0.5 || size_ratio > 2.0) {
        return 0.9f;
    }
    size_t sample_points = 10; // Menos amostras para mais eficiência
    size_t min_len = (img1->len < img2->len) ? img1->len : img2->len;
    if (min_len < sample_points * 10) {
        return fabs(size_ratio - 1.0f);
    }
    uint64_t diff_sum = 0;
    for (size_t i = 0; i < sample_points; i++) {
        size_t pos = (i * min_len) / sample_points;
        diff_sum += abs((int)img1->buf[pos] - (int)img2->buf[pos]);
    }
    float avg_diff = (float)diff_sum / (sample_points * 255);
    float total_diff = (avg_diff * 0.7f) + (fabs(size_ratio - 1.0f) * 0.3f);
    return total_diff > 1.0f ? 1.0f : total_diff;
} 