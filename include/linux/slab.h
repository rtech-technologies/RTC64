#ifndef LINUX_SLAB_H
#define LINUX_SLAB_H

#include <stdlib.h>
#include <string.h>

#define GFP_KERNEL 0
#define GFP_ATOMIC 0

static inline void *kmalloc(size_t size, int flags) {
    (void)flags;
    return malloc(size);
}

static inline void *kzalloc(size_t size, int flags) {
    void *p = kmalloc(size, flags);
    if (p) memset(p, 0, size);
    return p;
}

static inline void kfree(const void *ptr) {
    free((void*)ptr);
}

#endif
