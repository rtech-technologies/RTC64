/* Copyright (C) 2025 Sovereign RTC64 Project. All rights reserved.
 * Licensed under the 'respect people's property' OS license. */
/* Modified by Sovereign: MEATY TLSF Implementation with Block Splitting, Coalescing and Metrics */
#include <stddef.h>
#include <stdint.h>
#include "pro_os.h"

#define FL_INDEX_MAX 30
#define SL_INDEX_COUNT 16
#define SL_INDEX_LOG2 4
#define MIN_BLOCK_SIZE (1 << 6)
#define ALIGN_SIZE 8

typedef struct block_header {
    struct block_header* prev_phys;
    size_t size; /* bit 0: free, bit 1: prev_free */
    struct block_header* next_free;
    struct block_header* prev_free;
} block_header_t;

#define BLOCK_FREE_BIT 0x01
#define BLOCK_PREV_FREE_BIT 0x02
#define BLOCK_SIZE_MASK ~(BLOCK_FREE_BIT | BLOCK_PREV_FREE_BIT)

typedef struct {
    uint32_t fl_bitmap;
    uint16_t sl_bitmap[FL_INDEX_MAX];
    block_header_t* blocks[FL_INDEX_MAX][SL_INDEX_COUNT];
    size_t total_size;
    size_t used_size;
} tlsf_control_t;

extern void* tlsf_get_global(void);

static void mapping_insert(size_t size, int* fl, int* sl) {
    if (size < MIN_BLOCK_SIZE) {
        *fl = 0;
        *sl = (int)(size / (MIN_BLOCK_SIZE / SL_INDEX_COUNT));
    } else {
        *fl = 31 - __builtin_clz((uint32_t)size);
        *sl = (int)((size >> (*fl - SL_INDEX_LOG2)) & (SL_INDEX_COUNT - 1));
    }
}

static void mapping_search(size_t size, int* fl, int* sl) {
    if (size >= MIN_BLOCK_SIZE) {
        size += (1 << (31 - __builtin_clz((uint32_t)size) - SL_INDEX_LOG2)) - 1;
    }
    mapping_insert(size, fl, sl);
}

static void remove_free_block(tlsf_control_t* t, block_header_t* block, int fl, int sl) {
    if (block->next_free) block->next_free->prev_free = block->prev_free;
    if (block->prev_free) block->prev_free->next_free = block->next_free;
    else t->blocks[fl][sl] = block->next_free;

    if (!t->blocks[fl][sl]) {
        t->sl_bitmap[fl] &= ~(1U << sl);
        if (!t->sl_bitmap[fl]) t->fl_bitmap &= ~(1U << fl);
    }
}

static void insert_free_block(tlsf_control_t* t, block_header_t* block) {
    int fl, sl;
    mapping_insert(block->size & BLOCK_SIZE_MASK, &fl, &sl);
    block->next_free = t->blocks[fl][sl];
    block->prev_free = NULL;
    if (block->next_free) block->next_free->prev_free = block;
    t->blocks[fl][sl] = block;
    t->fl_bitmap |= (1U << fl);
    t->sl_bitmap[fl] |= (1U << sl);
}

void* tlsf_malloc(void* tlsf_ptr, size_t size) {
    tlsf_control_t* t = (tlsf_control_t*)tlsf_ptr;
    if (!t) return NULL;

    size = (size + ALIGN_SIZE - 1) & ~(ALIGN_SIZE - 1);
    if (size < MIN_BLOCK_SIZE) size = MIN_BLOCK_SIZE;

    int fl, sl;
    mapping_search(size, &fl, &sl);

    uint32_t sl_map = t->sl_bitmap[fl] & (~0U << sl);
    if (!sl_map) {
        uint32_t fl_map = t->fl_bitmap & (~0U << (fl + 1));
        if (!fl_map) return NULL;
        fl = __builtin_ctz(fl_map);
        sl_map = t->sl_bitmap[fl];
    }
    sl = __builtin_ctz(sl_map);

    block_header_t* block = t->blocks[fl][sl];
    remove_free_block(t, block, fl, sl);

    size_t current_size = block->size & BLOCK_SIZE_MASK;
    if (current_size >= size + sizeof(block_header_t) + MIN_BLOCK_SIZE) {
        block_header_t* remaining = (block_header_t*)((uint8_t*)block + sizeof(block_header_t) + size);
        remaining->size = (current_size - size - sizeof(block_header_t)) | BLOCK_FREE_BIT;
        remaining->prev_phys = block;

        block->size = size | (block->size & BLOCK_PREV_FREE_BIT);
        insert_free_block(t, remaining);
        t->used_size += size + sizeof(block_header_t);
    } else {
        block->size &= ~BLOCK_FREE_BIT;
        t->used_size += current_size + sizeof(block_header_t);
    }

    return (void*)((uint8_t*)block + sizeof(block_header_t));
}

void tlsf_free(void* tlsf_ptr, void* ptr) {
    if (!ptr || !tlsf_ptr) return;
    tlsf_control_t* t = (tlsf_control_t*)tlsf_ptr;
    block_header_t* block = (block_header_t*)((uint8_t*)ptr - sizeof(block_header_t));
    size_t block_size = block->size & BLOCK_SIZE_MASK;

    /* Industrial Scrubbing: Zero out memory before deallocation */
    memset(ptr, 0, block_size);

    t->used_size -= (block_size + sizeof(block_header_t));
    block->size |= BLOCK_FREE_BIT;

    block_header_t* next = (block_header_t*)((uint8_t*)block + sizeof(block_header_t) + block_size);
    if (next->size & BLOCK_FREE_BIT) {
        int fl, sl;
        mapping_insert(next->size & BLOCK_SIZE_MASK, &fl, &sl);
        remove_free_block(t, next, fl, sl);
        block->size += (next->size & BLOCK_SIZE_MASK) + sizeof(block_header_t);
    }

    if (block->size & BLOCK_PREV_FREE_BIT) {
        block_header_t* prev = block->prev_phys;
        if (prev && (prev->size & BLOCK_FREE_BIT)) {
            int fl, sl;
            mapping_insert(prev->size & BLOCK_SIZE_MASK, &fl, &sl);
            remove_free_block(t, prev, fl, sl);
            prev->size += (block->size & BLOCK_SIZE_MASK) + sizeof(block_header_t);
            block = prev;
        }
    }

    insert_free_block(t, block);
}

void* tlsf_create_with_pool(void* mem, size_t bytes) {
    if (!mem || bytes < sizeof(tlsf_control_t) + sizeof(block_header_t) + MIN_BLOCK_SIZE) return NULL;
    tlsf_control_t* t = (tlsf_control_t*)mem;
    memset(t, 0, sizeof(tlsf_control_t));
    t->total_size = bytes;
    t->used_size = sizeof(tlsf_control_t);

    block_header_t* initial = (block_header_t*)((uint8_t*)mem + sizeof(tlsf_control_t));
    initial->size = (bytes - sizeof(tlsf_control_t) - sizeof(block_header_t)) | BLOCK_FREE_BIT;
    initial->prev_phys = NULL;
    insert_free_block(t, initial);
    return (void*)t;
}

void* tlsf_realloc(void* tlsf, void* ptr, size_t size) {
    if (!ptr) return tlsf_malloc(tlsf, size);
    block_header_t* block = (block_header_t*)((uint8_t*)ptr - sizeof(block_header_t));
    if ((block->size & BLOCK_SIZE_MASK) >= size) return ptr;
    void* new_ptr = tlsf_malloc(tlsf, size);
    if (new_ptr) {
        memcpy(new_ptr, ptr, block->size & BLOCK_SIZE_MASK);
        tlsf_free(tlsf, ptr);
    }
    return new_ptr;
}

/* MEATY: Metric Accessors */
size_t hal_malloc_get_used(void) {
    tlsf_control_t* t = (tlsf_control_t*)tlsf_get_global();
    return t ? t->used_size : 0;
}

size_t hal_malloc_get_total(void) {
    tlsf_control_t* t = (tlsf_control_t*)tlsf_get_global();
    return t ? t->total_size : 0;
}
