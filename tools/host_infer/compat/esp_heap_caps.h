#ifndef ESP_HEAP_CAPS_H_COMPAT
#define ESP_HEAP_CAPS_H_COMPAT

#include <stdlib.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MALLOC_CAP_SPIRAM  (1<<0)
#define MALLOC_CAP_8BIT    (1<<1)

static inline void* heap_caps_malloc(size_t size, uint32_t caps) {
    (void)caps; return malloc(size);
}
static inline void* heap_caps_calloc(size_t n, size_t size, uint32_t caps) {
    (void)caps; return calloc(n, size);
}
static inline void  heap_caps_free(void* p) {
    free(p);
}
static inline size_t heap_caps_get_free_size(uint32_t caps) {
    (void)caps; return 0;
}

#ifdef __cplusplus
}
#endif

#endif // ESP_HEAP_CAPS_H_COMPAT


