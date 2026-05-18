#include <pro_os.h>
#include <tlsf.h>

#define HEAP_SIZE (64 * 1024 * 1024)
static uint8_t heap[HEAP_SIZE] __attribute__((aligned(8)));
static tlsf_t pool = NULL;

void* bump_alloc(size_t size) {
    if (!pool) {
        pool = tlsf_create_with_pool(heap, HEAP_SIZE);
    }
    return tlsf_malloc(pool, size);
}

void bump_reset(void) {
    // With TLSF, reset means recreating the pool
    pool = tlsf_create_with_pool(heap, HEAP_SIZE);
}

void* malloc(size_t size) { return bump_alloc(size); }
void free(void* ptr) { if (pool && ptr) tlsf_free(pool, ptr); }
