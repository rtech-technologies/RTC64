#ifndef LINUX_DMA_MAPPING_H
#define LINUX_DMA_MAPPING_H

#include <stdint.h>
#include <stddef.h>

typedef uintptr_t dma_addr_t;

static inline void *dma_alloc_coherent(void *dev, size_t size, dma_addr_t *dma_handle, int flag) {
    (void)dev;
    (void)flag;
    void *ptr = malloc(size);
    if (ptr && dma_handle) {
        *dma_handle = (dma_addr_t)(uintptr_t)ptr;
    }
    return ptr;
}

static inline void dma_free_coherent(void *dev, size_t size, void *cpu_addr, dma_addr_t dma_handle) {
    (void)dev;
    (void)size;
    (void)dma_handle;
    free(cpu_addr);
}

static inline dma_addr_t dma_map_single(void *dev, void *cpu_addr, size_t size, int direction) {
    (void)dev;
    (void)size;
    (void)direction;
    return (dma_addr_t)(uintptr_t)cpu_addr;
}

static inline void dma_unmap_single(void *dev, dma_addr_t dma_addr, size_t size, int direction) {
    (void)dev;
    (void)dma_addr;
    (void)size;
    (void)direction;
}

#endif
