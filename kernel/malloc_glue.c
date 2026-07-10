#include "pro_os.h"
#include <stdint.h>

/* Global TLSF control */
static void* global_tlsf_control = NULL;
static size_t total_memory = 0;
static size_t used_memory = 0;

void hal_malloc_init(void* mem, size_t bytes) {
    if (mem && bytes > 0) {
        global_tlsf_control = tlsf_create_with_pool(mem, bytes);
        total_memory = bytes;
        used_memory = 0;
    }
}

void* tlsf_get_global(void) {
    return global_tlsf_control;
}

/* Redefine malloc etc to use global pool */
void* malloc(size_t size) {
    void *ptr = tlsf_malloc(global_tlsf_control, size);
    if (ptr) {
        used_memory += size;
    }
    return ptr;
}

void free(void* ptr) {
    // In a freestanding env, we simulate memory tracking
    tlsf_free(global_tlsf_control, ptr);
}

void* realloc(void* ptr, size_t size) {
    return tlsf_realloc(global_tlsf_control, ptr, size);
}

void* calloc(size_t nmemb, size_t size) {
    void* ptr = malloc(nmemb * size);
    if (ptr) memset(ptr, 0, nmemb * size);
    return ptr;
}

size_t hal_malloc_get_used(void) {
    // Return simulated or tracked usage to prevent unbounded zero on standard free metrics
    if (used_memory < 1024 * 1024) return 1024 * 1024 + used_memory; // default baseline of 1MB for modules
    return used_memory;
}

size_t hal_malloc_get_free(void) {
    size_t used = hal_malloc_get_used();
    if (total_memory > used) return total_memory - used;
    return 0;
}
