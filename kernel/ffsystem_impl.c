#include "fatfs/ff.h"
#include "pro_os.h"

/* FatFS System Interface for Sovereign Kernel */

void* ff_memalloc(UINT msize) {
    extern void* malloc(size_t);
    return malloc(msize);
}

void ff_memfree(void* mblock) {
    extern void free(void*);
    free(mblock);
}
