#include "pro_os.h"
#include <stdint.h>

/* Global TLSF control */
static void* global_tlsf_control = NULL;

void hal_malloc_init(void* mem, size_t bytes) {
    if (mem && bytes > 0) {
        global_tlsf_control = tlsf_create_with_pool(mem, bytes);
    }
}

void* tlsf_get_global(void) {
    return global_tlsf_control;
}

/* Redefine malloc etc to use global pool */
void* malloc(size_t size) {
    return tlsf_malloc(global_tlsf_control, size);
}

void free(void* ptr) {
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
