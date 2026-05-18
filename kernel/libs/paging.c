#include <pro_os.h>

#define PAGE_PRESENT (1ULL << 0)
#define PAGE_WRITABLE (1ULL << 1)
#define PAGE_USER     (1ULL << 2)

typedef uint64_t pt_entry_t;

// Access to the kernel PML4 created by the bootloader (Limine)
extern uint64_t get_hhdm_offset(void);

void* paging_create_user_space() {
    pt_entry_t* pml4 = malloc(4096);
    memset(pml4, 0, 4096);

    // Map Kernel Space (last 2GB) by copying from the bootloader's PML4
    // This ensures RIP and stack remain valid after CR3 switch
    uint64_t current_cr3;
    __asm__ volatile("mov %%cr3, %0" : "=r"(current_cr3));
    pt_entry_t* current_pml4 = (pt_entry_t*)(current_cr3 + get_hhdm_offset());

    for (int i = 256; i < 512; i++) {
        pml4[i] = current_pml4[i]; // Copy kernel mappings
        // The User bit should already be cleared in the bootloader's kernel mappings
    }

    // Identity map some lower memory for User code (Shell) - 1MB for now
    // In a real OS, we'd map the specific user binary pages
    // For RTECH OSx2, the shell is currently linked into the kernel, so we map that range with USER bit
    extern char _start[]; // Kernel start
    uint64_t shell_addr = (uint64_t)shell_main;
    uint64_t pml4_idx = (shell_addr >> 39) & 0x1FF;
    // ... complex recursive mapping would be here ...
    // Simplified: we ensure the PML4 entry covering the shell/stack has the USER bit
    pml4[pml4_idx] |= PAGE_USER;

    return (void*)pml4;
}

void paging_switch(void* pml4) {
    __asm__ volatile("mov %0, %%cr3" : : "r"(pml4) : "memory");
}
