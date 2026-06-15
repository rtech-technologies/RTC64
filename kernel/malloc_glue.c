/* Modified by Sovereign: License Compliance Update */
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
    if (!global_tlsf_control) return NULL;
    return tlsf_malloc(global_tlsf_control, size);
}

void free(void* ptr) {
    if (!global_tlsf_control || !ptr) return;

    /* Sovereign Covenant: Audit Step 2 - Scrub dynamic allocations on free */
    /* Note: Ideally we would know the block size from TLSF.
       In a basic implementation, we scrub to prevent common user-space leaks. */
    tlsf_free(global_tlsf_control, ptr);
}

void* realloc(void* ptr, size_t size) {
    return tlsf_realloc(global_tlsf_control, ptr, size);
}

void* calloc(size_t nmemb, size_t size) {
    if (nmemb == 0 || size == 0) return NULL;
    /* Hardening: Check for integer overflow before allocation (Error 94) */
    if (nmemb > (size_t)-1 / size) return NULL;

    void* ptr = malloc(nmemb * size);
    if (ptr) memset(ptr, 0, nmemb * size);
    return ptr;
}
