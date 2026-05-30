#include "pmm.h"
#include "pro_os.h"
#include "serial.h"
#include <string.h>

static uint8_t* pmm_bitmap;
static size_t pmm_total_pages = 0;
static size_t pmm_free_pages = 0;
static uint64_t pmm_highest_addr = 0;

void pmm_init(struct limine_memmap_response* memmap) {
    serial_write("[PMM] Initializing...\n");
    for (uint64_t i = 0; i < memmap->entry_count; i++) {
        struct limine_memmap_entry* entry = memmap->entries[i];
        if (entry->type == LIMINE_MEMMAP_USABLE) {
            uint64_t top = entry->base + entry->length;
            if (top > pmm_highest_addr) pmm_highest_addr = top;
        }
    }
    pmm_total_pages = pmm_highest_addr / 4096;
    size_t bitmap_size = pmm_total_pages / 8;
    for (uint64_t i = 0; i < memmap->entry_count; i++) {
        struct limine_memmap_entry* entry = memmap->entries[i];
        if (entry->type == LIMINE_MEMMAP_USABLE && entry->length >= bitmap_size) {
            pmm_bitmap = (uint8_t*)(entry->base + hhdm_offset);
            memset(pmm_bitmap, 0xFF, bitmap_size);
            entry->base += bitmap_size;
            entry->length -= bitmap_size;
            break;
        }
    }
    for (uint64_t i = 0; i < memmap->entry_count; i++) {
        struct limine_memmap_entry* entry = memmap->entries[i];
        if (entry->type == LIMINE_MEMMAP_USABLE) {
            for (uint64_t addr = entry->base; addr < entry->base + entry->length; addr += 4096) {
                size_t page = addr / 4096;
                pmm_bitmap[page / 8] &= ~(1 << (page % 8));
                pmm_free_pages++;
            }
        }
    }
    serial_printf("[PMM] Ready. Free pages: %llu\n", pmm_free_pages);
}

void* pmm_alloc(size_t pages) {
    size_t count = 0;
    for (size_t i = 0; i < pmm_total_pages; i++) {
        if (!(pmm_bitmap[i / 8] & (1 << (i % 8)))) {
            if (++count == pages) {
                size_t start = i - pages + 1;
                for (size_t j = start; j <= i; j++) pmm_bitmap[j / 8] |= (1 << (j % 8));
                pmm_free_pages -= pages;
                return (void*)(uintptr_t)(start * 4096 + hhdm_offset);
            }
        } else count = 0;
    }
    return NULL;
}

void pmm_free(void* ptr, size_t pages) {
    size_t start = ((uintptr_t)ptr - hhdm_offset) / 4096;
    for (size_t i = start; i < start + pages; i++) pmm_bitmap[i / 8] &= ~(1 << (i % 8));
    pmm_free_pages += pages;
}
