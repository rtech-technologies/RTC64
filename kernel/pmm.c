/* Modified by Sovereign: Meaty Physical Memory Manager with Bitmap-based Page Allocation */
#include "pro_os.h"
#include <string.h>

#define PAGE_SIZE 4096
static uint64_t* pmm_bitmap = NULL;
static uint64_t  pmm_total_pages = 0;
static uint64_t  pmm_bitmap_size = 0;

void pmm_init(struct limine_memmap_response* map) {
    uint64_t top_address = 0;
    for (uint64_t i = 0; i < map->entry_count; i++) {
        struct limine_memmap_entry* en = map->entries[i];
        if (en->type == LIMINE_MEMMAP_USABLE) {
            if (en->base + en->length > top_address) top_address = en->base + en->length;
        }
    }

    pmm_total_pages = top_address / PAGE_SIZE;
    pmm_bitmap_size = (pmm_total_pages / 64) + 1;

    /* Find a spot for the bitmap */
    for (uint64_t i = 0; i < map->entry_count; i++) {
        struct limine_memmap_entry* en = map->entries[i];
        if (en->type == LIMINE_MEMMAP_USABLE && en->length >= pmm_bitmap_size * 8) {
            pmm_bitmap = (uint64_t*)(en->base + hhdm_offset);
            memset(pmm_bitmap, 0xFF, pmm_bitmap_size * 8); /* Mark all as used initially */
            en->base += pmm_bitmap_size * 8;
            en->length -= pmm_bitmap_size * 8;
            break;
        }
    }

    /* Mark usable pages as free in bitmap */
    for (uint64_t i = 0; i < map->entry_count; i++) {
        struct limine_memmap_entry* en = map->entries[i];
        if (en->type == LIMINE_MEMMAP_USABLE) {
            for (uint64_t addr = en->base; addr < en->base + en->length; addr += PAGE_SIZE) {
                uint64_t page = addr / PAGE_SIZE;
                pmm_bitmap[page / 64] &= ~(1ULL << (page % 64));
            }
        }
    }
}

void* pmm_alloc(void) {
    for (uint64_t i = 0; i < pmm_bitmap_size; i++) {
        if (pmm_bitmap[i] != 0xFFFFFFFFFFFFFFFFULL) {
            for (int j = 0; j < 64; j++) {
                if (!(pmm_bitmap[i] & (1ULL << j))) {
                    pmm_bitmap[i] |= (1ULL << j);
                    return (void*)((i * 64 + j) * PAGE_SIZE);
                }
            }
        }
    }
    return NULL;
}
