/* Modified by Sovereign: Robust 64-bit GDT implementation for High-Power Mode */
#include <stdint.h>
#include "pro_os.h"

typedef struct {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t  base_middle;
    uint8_t  access;
    uint8_t  granularity;
    uint8_t  base_high;
} __attribute__((packed)) gdt_entry_t;

typedef struct {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed)) gdt_ptr_t;

static gdt_entry_t gdt[5];
static gdt_ptr_t gdt_ptr;

void gdt_init(void) {
    /* Null descriptor */
    memset(&gdt[0], 0, sizeof(gdt_entry_t));
    /* Kernel Code 64: Access 0x9A, Granularity 0xAF */
    gdt[1] = (gdt_entry_t){0, 0, 0, 0x9A, 0xAF, 0};
    /* Kernel Data 64: Access 0x92, Granularity 0xCF */
    gdt[2] = (gdt_entry_t){0, 0, 0, 0x92, 0xCF, 0};
    /* User Code 64: Access 0xFA, Granularity 0xAF */
    gdt[3] = (gdt_entry_t){0, 0, 0, 0xFA, 0xAF, 0};
    /* User Data 64: Access 0xF2, Granularity 0xCF */
    gdt[4] = (gdt_entry_t){0, 0, 0, 0xF2, 0xCF, 0};

    gdt_ptr.limit = sizeof(gdt) - 1;
    gdt_ptr.base = (uint64_t)&gdt;

    __asm__ volatile (
        "lgdt %0\n\t"
        "push $0x08\n\t"
        "lea 1f(%%rip), %%rax\n\t"
        "push %%rax\n\t"
        "lretq\n"
        "1:\n\t"
        "mov $0x10, %%ax\n\t"
        "mov %%ax, %%ds\n\t"
        "mov %%ax, %%es\n\t"
        "mov %%ax, %%fs\n\t"
        "mov %%ax, %%gs\n\t"
        "mov %%ax, %%ss\n\t"
        : : "m"(gdt_ptr) : "rax", "memory"
    );
}
