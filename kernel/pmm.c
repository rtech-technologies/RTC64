/* Modified by Sovereign: Meaty Physical Memory Manager with Bitmap-based Page Allocation and Multi-block support */
#include "pro_os.h"
#include <string.h>
#include "serial.h"

#define PAGE_SIZE 4096
static uint64_t* pmm_bitmap = NULL;
static uint64_t  pmm_total_pages = 0;
static uint64_t  pmm_bitmap_size = 0;
static uint64_t  pmm_last_alloc = 0;

void pmm_init(struct limine_memmap_response* map) {
    uint64_t top_address = 0;
    uint64_t usable_memory = 0;

    for (uint64_t i = 0; i < map->entry_count; i++) {
        struct limine_memmap_entry* en = map->entries[i];
        if (en->type == LIMINE_MEMMAP_USABLE) {
            if (en->base + en->length > top_address) top_address = en->base + en->length;
            usable_memory += en->length;
        }
    }

    pmm_total_pages = top_address / PAGE_SIZE;
    pmm_bitmap_size = (pmm_total_pages / 64) + 1;

    serial_printf("[PMM] Total detected memory top: %p\n", (void*)top_address);
    serial_printf("[PMM] Usable memory: %d MB\n", (int)(usable_memory / (1024 * 1024)));

    /* Find a spot for the bitmap */
    for (uint64_t i = 0; i < map->entry_count; i++) {
        struct limine_memmap_entry* en = map->entries[i];
        if (en->type == LIMINE_MEMMAP_USABLE && en->length >= pmm_bitmap_size * 8) {
            pmm_bitmap = (uint64_t*)(en->base + hhdm_offset);
            memset(pmm_bitmap, 0xFF, pmm_bitmap_size * 8); /* Mark all as used initially */
            en->base += pmm_bitmap_size * 8;
            en->length -= pmm_bitmap_size * 8;
            serial_printf("[PMM] Bitmap placed at %p (Size: %d bytes)\n", pmm_bitmap, (int)(pmm_bitmap_size * 8));
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
    serial_printf("[PMM] Physical memory management active.\n");
}

static void pmm_mark_used(uint64_t page) {
    pmm_bitmap[page / 64] |= (1ULL << (page % 64));
}

static void pmm_mark_free(uint64_t page) {
    pmm_bitmap[page / 64] &= ~(1ULL << (page % 64));
}

static bool pmm_is_used(uint64_t page) {
    return (pmm_bitmap[page / 64] & (1ULL << (page % 64))) != 0;
}

void* pmm_alloc(void) {
    if (!pmm_bitmap) return NULL;
    for (uint64_t i = pmm_last_alloc; i < pmm_total_pages; i++) {
        if (!pmm_is_used(i)) {
            pmm_mark_used(i);
            pmm_last_alloc = i;
            return (void*)(i * PAGE_SIZE);
        }
    }
    /* Wrap around */
    for (uint64_t i = 0; i < pmm_last_alloc; i++) {
        if (!pmm_is_used(i)) {
            pmm_mark_used(i);
            pmm_last_alloc = i;
            return (void*)(i * PAGE_SIZE);
        }
    }
    return NULL;
}

void* pmm_alloc_low(void) {
    if (!pmm_bitmap) return NULL;
    uint64_t max_page = 0x100000000ULL / PAGE_SIZE;
    if (max_page > pmm_total_pages) max_page = pmm_total_pages;

    for (uint64_t i = 0; i < max_page; i++) {
        if (!pmm_is_used(i)) {
            pmm_mark_used(i);
            return (void*)(i * PAGE_SIZE);
        }
    }
    return NULL;
}

void* pmm_alloc_blocks(size_t count) {
    if (!pmm_bitmap || count == 0) return NULL;
    if (count == 1) return pmm_alloc();

    for (uint64_t i = 0; i < pmm_total_pages - count; i++) {
        bool found = true;
        for (size_t j = 0; j < count; j++) {
            if (pmm_is_used(i + j)) {
                found = false;
                i += j; /* Optimization: skip ahead */
                break;
            }
        }
        if (found) {
            for (size_t j = 0; j < count; j++) pmm_mark_used(i + j);
            return (void*)(i * PAGE_SIZE);
        }
    }
    return NULL;
}

void* pmm_alloc_blocks_low(size_t count) {
    if (count == 0) return NULL;
    uint64_t max_page = 0x100000000ULL / PAGE_SIZE;
    if (max_page > pmm_total_pages) max_page = pmm_total_pages;

    for (uint64_t i = 0; i < max_page - count; i++) {
        bool found = true;
        for (size_t j = 0; j < count; j++) {
            if (pmm_is_used(i + j)) {
                found = false;
                i += j;
                break;
            }
        }
        if (found) {
            for (size_t j = 0; j < count; j++) pmm_mark_used(i + j);
            return (void*)(i * PAGE_SIZE);
        }
    }
    return NULL;
}

/* Sovereign Covenant: Security-hardened memory reclamation with automated scrubbing */
void pmm_free(void* addr) {
    if (!addr) return;

    /* Audit Step 2: Scrub memory before returning to PMM (Sovereign Covenant Requirement) */
    memset((void*)((uint64_t)addr + hhdm_offset), 0, PAGE_SIZE);

    uint64_t page = (uint64_t)addr / PAGE_SIZE;
    if (page < pmm_total_pages) {
        pmm_mark_free(page);
    }
}

void pmm_free_blocks(void* addr, size_t count) {
    if (!addr) return;

    /* Audit Step 2: Scrub multiple blocks (Sovereign Covenant Requirement) */
    memset((void*)((uint64_t)addr + hhdm_offset), 0, count * PAGE_SIZE);

    uint64_t start_page = (uint64_t)addr / PAGE_SIZE;
    for (size_t i = 0; i < count; i++) {
        if (start_page + i < pmm_total_pages) {
            pmm_mark_free(start_page + i);
        }
    }
}
