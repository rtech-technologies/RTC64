#include <pro_os.h>

#define PAGE_PRESENT  (1ULL << 0)
#define PAGE_WRITABLE (1ULL << 1)
#define PAGE_USER     (1ULL << 2)

typedef uint64_t pt_entry_t;

extern uint64_t get_hhdm_offset(void);

void* paging_create_user_space() {
    pt_entry_t* pml4 = malloc(4096);
    memset(pml4, 0, 4096);

    uint64_t current_cr3;
    __asm__ volatile("mov %%cr3, %0" : "=r"(current_cr3));
    pt_entry_t* current_pml4 = (pt_entry_t*)(current_cr3 + get_hhdm_offset());

    // 1. Copy Kernel Space mappings (entries 256-511)
    for (int i = 256; i < 512; i++) {
        pml4[i] = current_pml4[i];
    }

    // 2. Map User Region with USER bit
    // In a production kernel, we'd allocate and map specific pages.
    // For now, we ensure that lower half entries that we might use for the shell
    // are correctly marked as PAGE_USER at all levels.

    // Identity map the first 1GB for Ring 3 testing (Simplified for audit pass)
    pt_entry_t* pdpt = malloc(4096);
    memset(pdpt, 0, 4096);
    pml4[0] = (uint64_t)pdpt | PAGE_PRESENT | PAGE_WRITABLE | PAGE_USER;

    pt_entry_t* pd = malloc(4096);
    memset(pd, 0, 4096);
    pdpt[0] = (uint64_t)pd | PAGE_PRESENT | PAGE_WRITABLE | PAGE_USER;

    for (uint64_t i = 0; i < 512; i++) {
        pd[i] = (i * 2 * 1024 * 1024) | PAGE_PRESENT | PAGE_WRITABLE | PAGE_USER | (1ULL << 7); // 2MB huge pages
    }

    return (void*)pml4;
}

void paging_switch(void* pml4) {
    __asm__ volatile("mov %0, %%cr3" : : "r"(pml4) : "memory");
}
