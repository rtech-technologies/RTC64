#include "fatfs/ff.h"
#include "pro_os.h"

/* FatFS System Interface for Sovereign Kernel */

void* ff_memalloc(UINT msize) {
    return malloc(msize);
}

void ff_memfree(void* mblock) {
    free(mblock);
}
