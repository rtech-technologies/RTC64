#include "pro_os.h"
#include <stdint.h>
#include "serial.h"
static void* tlsf_ptr = 0; static size_t used = 0, total = 0;
void hal_malloc_init(void* m, size_t b) { total = b; used = 0; if (m && b > 0) tlsf_ptr = tlsf_create_with_pool(m, b); }
void* tlsf_get_global(void) { return tlsf_ptr; }
void* malloc(size_t s) { void* p = tlsf_malloc(tlsf_ptr, s); if (p) used += s; return p; }
void free(void* p) { tlsf_free(tlsf_ptr, p); }
void* realloc(void* p, size_t s) { return tlsf_realloc(tlsf_ptr, p, s); }
void* calloc(size_t n, size_t s) { void* p = malloc(n * s); if (p) memset(p, 0, n * s); return p; }
size_t hal_get_heap_used(void) { return used; }
size_t hal_get_heap_total(void) { return total; }
