#ifndef MODEL_H
#define MODEL_H

#include <stdint.h>
#include "esp_err.h"

// Estrutura para armazenar a predição do modelo
typedef struct {
    int class_id;           // ID da classe predita
    float confidence;       // Confiança da predição (0-1)
    char class_name[32];    // Nome da classe predita
} model_prediction_t;

// Inicializa o modelo carregando-o da partição flash
esp_err_t model_init(void);

// Realiza a inferência do modelo em uma imagem
esp_err_t model_inference(const uint8_t* image_data, model_prediction_t* prediction);

// Libera os recursos do modelo
void model_deinit(void);

#endif // MODEL_H 