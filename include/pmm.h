#ifndef PMM_H
#define PMM_H
#include <stdint.h>
#include <stddef.h>
#include "limine.h"
void pmm_init(struct limine_memmap_response* m);
void* pmm_alloc(size_t p);
void pmm_free(void* ptr, size_t p);
#endif
