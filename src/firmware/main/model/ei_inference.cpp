// Implementação real da API de inferência Edge Impulse

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "esp_err.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "esp_heap_caps.h"
#include "esp_camera.h"
#include "img_converters.h"

#include "edge-impulse-sdk/classifier/ei_run_classifier.h"
#include "edge-impulse-sdk/dsp/numpy.hpp"
#include "model-parameters/model_metadata.h"
#include "model/ei_inference.h"
#include "portable/portable_preprocess.h"

static const char *TAG = "EI_INFER";

// Parser simples para dimensões de JPEG (SOF0..SOF3)
static bool jpeg_get_size(const uint8_t *buf, size_t len, int *out_w, int *out_h) {
    if (!buf || len < 4) return false;
    size_t i = 0;
    while (i + 9 < len) {
        if (buf[i] == 0xFF) {
            uint8_t marker = buf[i + 1];
            if (marker >= 0xC0 && marker <= 0xC3) {
                if (i + 8 < len) {
                    int h = (buf[i + 5] << 8) | buf[i + 6];
                    int w = (buf[i + 7] << 8) | buf[i + 8];
                    if (w > 0 && h > 0) {
                        if (out_w) *out_w = w;
                        if (out_h) *out_h = h;
                        return true;
                    }
                }
                return false;
            }
        }
        i++;
    }
    return false;
}

// Redimensionamento nearest-neighbor de RGB888
static __attribute__((unused)) void resize_rgb888_nn(const uint8_t *src, int src_w, int src_h,
                             uint8_t *dst, int dst_w, int dst_h) {
    for (int y = 0; y < dst_h; y++) {
        int sy = (y * src_h) / dst_h;
        const uint8_t *src_row = src + (size_t)sy * src_w * 3;
        uint8_t *dst_row = dst + (size_t)y * dst_w * 3;
        for (int x = 0; x < dst_w; x++) {
            int sx = (x * src_w) / dst_w;
            const uint8_t *sp = src_row + (size_t)sx * 3;
            uint8_t *dp = dst_row + (size_t)x * 3;
            dp[0] = sp[0];
            dp[1] = sp[1];
            dp[2] = sp[2];
        }
    }
}

// Empacota RGB888 em float com valor 0xRRGGBB (como esperado pelo DSP de imagem)
static __attribute__((unused)) void pack_rgb_to_signal_u32_as_float(const uint8_t *rgb, int w, int h, float *out) {
    size_t n = (size_t)w * (size_t)h;
    for (size_t i = 0; i < n; i++) {
        uint8_t r = rgb[i * 3 + 0];
        uint8_t g = rgb[i * 3 + 1];
        uint8_t b = rgb[i * 3 + 2];
        uint32_t pixel = ((uint32_t)r << 16) | ((uint32_t)g << 8) | (uint32_t)b;
        out[i] = (float)pixel;
    }
}

extern "C" esp_err_t ei_init(void) {
    // Para modelos EON/TFLM, a inicialização é feita sob demanda em run_classifier
    ESP_LOGI(TAG, "Inicialização EI pronta (EON/TFLM)");
    return ESP_OK;
}

extern "C" esp_err_t ei_run_inference(camera_fb_t* fb, ei_result_t* out_result) {
    if (!out_result || !fb || !fb->buf || fb->len == 0) {
        return ESP_ERR_INVALID_ARG;
    }

    // Dimensões da entrada do modelo
    const int in_w = EI_CLASSIFIER_INPUT_WIDTH;
    const int in_h = EI_CLASSIFIER_INPUT_HEIGHT;
    const int in_ch = EI_CLASSIFIER_INPUT_CHANNELS; // esperado 1 (Gray) ou 3 (RGB)

    if (in_ch != 1 && in_ch != 3) {
        ESP_LOGE(TAG, "Canais de entrada não suportados: %d", in_ch);
        return ESP_ERR_NOT_SUPPORTED;
    }

    int src_w = fb->width;
    int src_h = fb->height;
    if (fb->format == PIXFORMAT_JPEG) {
        // Sempre ler dimensões reais do JPEG para evitar subalocação e corrupção de heap
        int jw = 0, jh = 0;
        if (!jpeg_get_size(fb->buf, fb->len, &jw, &jh)) {
            ESP_LOGE(TAG, "Falha ao obter dimensões do JPEG");
            return ESP_FAIL;
        }
        src_w = jw;
        src_h = jh;
    }

    // Decodificando para RGB888 full size; usar dimensões reais
    int sw = src_w;
    int sh = src_h;

    // Decodificar JPEG -> RGB888 (sem risco de alinhamento do RGB565)
    // Observação: com Wi-Fi pausado, há memória suficiente para 640x480x3
    size_t rgb_size = (size_t)src_w * (size_t)src_h * 3;
    uint8_t *rgb_full = (uint8_t*)heap_caps_malloc(rgb_size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    if (!rgb_full) {
        ESP_LOGE(TAG, "Falha ao alocar %lu bytes para RGB888", (unsigned long)rgb_size);
        return ESP_ERR_NO_MEM;
    }
    bool ok = fmt2rgb888(fb->buf, fb->len, PIXFORMAT_JPEG, rgb_full);
    if (!ok) {
        heap_caps_free(rgb_full);
        ESP_LOGE(TAG, "Decodificação JPEG -> RGB888 falhou");
        return ESP_FAIL;
    }

    // Empacotar em buffer float (0xRRGGBB) redimensionando on-the-fly de RGB565 -> 96x96
    // Modo: fit-shortest + crop central, preservando aspecto como no EI
    size_t sig_len = (size_t)in_w * (size_t)in_h;
    float *signal_buf = (float*)heap_caps_malloc(sig_len * sizeof(float), MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    if (!signal_buf) {
        heap_caps_free(rgb_full);
        ESP_LOGE(TAG, "Falha ao alocar %lu bytes para signal", (unsigned long)(sig_len * sizeof(float)));
        return ESP_ERR_NO_MEM;
    }

    if (!pp_fit_shortest_center_rgb888_to_u32float_signal(
            rgb_full, sw, sh, in_w, in_h, in_ch, signal_buf)) {
        heap_caps_free(rgb_full);
        heap_caps_free(signal_buf);
        ESP_LOGE(TAG, "Pré-processamento portátil falhou");
        return ESP_FAIL;
    }

    // Liberar RGB completo
    heap_caps_free(rgb_full);

    // Montar signal_t para o classificador
    signal_t signal;
    if (numpy::signal_from_buffer(signal_buf, sig_len, &signal) != 0) {
        heap_caps_free(signal_buf);
        ESP_LOGE(TAG, "signal_from_buffer falhou");
        return ESP_FAIL;
    }

    // Rodar classificador (o SDK decide caminho quantizado otimizado)
    ei_impulse_result_t result = {0};
    uint64_t t0 = esp_timer_get_time();
    EI_IMPULSE_ERROR ei_err = run_classifier(&signal, &result, false);
    float latency_ms = (float)(esp_timer_get_time() - t0) / 1000.0f;

    // Liberar buffer do signal
    heap_caps_free(signal_buf);

    if (ei_err != EI_IMPULSE_OK) {
        ESP_LOGW(TAG, "run_classifier falhou (%d)", (int)ei_err);
        return ESP_FAIL;
    }

    // Preencher resultado simples
    out_result->count = EI_CLASSIFIER_LABEL_COUNT;
    out_result->latency_ms = latency_ms;
    out_result->predictions = (ei_prediction_t*)heap_caps_malloc(sizeof(ei_prediction_t) * out_result->count, MALLOC_CAP_8BIT | MALLOC_CAP_SPIRAM);
    if (!out_result->predictions) {
        ESP_LOGE(TAG, "Falha ao alocar predições");
        return ESP_ERR_NO_MEM;
    }
    for (int i = 0; i < out_result->count; i++) {
        out_result->predictions[i].label = result.classification[i].label;
        out_result->predictions[i].score = result.classification[i].value;
    }

    return ESP_OK;
}

extern "C" void ei_free_result(ei_result_t* result) {
    if (!result) return;
    if (result->predictions) {
        heap_caps_free(result->predictions);
        result->predictions = NULL;
    }
    result->count = 0;
    result->latency_ms = 0.0f;
}

