#include <stddef.h>
#include <stdint.h>

/*
 * Sovereign TLSF (Two-Level Segregated Fit) Allocator
 * Technical Implementation for R-TECH(TM) Kernel
 * Provides O(1) time complexity for malloc/free.
 */

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

typedef struct tlsf_control {
    uint32_t fl_bitmap;
    uint16_t sl_bitmap[FL_INDEX_MAX];
    block_header_t* blocks[FL_INDEX_MAX][SL_INDEX_COUNT];
} tlsf_t;

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

void* tlsf_malloc(void* tlsf_ptr, size_t size) {
    tlsf_t* t = (tlsf_t*)tlsf_ptr;
    if (!t) return NULL;

    size = (size + ALIGN_SIZE - 1) & ~(ALIGN_SIZE - 1);
    if (size < MIN_BLOCK_SIZE) size = MIN_BLOCK_SIZE;

    int fl, sl;
    mapping_search(size, &fl, &sl);

    /* Search for a suitable block */
    uint32_t sl_map = t->sl_bitmap[fl] & (~0U << sl);
    if (!sl_map) {
        uint32_t fl_map = t->fl_bitmap & (~0U << (fl + 1));
        if (!fl_map) return NULL;
        fl = __builtin_ctz(fl_map);
        sl_map = t->sl_bitmap[fl];
    }
    sl = __builtin_ctz(sl_map);

    block_header_t* block = t->blocks[fl][sl];
    if (!block) return NULL;

    /* Remove from list */
    t->blocks[fl][sl] = block->next_free;
    if (block->next_free) block->next_free->prev_free = NULL;
    if (!t->blocks[fl][sl]) t->sl_bitmap[fl] &= ~(1U << sl);
    if (!t->sl_bitmap[fl]) t->fl_bitmap &= ~(1U << fl);

    block->size &= ~BLOCK_FREE_BIT;
    return (void*)((uint8_t*)block + sizeof(block_header_t));
}

void tlsf_free(void* tlsf_ptr, void* ptr) {
    if (!ptr || !tlsf_ptr) return;
    tlsf_t* t = (tlsf_t*)tlsf_ptr;
    block_header_t* block = (block_header_t*)((uint8_t*)ptr - sizeof(block_header_t));
    block->size |= BLOCK_FREE_BIT;

    int fl, sl;
    mapping_insert(block->size & ~0x03, &fl, &sl);

    block->next_free = t->blocks[fl][sl];
    if (block->next_free) block->next_free->prev_free = block;
    t->blocks[fl][sl] = block;
    block->prev_free = NULL;

    t->fl_bitmap |= (1U << fl);
    t->sl_bitmap[fl] |= (1U << sl);
}

void* tlsf_create_with_pool(void* mem, size_t bytes) {
    if (!mem || bytes < sizeof(tlsf_t) + sizeof(block_header_t) + MIN_BLOCK_SIZE) return NULL;

    tlsf_t* t = (tlsf_t*)mem;
    for(int i=0; i<FL_INDEX_MAX; i++) {
        t->sl_bitmap[i] = 0;
        for(int j=0; j<SL_INDEX_COUNT; j++) t->blocks[i][j] = NULL;
    }
    t->fl_bitmap = 0;

    /* Create initial big block */
    block_header_t* initial = (block_header_t*)((uint8_t*)mem + sizeof(tlsf_t));
    initial->size = (bytes - sizeof(tlsf_t) - sizeof(block_header_t)) | BLOCK_FREE_BIT;
    initial->prev_phys = NULL;

    /* Initialize block headers to mark them as free */
    initial->next_free = NULL;
    initial->prev_free = NULL;

    tlsf_free(t, (void*)((uint8_t*)initial + sizeof(block_header_t)));

    return (void*)t;
}

void* tlsf_realloc(void* tlsf, void* ptr, size_t size) {
    if (!ptr) return tlsf_malloc(tlsf, size);
    block_header_t* block = (block_header_t*)((uint8_t*)ptr - sizeof(block_header_t));
    if ((block->size & ~0x03) >= size) return ptr;

    void* new_ptr = tlsf_malloc(tlsf, size);
    if (new_ptr) {
        size_t old_size = block->size & ~0x03;
        uint8_t* src = (uint8_t*)ptr;
        uint8_t* dst = (uint8_t*)new_ptr;
        for (size_t i = 0; i < old_size && i < size; i++) dst[i] = src[i];
        tlsf_free(tlsf, ptr);
    }
    return new_ptr;
}
