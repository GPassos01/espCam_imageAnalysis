#include "model.h"
#include "esp_log.h"
#include "esp_heap_caps.h"
#include "esp_timer.h"
#include "esp_partition.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

static const char *TAG = "MODEL";

// Variáveis globais para o modelo
static const uint8_t* model_data = NULL;
static size_t model_size = 0;
static esp_partition_mmap_handle_t model_mmap_handle = 0;

// Nomes das classes (ajuste conforme seu modelo)
static const char* class_names[] = {
    "normal",
    "enchente",
    "alerta"
};

// Dimensões da imagem simulada
#define SIM_IMAGE_WIDTH  320
#define SIM_IMAGE_HEIGHT 240
#define SIM_IMAGE_CHANNELS 3

// Gera uma imagem simulada com padrões que mudam ao longo do tempo
static esp_err_t generate_simulated_image(uint8_t* image_data, size_t* image_size) {
    static uint32_t frame_count = 0;
    
    // Aloca memória para a imagem
    *image_size = SIM_IMAGE_WIDTH * SIM_IMAGE_HEIGHT * SIM_IMAGE_CHANNELS;
    
    // Gera um padrão que muda a cada frame
    for (int y = 0; y < SIM_IMAGE_HEIGHT; y++) {
        for (int x = 0; x < SIM_IMAGE_WIDTH; x++) {
            int idx = (y * SIM_IMAGE_WIDTH + x) * SIM_IMAGE_CHANNELS;
            
            // Gera um padrão de onda que muda com o tempo
            float wave = sin(x * 0.1 + frame_count * 0.1) * 0.5 + 0.5;
            
            // Simula diferentes níveis de água
            if (y > SIM_IMAGE_HEIGHT * (0.5 + wave * 0.3)) {
                // Água (tons de azul)
                image_data[idx] = 0;     // R
                image_data[idx + 1] = 0; // G
                image_data[idx + 2] = 255; // B
            } else {
                // Céu (tons de azul claro)
                image_data[idx] = 135;     // R
                image_data[idx + 1] = 206; // G
                image_data[idx + 2] = 235; // B
            }
        }
    }
    
    frame_count++;
    return ESP_OK;
}

esp_err_t model_init(void) {
    const esp_partition_t* model_partition = esp_partition_find_first(
        0x99, // Tipo personalizado para o modelo
        0x00, // Subtipo
        "model"
    );

    if (model_partition == NULL) {
        ESP_LOGE("MODEL", "Partição do modelo não encontrada");
        return ESP_ERR_NOT_FOUND;
    }

    esp_err_t err = esp_partition_mmap(
        model_partition,
        0,
        model_partition->size,
        ESP_PARTITION_MMAP_DATA,
        (const void**)&model_data,
        &model_mmap_handle
    );

    if (err != ESP_OK) {
        ESP_LOGE("MODEL", "Falha ao mapear partição do modelo: %d", err);
        return err;
    }

    return ESP_OK;
}

esp_err_t model_inference(const uint8_t* image_data, model_prediction_t* prediction) {
    if (model_data == NULL || image_data == NULL || prediction == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    // Simulação de inferência
    prediction->confidence = 0.85f;
    prediction->class_id = 1;
    strcpy(prediction->class_name, "Pessoa");

    return ESP_OK;
}

void model_deinit(void) {
    if (model_data != NULL) {
        esp_partition_munmap(model_mmap_handle);
        model_data = NULL;
    }
} 