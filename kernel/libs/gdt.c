#include <pro_os.h>

typedef struct {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t  base_middle;
    uint8_t  access;
    uint8_t  granularity;
    uint8_t  base_high;
} __attribute__((packed)) gdt_entry_t;

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

static gdt_entry_t gdt[7]; // Null, KCode, KData, UCode, UData, TSS (2 entries)
static tss_t tss;
static gdt_ptr_t gdt_ptr;

static uint8_t kernel_stack[16384];

void gdt_set_entry(int num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran) {
    gdt[num].base_low    = (base & 0xFFFF);
    gdt[num].base_middle = (base >> 16) & 0xFF;
    gdt[num].base_high   = (base >> 24) & 0xFF;
    gdt[num].limit_low   = (limit & 0xFFFF);
    gdt[num].granularity = (limit >> 16) & 0x0F;
    gdt[num].granularity |= gran & 0xF0;
    gdt[num].access      = access;
}

void gdt_init() {
    gdt_ptr.limit = (sizeof(gdt_entry_t) * 7) - 1;
    gdt_ptr.base  = (uint64_t)&gdt;

    gdt_set_entry(0, 0, 0, 0, 0);                // Null
    gdt_set_entry(1, 0, 0xFFFFFFFF, 0x9A, 0xA0); // Kernel Code (0x08)
    gdt_set_entry(2, 0, 0xFFFFFFFF, 0x92, 0xA0); // Kernel Data (0x10)
    gdt_set_entry(3, 0, 0xFFFFFFFF, 0xFA, 0xA0); // User Code   (0x1B)
    gdt_set_entry(4, 0, 0xFFFFFFFF, 0xF2, 0xA0); // User Data   (0x23)

    // TSS
    uint64_t tss_base = (uint64_t)&tss;
    gdt_set_entry(5, tss_base & 0xFFFFFFFF, sizeof(tss) - 1, 0x89, 0x40);
    *(uint64_t*)&gdt[6] = (tss_base >> 32);

    memset(&tss, 0, sizeof(tss));
    tss.rsp0 = (uint64_t)kernel_stack + sizeof(kernel_stack);

    __asm__ volatile("lgdt %0" : : "m"(gdt_ptr));
    __asm__ volatile("ltr %%ax" : : "a"(0x28)); // 5th entry * 8

    // Segment reload
    __asm__ volatile(
        "push $0x10\n"
        "push %%rsp\n"
        "pushf\n"
        "push $0x08\n"
        "push $1f\n"
        "iretq\n"
        "1:\n"
        "mov $0x10, %%ax\n"
        "mov %%ax, %%ds\n"
        "mov %%ax, %%es\n"
        "mov %%ax, %%fs\n"
        "mov %%ax, %%gs\n"
        : : : "rax", "memory"
    );
}
