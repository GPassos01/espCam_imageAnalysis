/**
 * @file advanced_analysis.h
 * @brief Análise Avançada de Imagens com Buffer de Histórico
 * 
 * Este módulo implementa recursos avançados de análise de imagens
 * aproveitando os ~4MB de PSRAM utilizáveis:
 * - Buffer circular de histórico de imagens
 * - Análise temporal de padrões
 * - Múltiplas imagens de referência
 * - Detecção de tendências
 * 
 * @author Gabriel Passos - UNESP 2025
 */

#ifndef ADVANCED_ANALYSIS_H
#define ADVANCED_ANALYSIS_H

#include "esp_camera.h"
#include "config.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Estrutura para histórico de imagens
typedef struct {
    camera_fb_t* frames[HISTORY_BUFFER_SIZE];
    float differences[HISTORY_BUFFER_SIZE];
    uint64_t timestamps[HISTORY_BUFFER_SIZE];
    int current_index;
    int count;
    bool initialized;
} image_history_t;

/**
 * @brief Estrutura para análise temporal
 */
typedef struct {
    float average_change;        ///< Mudança média
    float max_change;           ///< Mudança máxima
    float trend_slope;          ///< Inclinação da tendência
    float stability_index;      ///< Índice de estabilidade (0-1)
    bool increasing_trend;      ///< Tendência crescente
    bool decreasing_trend;      ///< Tendência decrescente
} temporal_analysis_t;

// Estrutura para múltiplas referências
typedef struct {
    camera_fb_t* day_reference;     // Referência diurna
    camera_fb_t* night_reference;   // Referência noturna
    camera_fb_t* clear_reference;   // Referência tempo claro
    camera_fb_t* weather_reference; // Referência tempo ruim
    uint64_t last_update_time;
} multi_reference_t;

/**
 * @brief Estrutura para estatísticas de eficiência de memória
 */
typedef struct {
    size_t total_psram_kb;      ///< PSRAM total em KB
    size_t free_psram_kb;       ///< PSRAM livre em KB
    size_t used_by_analysis_kb; ///< Memória usada pela análise em KB
    float psram_utilization;    ///< Utilização total da PSRAM em %
    float analysis_efficiency;  ///< Eficiência da análise vs estimado em %
    int active_references;      ///< Número de referências ativas
    int history_frames;         ///< Frames no histórico
    float buffer_utilization;   ///< Utilização do buffer em %
} memory_efficiency_t;

/**
 * Inicializa o sistema de análise avançada
 * @return ESP_OK se bem-sucedido
 */
esp_err_t advanced_analysis_init(void);

/**
 * Adiciona uma nova imagem ao buffer de histórico
 * @param frame Frame a ser adicionado
 * @param difference Diferença calculada
 * @return ESP_OK se bem-sucedido
 */
esp_err_t add_to_history(camera_fb_t* frame, float difference);

/**
 * Realiza análise temporal baseada no histórico
 * @param analysis Estrutura para armazenar resultados
 * @return ESP_OK se bem-sucedido
 */
esp_err_t perform_temporal_analysis(temporal_analysis_t* analysis);

/**
 * Atualiza as referências múltiplas baseado nas condições
 * @param current_frame Frame atual
 * @param time_of_day Hora do dia (0-23)
 * @param weather_condition Condição climática estimada
 * @return ESP_OK se bem-sucedido
 */
esp_err_t update_multi_references(camera_fb_t* current_frame, int time_of_day, int weather_condition);

/**
 * Seleciona a melhor referência para comparação
 * @param time_of_day Hora do dia atual
 * @param weather_condition Condição climática atual
 * @return Ponteiro para a melhor referência
 */
camera_fb_t* get_best_reference(int time_of_day, int weather_condition);

/**
 * Calcula índice de estabilidade da cena
 * @return Valor entre 0.0 (instável) e 1.0 (estável)
 */
float calculate_stability_index(void);

/**
 * Detecta padrões anômalos no histórico
 * @return true se anomalia detectada
 */
bool detect_anomaly_pattern(void);

/**
 * Obtém estatísticas do buffer de histórico
 * @param used_memory Memória usada em bytes
 * @param buffer_utilization Utilização do buffer (0.0-1.0)
 * @return ESP_OK se bem-sucedido
 */
esp_err_t get_history_stats(size_t* used_memory, float* buffer_utilization);

/**
 * Limpa o buffer de histórico
 */
void clear_history_buffer(void);

/**
 * Deinicializa o sistema e libera memória
 */
void advanced_analysis_deinit(void);

/**
 * @brief Obter estatísticas de eficiência de memória
 * @param stats Ponteiro para estrutura de estatísticas
 * @return ESP_OK se sucesso, erro caso contrário
 */
esp_err_t get_memory_efficiency_stats(memory_efficiency_t* stats);

/**
 * @brief Imprimir relatório detalhado de eficiência de memória
 */
void print_memory_efficiency_report(void);

#ifdef __cplusplus
}
#endif

#endif // ADVANCED_ANALYSIS_H 