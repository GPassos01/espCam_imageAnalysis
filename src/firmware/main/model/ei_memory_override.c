/**
 * @file ei_memory_override.c
 * @brief Override das funções de alocação de memória do Edge Impulse para usar PSRAM
 */

#include <stdlib.h>
#include <string.h>
#include "esp_heap_caps.h"
#include "esp_log.h"
#include "esp_system.h"



static const char* TAG = "EI_MEM";

// Override das funções de alocação para usar PSRAM quando disponível
void *ei_malloc(size_t size) {
    void* ptr = NULL;
    
    // Para alocações MUITO grandes (>100KB), usar PSRAM
    if (size > 100 * 1024) {
        ptr = heap_caps_malloc(size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
        if (ptr) {
            ESP_LOGI(TAG, "✅ Alocado %zu bytes na PSRAM (malloc) - muito grande", size);
            return ptr;
        } else {
            ESP_LOGW(TAG, "⚠️ Falha PSRAM para %zu bytes, tentando RAM interna", size);
        }
    }
    
    // Para alocações normais, usar RAM interna (mais rápida)
    ptr = malloc(size);
    if (ptr) {
        ESP_LOGD(TAG, "Alocado %zu bytes na RAM interna (malloc) - otimizado", size);
    } else {
        ESP_LOGE(TAG, "❌ Falha ao alocar %zu bytes (malloc)", size);
    }
    
    return ptr;
}

void *ei_calloc(size_t nitems, size_t size) {
    size_t total_size = nitems * size;
    void* ptr = NULL;
    
    // Para alocações MUITO grandes (>100KB), usar PSRAM
    if (total_size > 100 * 1024) {
        ptr = heap_caps_calloc(nitems, size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
        if (ptr) {
            ESP_LOGI(TAG, "✅ Alocado %zu bytes na PSRAM (calloc) - muito grande", total_size);
            return ptr;
        } else {
            ESP_LOGW(TAG, "⚠️ Falha PSRAM para %zu bytes, tentando RAM interna", total_size);
        }
    }
    
    // Para alocações normais, usar RAM interna (mais rápida)
    ptr = calloc(nitems, size);
    if (ptr) {
        ESP_LOGD(TAG, "Alocado %zu bytes na RAM interna (calloc) - otimizado", total_size);
    } else {
        ESP_LOGE(TAG, "❌ Falha ao alocar %zu bytes (calloc)", total_size);
    }
    
    return ptr;
}

// Função específica para alocação alinhada (usada pelo tensor arena)
void *ei_aligned_calloc(size_t align, size_t size) {
    void* ptr = NULL;
    
    ESP_LOGI(TAG, "🎯 ei_aligned_calloc: alinhamento=%zu, tamanho=%zu bytes", align, size);
    
    // Para tensor arena, priorizar PSRAM para evitar overflow de DRAM
    ptr = heap_caps_aligned_calloc(align, 1, size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    if (ptr) {
        ESP_LOGI(TAG, "✅ Tensor arena alocada na PSRAM: %zu bytes (align %zu)", size, align);
        return ptr;
    }

    ESP_LOGW(TAG, "⚠️ PSRAM indisponível, tentando RAM interna para tensor arena");
    ptr = aligned_alloc(align, size);
    if (ptr) {
        memset(ptr, 0, size);  // Simular calloc
        ESP_LOGW(TAG, "⚠️ Tensor arena alocada na RAM interna (risco de DRAM overflow): %zu bytes", size);
        return ptr;
    }

    ESP_LOGE(TAG, "❌ FALHA TOTAL: não foi possível alocar tensor arena de %zu bytes", size);
    
    return ptr;
}

void ei_free(void *ptr) {
    free(ptr);
}

// Função auxiliar para debug de memória
void ei_print_memory_info(void) {
    ESP_LOGI(TAG, "=== Info Memória ===");
    ESP_LOGI(TAG, "Heap livre total: %lu bytes", (unsigned long)esp_get_free_heap_size());
    ESP_LOGI(TAG, "Heap livre interna: %lu bytes", 
             (unsigned long)heap_caps_get_free_size(MALLOC_CAP_INTERNAL));
    ESP_LOGI(TAG, "Heap livre PSRAM: %lu bytes", 
             (unsigned long)heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
    ESP_LOGI(TAG, "Maior bloco livre interno: %lu bytes", 
             (unsigned long)heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL));
    ESP_LOGI(TAG, "Maior bloco livre PSRAM: %lu bytes", 
             (unsigned long)heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM));
}


