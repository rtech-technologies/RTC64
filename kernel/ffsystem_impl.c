#include "ff.h"
#include <stddef.h>

/* FatFs system hooks for freestanding kernel */

extern void* malloc(size_t size);
extern void free(void* ptr);

void* ff_memalloc(UINT msize) {
    return malloc((size_t)msize);
}

void ff_memfree(void* mblock) {
    free(mblock);
}
