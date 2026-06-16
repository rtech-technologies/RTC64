/* Modified by Sovereign: Robust 64-bit GDT implementation for High-Power Mode */
#include <stdint.h>
#include "pro_os.h"
#include <string.h>
#include "serial.h"

typedef struct {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t  base_middle;
    uint8_t  access;
    uint8_t  granularity;
    uint8_t  base_high;
} __attribute__((packed)) gdt_entry_t;

typedef struct {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t  base_middle;
    uint8_t  access;
    uint8_t  granularity;
    uint8_t  base_high;
    uint32_t base_upper;
    uint32_t reserved;
} __attribute__((packed)) gdt_tss_entry_t;

typedef struct {
    uint32_t reserved0;
    uint64_t rsp0;
    uint64_t rsp1;
    uint64_t rsp2;
    uint64_t reserved1;
    uint64_t ist1;
    uint64_t ist2;
    uint64_t ist3;
    uint64_t ist4;
    uint64_t ist5;
    uint64_t ist6;
    uint64_t ist7;
    uint64_t reserved2;
    uint16_t reserved3;
    uint16_t iopb_offset;
} __attribute__((packed)) tss_t;

typedef struct {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed)) gdt_ptr_t;

static struct {
    gdt_entry_t null;
    gdt_entry_t kernel_code;
    gdt_entry_t kernel_data;
    gdt_entry_t user_code;
    gdt_entry_t user_data;
    gdt_tss_entry_t tss;
} __attribute__((packed, aligned(4096))) gdt;

static gdt_ptr_t gdt_ptr;
static tss_t tss __attribute__((aligned(16)));

static uint8_t double_fault_stack[16384] __attribute__((aligned(4096)));

void gdt_init(void) {
    memset(&gdt, 0, sizeof(gdt));

    /* Kernel Code 64: Access 0x9A, Granularity 0xAF */
    gdt.kernel_code = (gdt_entry_t){0, 0, 0, 0x9A, 0xAF, 0};
    /* Kernel Data 64: Access 0x92, Granularity 0xCF */
    gdt.kernel_data = (gdt_entry_t){0, 0, 0, 0x92, 0xCF, 0};
    /* User Code 64: Access 0xFA, Granularity 0xAF */
    gdt.user_code = (gdt_entry_t){0, 0, 0, 0xFA, 0xAF, 0};
    /* User Data 64: Access 0xF2, Granularity 0xCF */
    gdt.user_data = (gdt_entry_t){0, 0, 0, 0xF2, 0xCF, 0};

    /* Setup TSS */
    memset(&tss, 0, sizeof(tss));
    uint64_t tss_base = (uint64_t)&tss;
    uint32_t tss_limit = sizeof(tss) - 1;

    gdt.tss.limit_low = tss_limit & 0xFFFF;
    gdt.tss.base_low = tss_base & 0xFFFF;
    gdt.tss.base_middle = (tss_base >> 16) & 0xFF;
    gdt.tss.access = 0x89; /* Present, TSS type */
    gdt.tss.granularity = (tss_limit >> 16) & 0x0F;
    gdt.tss.base_high = (tss_base >> 24) & 0xFF;
    gdt.tss.base_upper = (tss_base >> 32) & 0xFFFFFFFF;

    /* Setup IST1 for Double Fault (Vector 8) */
    tss.ist1 = (uint64_t)&double_fault_stack[sizeof(double_fault_stack)];

    serial_printf("[GDT] DEBUG: TSS Base at %p, IST1 Stack at %p\n", &tss, (void*)tss.ist1);

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
        "mov $0x28, %%ax\n\t"
        "ltr %%ax\n\t"
        : : "m"(gdt_ptr) : "rax", "memory"
    );
}
