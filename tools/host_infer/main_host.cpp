#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <vector>
#include <string>
#include <cstring>

// Loader de imagem PPM P3/P6 (RGB)
static unsigned char* load_ppm_p6(const char* path, int* w, int* h) {
    FILE* f = fopen(path, "rb");
    if (!f) return nullptr;
    char magic[3] = {0};
    if (fread(magic, 1, 2, f) != 2 || magic[0] != 'P' || (magic[1] != '3' && magic[1] != '6')) {
        fclose(f); return nullptr;
    }
    bool is_ascii = (magic[1] == '3');
    int width = 0, height = 0, maxv = 0;
    // pular comentários e espaços
    auto skip_ws = [&](void){
        int ch;
        do {
            ch = fgetc(f);
            if (ch == '#') { // comentário
                while ((ch = fgetc(f)) != '\n' && ch != EOF);
            }
        } while (ch == ' ' || ch == '\n' || ch == '\r' || ch == '\t' || ch == '\v' || ch == '\f');
        ungetc(ch, f);
    };
    skip_ws(); if (fscanf(f, "%d", &width) != 1) { fclose(f); return nullptr; }
    skip_ws(); if (fscanf(f, "%d", &height) != 1) { fclose(f); return nullptr; }
    skip_ws(); if (fscanf(f, "%d", &maxv) != 1 || maxv != 255) { fclose(f); return nullptr; }
    fgetc(f); // consumir um byte de whitespace após maxv
    size_t sz = (size_t)width * (size_t)height * 3;
    unsigned char* buf = (unsigned char*)malloc(sz);
    if (!buf) { fclose(f); return nullptr; }
    if (is_ascii) {
        // P3: ler valores ASCII
        for (size_t i = 0; i < sz; i++) {
            int val;
            if (fscanf(f, "%d", &val) != 1) { free(buf); fclose(f); return nullptr; }
            buf[i] = (unsigned char)val;
        }
    } else {
        // P6: ler binário
        if (fread(buf, 1, sz, f) != sz) { free(buf); fclose(f); return nullptr; }
    }
    fclose(f);
    *w = width; *h = height; return buf;
}

#include "edge-impulse-sdk/classifier/ei_run_classifier.h"
#include "model-parameters/model_metadata.h"
#include "portable/portable_preprocess.h"

static void usage(const char* argv0) {
    fprintf(stderr, "Uso: %s <imagem.jpg>\n", argv0);
}

int main(int argc, char** argv) {
    if (argc < 2) {
        usage(argv[0]);
        return 2;
    }
    const char* path = argv[1];

    int w, h;
    unsigned char* data = load_ppm_p6(path, &w, &h);
    if (!data) {
        fprintf(stderr, "Falha ao carregar PPM P3/P6: %s\n", path);
        return 1;
    }

    const int in_w = EI_CLASSIFIER_INPUT_WIDTH;
    const int in_h = EI_CLASSIFIER_INPUT_HEIGHT;
    const int in_ch = EI_CLASSIFIER_INPUT_CHANNELS;

    size_t sig_len = (size_t)in_w * (size_t)in_h;
    std::vector<float> signal_buf(sig_len);
    if (!pp_fit_shortest_center_rgb888_to_u32float_signal(data, w, h, in_w, in_h, in_ch, signal_buf.data())) {
        free(data);
        fprintf(stderr, "Pré-processamento falhou\n");
        return 1;
    }
    free(data);

    signal_t signal;
    if (numpy::signal_from_buffer(signal_buf.data(), sig_len, &signal) != 0) {
        fprintf(stderr, "signal_from_buffer falhou\n");
        return 1;
    }

    ei_impulse_result_t result = {0};
    EI_IMPULSE_ERROR err = run_classifier(&signal, &result, false);
    if (err != EI_IMPULSE_OK) {
        fprintf(stderr, "run_classifier falhou (%d)\n", (int)err);
        return 1;
    }

    printf("Resultado:\n");
    for (size_t i = 0; i < (size_t)EI_CLASSIFIER_LABEL_COUNT; ++i) {
        printf("  %s: %.5f\n", result.classification[i].label, result.classification[i].value);
    }
    printf("latencia_ms=%d\n", result.timing.classification);

    return 0;
}


