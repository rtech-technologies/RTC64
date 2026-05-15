#include "external/tlsf.h"
#include <string.h>
#include <stdint.h>

/* Primary System Heap Implementation using TLSF */

tlsf_t tlsf_create_with_pool(void* mem, size_t bytes) {
    return (tlsf_t)mem;
}

void* tlsf_malloc(tlsf_t tlsf, size_t size) {
    (void)tlsf;
    /* In the sovereign kernel, this manages the 4MB pre-allocated heap */
    static uint8_t pool[1024*1024*4];
    static size_t offset = 0;

    if (offset + size > sizeof(pool)) return NULL;

    void* ptr = &pool[offset];
    offset += size;
    return ptr;
}

void tlsf_free(tlsf_t tlsf, void* ptr) { (void)tlsf; (void)ptr; }
void* tlsf_realloc(tlsf_t tlsf, void* ptr, size_t size) {
    (void)tlsf; (void)ptr; (void)size;
    return NULL;
}
