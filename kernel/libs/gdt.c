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
    uint16_t limit;
    uint64_t base;
} __attribute__((packed)) gdt_ptr_t;

static gdt_entry_t gdt[5];
static gdt_ptr_t gdt_ptr;

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
    gdt_ptr.limit = (sizeof(gdt_entry_t) * 5) - 1;
    gdt_ptr.base  = (uint64_t)&gdt;

    gdt_set_entry(0, 0, 0, 0, 0);                // Null
    gdt_set_entry(1, 0, 0xFFFFFFFF, 0x9A, 0xA0); // Kernel Code (Ring 0)
    gdt_set_entry(2, 0, 0xFFFFFFFF, 0x92, 0xA0); // Kernel Data (Ring 0)
    gdt_set_entry(3, 0, 0xFFFFFFFF, 0xFA, 0xA0); // User Code (Ring 3)
    gdt_set_entry(4, 0, 0xFFFFFFFF, 0xF2, 0xA0); // User Data (Ring 3)

    __asm__ volatile("lgdt %0" : : "m"(gdt_ptr));

    // Reload segment registers
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
