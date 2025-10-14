/**
 * @file test_dataset.c
 * @brief Sistema para testar inferência com imagens do dataset local
 */

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "esp_spiffs.h"
#include "esp_camera.h"
#include "esp_heap_caps.h"
#include "img_converters.h"
#include "ei_inference.h"

static const char* TAG = "TEST_DATASET";

// Estrutura para armazenar resultado da classificação
typedef struct {
    char category[32];
    float confidence;
    uint32_t inference_time_ms;
} classification_result_t;

/**
 * Inicializa SPIFFS para leitura de imagens
 */
esp_err_t init_spiffs(void) {
    esp_vfs_spiffs_conf_t conf = {
        .base_path = "/spiffs",
        .partition_label = NULL,
        .max_files = 5,
        .format_if_mount_failed = true
    };
    
    esp_err_t ret = esp_vfs_spiffs_register(&conf);
    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Falha ao montar ou formatar filesystem");
        } else if (ret == ESP_ERR_NOT_FOUND) {
            ESP_LOGE(TAG, "Falha ao encontrar partição SPIFFS");
        } else {
            ESP_LOGE(TAG, "Falha ao inicializar SPIFFS (%s)", esp_err_to_name(ret));
        }
        return ret;
    }
    
    size_t total = 0, used = 0;
    ret = esp_spiffs_info(conf.partition_label, &total, &used);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Falha ao obter informações da partição SPIFFS (%s)", esp_err_to_name(ret));
    } else {
        ESP_LOGI(TAG, "Partição SPIFFS: total: %d, usado: %d", total, used);
    }
    
    return ESP_OK;
}

/**
 * Carrega imagem JPEG do SPIFFS
 */
static uint8_t* load_jpeg_from_spiffs(const char* filename, size_t* out_size) {
    char filepath[128];
    snprintf(filepath, sizeof(filepath), "/spiffs/%s", filename);
    
    FILE* f = fopen(filepath, "rb");
    if (!f) {
        ESP_LOGE(TAG, "Falha ao abrir arquivo: %s", filepath);
        return NULL;
    }
    
    // Obter tamanho do arquivo
    fseek(f, 0, SEEK_END);
    size_t size = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    // Alocar memória
    uint8_t* buffer = (uint8_t*)heap_caps_malloc(size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    if (!buffer) {
        ESP_LOGE(TAG, "Falha ao alocar %d bytes para imagem", size);
        fclose(f);
        return NULL;
    }
    
    // Ler arquivo
    size_t read = fread(buffer, 1, size, f);
    fclose(f);
    
    if (read != size) {
        ESP_LOGE(TAG, "Erro ao ler arquivo (lido: %d, esperado: %d)", read, size);
        free(buffer);
        return NULL;
    }
    
    *out_size = size;
    ESP_LOGI(TAG, "Imagem carregada: %s (%d bytes)", filename, size);
    return buffer;
}

/**
 * Classifica uma imagem do dataset
 */
esp_err_t classify_dataset_image(const char* filename, classification_result_t* result) {
    if (!filename || !result) {
        return ESP_ERR_INVALID_ARG;
    }
    
    // Carregar imagem JPEG
    size_t jpeg_size = 0;
    uint8_t* jpeg_buffer = load_jpeg_from_spiffs(filename, &jpeg_size);
    if (!jpeg_buffer) {
        return ESP_FAIL;
    }
    
    // Criar estrutura camera_fb_t fake para usar com ei_run_inference
    camera_fb_t fb = {
        .buf = jpeg_buffer,
        .len = jpeg_size,
        .width = 0,  // Será detectado pelo decodificador
        .height = 0,
        .format = PIXFORMAT_JPEG,
        .timestamp = {0}
    };
    
    // Executar inferência
    ei_result_t ei_result;
    int64_t start_time = esp_timer_get_time();
    
    esp_err_t ret = ei_run_inference(&fb, &ei_result);
    
    int64_t inference_time = (esp_timer_get_time() - start_time) / 1000; // converter para ms
    
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Falha na inferência: %s", esp_err_to_name(ret));
        free(jpeg_buffer);
        return ret;
    }
    
    // Encontrar classe com maior confiança
    float max_score = 0;
    const char* best_label = "unknown";
    
    for (int i = 0; i < ei_result.count; i++) {
        ESP_LOGI(TAG, "  %s: %.2f%%", 
                 ei_result.predictions[i].label, 
                 ei_result.predictions[i].score * 100);
        
        if (ei_result.predictions[i].score > max_score) {
            max_score = ei_result.predictions[i].score;
            best_label = ei_result.predictions[i].label;
        }
    }
    
    // Preencher resultado
    strncpy(result->category, best_label, sizeof(result->category) - 1);
    result->confidence = max_score;
    result->inference_time_ms = inference_time;
    
    ESP_LOGI(TAG, "🎯 Classificação: %s (%.1f%%) - Tempo: %lldms", 
             result->category, result->confidence * 100, (long long)inference_time);
    
    // Limpar memória
    if (ei_result.predictions) {
        free(ei_result.predictions);
    }
    free(jpeg_buffer);
    
    return ESP_OK;
}

/**
 * Testa múltiplas imagens do dataset
 */
void test_dataset_images(void) {
    ESP_LOGI(TAG, "=== INICIANDO TESTE COM IMAGENS DO DATASET ===");
    
    // Lista de imagens de teste (você precisa copiar estas para SPIFFS)
    const char* test_images[] = {
        "flood_test.jpg",
        "high_test.jpg", 
        "low_test.jpg",
        "medium_test.jpg"
    };
    
    const char* expected_classes[] = {
        "flood",
        "high",
        "low", 
        "medium"
    };
    
    int num_images = sizeof(test_images) / sizeof(test_images[0]);
    int correct = 0;
    
    for (int i = 0; i < num_images; i++) {
        ESP_LOGI(TAG, "\n📸 Testando imagem %d/%d: %s", i+1, num_images, test_images[i]);
        ESP_LOGI(TAG, "   Classe esperada: %s", expected_classes[i]);
        
        classification_result_t result;
        esp_err_t ret = classify_dataset_image(test_images[i], &result);
        
        if (ret == ESP_OK) {
            bool is_correct = (strcmp(result.category, expected_classes[i]) == 0);
            if (is_correct) {
                correct++;
                ESP_LOGI(TAG, "   ✅ CORRETO! %s (%.1f%%)", 
                        result.category, result.confidence * 100);
            } else {
                ESP_LOGW(TAG, "   ❌ INCORRETO! Obtido: %s (%.1f%%), Esperado: %s", 
                        result.category, result.confidence * 100, expected_classes[i]);
            }
        } else {
            ESP_LOGE(TAG, "   ⚠️  Erro na classificação");
        }
        
        vTaskDelay(pdMS_TO_TICKS(1000)); // Pausa entre testes
    }
    
    ESP_LOGI(TAG, "\n=== RESULTADO FINAL ===");
    ESP_LOGI(TAG, "Acurácia: %d/%d (%.1f%%)", 
             correct, num_images, (float)correct * 100.0f / num_images);
}

/**
 * Função para testar uma imagem específica via comando
 */
void test_single_image(const char* category, const char* filename) {
    ESP_LOGI(TAG, "=== TESTE DE IMAGEM ÚNICA ===");
    ESP_LOGI(TAG, "Arquivo: %s", filename);
    ESP_LOGI(TAG, "Categoria esperada: %s", category);
    
    classification_result_t result;
    esp_err_t ret = classify_dataset_image(filename, &result);
    
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "\n📊 RESULTADO DA CLASSIFICAÇÃO:");
        ESP_LOGI(TAG, "   Categoria detectada: %s", result.category);
        ESP_LOGI(TAG, "   Confiança: %.1f%%", result.confidence * 100);
        ESP_LOGI(TAG, "   Tempo de inferência: %lums", (unsigned long)result.inference_time_ms);
        
        if (strcmp(result.category, category) == 0) {
            ESP_LOGI(TAG, "   ✅ Classificação CORRETA!");
        } else {
            ESP_LOGW(TAG, "   ❌ Classificação INCORRETA (esperado: %s)", category);
        }
    } else {
        ESP_LOGE(TAG, "Erro ao classificar imagem: %s", esp_err_to_name(ret));
    }
}
