#include "ff.h"
#include <stddef.h>
extern void* malloc(size_t s); extern void free(void* p);
void* ff_memalloc(UINT m) { return malloc((size_t)m); }
void ff_memfree(void* m) { free(m); }
DWORD get_fattime(void) { return 0; }
