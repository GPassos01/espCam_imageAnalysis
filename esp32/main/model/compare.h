#ifndef COMPARE_H
#define COMPARE_H
#include <stddef.h>
#include <stdint.h>
#ifdef ESP_PLATFORM
#include "esp_camera.h"
#endif

typedef struct {
    uint8_t *buf;
    size_t len;
    uint32_t timestamp;
    size_t width;
    size_t height;
#ifdef ESP_PLATFORM
    pixformat_t format;
#else
    int format;
#endif
} camera_frame_t;

float calculate_image_difference(camera_frame_t *img1, camera_frame_t *img2);

#endif // COMPARE_H 