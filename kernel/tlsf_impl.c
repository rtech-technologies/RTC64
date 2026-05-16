#include <stddef.h>
#include <stdint.h>

/* Simple First-Fit Allocator for Sovereign Kernel */

#define HEAP_SIZE (1024 * 1024 * 4)
static uint8_t heap[HEAP_SIZE];

typedef struct block {
    size_t size;
    int free;
    struct block* next;
} block_t;

static block_t* free_list = (block_t*)heap;

void init_heap() {
    free_list->size = HEAP_SIZE - sizeof(block_t);
    free_list->free = 1;
    free_list->next = NULL;
}

void* tlsf_malloc(void* tlsf, size_t size) {
    (void)tlsf;
    static int initialized = 0;
    if (!initialized) {
        init_heap();
        initialized = 1;
    }

    block_t* curr = free_list;
    while (curr) {
        if (curr->free && curr->size >= size) {
            if (curr->size > size + sizeof(block_t) + 16) {
                block_t* next = (block_t*)((uint8_t*)curr + sizeof(block_t) + size);
                next->size = curr->size - size - sizeof(block_t);
                next->free = 1;
                next->next = curr->next;
                curr->size = size;
                curr->next = next;
            }
            curr->free = 0;
            return (void*)((uint8_t*)curr + sizeof(block_t));
        }
        curr = curr->next;
    }
    return NULL;
}

void tlsf_free(void* tlsf, void* ptr) {
    (void)tlsf;
    if (!ptr) return;
    block_t* block = (block_t*)((uint8_t*)ptr - sizeof(block_t));
    block->free = 1;

    /* Simple Coalescing */
    block_t* curr = free_list;
    while (curr) {
        if (curr->free && curr->next && curr->next->free) {
            curr->size += curr->next->size + sizeof(block_t);
            curr->next = curr->next->next;
        } else {
            curr = curr->next;
        }
    }
}

void* tlsf_realloc(void* tlsf, void* ptr, size_t size) {
    if (!ptr) return tlsf_malloc(tlsf, size);
    block_t* block = (block_t*)((uint8_t*)ptr - sizeof(block_t));
    if (block->size >= size) return ptr;

    void* new_ptr = tlsf_malloc(tlsf, size);
    if (new_ptr) {
        size_t copy_size = block->size < size ? block->size : size;
        for (size_t i = 0; i < copy_size; i++) {
            ((uint8_t*)new_ptr)[i] = ((uint8_t*)ptr)[i];
        }
        tlsf_free(tlsf, ptr);
    }
    return new_ptr;
}

void* tlsf_create_with_pool(void* mem, size_t bytes) {
    (void)mem; (void)bytes;
    init_heap();
    return (void*)heap;
}
