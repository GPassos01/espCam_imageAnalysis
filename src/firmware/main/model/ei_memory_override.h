/**
 * @file ei_memory_override.h
 * @brief Header para override das funções de alocação de memória do Edge Impulse
 * 
 * @author Gabriel Passos - UNESP 2025
 */

#ifndef EI_MEMORY_OVERRIDE_H
#define EI_MEMORY_OVERRIDE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

/**
 * @brief Override da função malloc para usar PSRAM quando apropriado
 * @param size Tamanho em bytes para alocar
 * @return Ponteiro para memória alocada ou NULL se falhar
 */
void *ei_malloc(size_t size);

/**
 * @brief Override da função calloc para usar PSRAM quando apropriado
 * @param nitems Número de itens
 * @param size Tamanho de cada item em bytes
 * @return Ponteiro para memória alocada ou NULL se falhar
 */
void *ei_calloc(size_t nitems, size_t size);

/**
 * @brief Override da função aligned_calloc para usar PSRAM quando apropriado
 * @param align Alinhamento desejado
 * @param size Tamanho em bytes para alocar
 * @return Ponteiro para memória alinhada ou NULL se falhar
 */
void *ei_aligned_calloc(size_t align, size_t size);

/**
 * @brief Override da função free
 * @param ptr Ponteiro para memória a ser liberada
 */
void ei_free(void *ptr);

/**
 * @brief Função auxiliar para debug de memória
 */
void ei_print_memory_info(void);

#ifdef __cplusplus
}
#endif

#endif // EI_MEMORY_OVERRIDE_H
