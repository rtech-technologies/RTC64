#include "pro_os.h"
#include <stdint.h>

/* Global TLSF control */
static void* global_tlsf_control = NULL;
static size_t heap_used = 0;
static size_t heap_total = 0;

void hal_malloc_init(void* mem, size_t bytes) {
    heap_total = bytes;
    if (mem && bytes > 0) {
        global_tlsf_control = tlsf_create_with_pool(mem, bytes);
    }
}

void* tlsf_get_global(void) {
    return global_tlsf_control;
}

/* Redefine malloc etc to use global pool */
void* malloc(size_t size) {
    void* ptr = tlsf_malloc(global_tlsf_control, size);
    if (ptr) heap_used += size;
    return ptr;
}

void free(void* ptr) {
    // Note: TLSF doesn't easily report block size on free without internal access
    // This is a simplified tracker
    tlsf_free(global_tlsf_control, ptr);
}

size_t hal_get_heap_used(void) { return heap_used; }
size_t hal_get_heap_total(void) { return heap_total; }

void* realloc(void* ptr, size_t size) {
    return tlsf_realloc(global_tlsf_control, ptr, size);
}

void* calloc(size_t nmemb, size_t size) {
    void* ptr = malloc(nmemb * size);
    if (ptr) memset(ptr, 0, nmemb * size);
    return ptr;
}
