#include "idt.h"
#include <string.h>

static struct idt_entry idt[256];
static struct idtr idtr;

extern void isr0_stub(void); extern void isr1_stub(void); extern void isr2_stub(void); extern void isr3_stub(void);
extern void isr4_stub(void); extern void isr5_stub(void); extern void isr6_stub(void); extern void isr7_stub(void);
extern void isr8_stub(void); extern void isr9_stub(void); extern void isr10_stub(void); extern void isr11_stub(void);
extern void isr12_stub(void); extern void isr13_stub(void); extern void isr14_stub(void); extern void isr15_stub(void);
extern void isr16_stub(void); extern void isr17_stub(void); extern void isr18_stub(void); extern void isr19_stub(void);
extern void isr20_stub(void); extern void isr21_stub(void); extern void isr22_stub(void); extern void isr23_stub(void);
extern void isr24_stub(void); extern void isr25_stub(void); extern void isr26_stub(void); extern void isr27_stub(void);
extern void isr28_stub(void); extern void isr29_stub(void); extern void isr30_stub(void); extern void isr31_stub(void);

static void idt_set_descriptor(uint8_t vector, void* isr, uint8_t flags) {
    struct idt_entry* entry = &idt[vector];
    uint64_t addr = (uint64_t)isr;

    entry->isr_low = addr & 0xFFFF;
    entry->kernel_cs = 0x08; // Kernel Code Segment
    entry->ist = 0;
    entry->attributes = flags;
    entry->isr_mid = (addr >> 16) & 0xFFFF;
    entry->isr_high = (addr >> 32) & 0xFFFFFFFF;
    entry->reserved = 0;
}

void idt_init(void) {
    memset(idt, 0, sizeof(idt));

    // IDT Attributes: 0x8E (Interrupt Gate, Present, Ring 0)
    idt_set_descriptor(0, isr0_stub, 0x8E);
    idt_set_descriptor(1, isr1_stub, 0x8E);
    idt_set_descriptor(2, isr2_stub, 0x8E);
    idt_set_descriptor(3, isr3_stub, 0x8E);
    idt_set_descriptor(4, isr4_stub, 0x8E);
    idt_set_descriptor(5, isr5_stub, 0x8E);
    idt_set_descriptor(6, isr6_stub, 0x8E);
    idt_set_descriptor(7, isr7_stub, 0x8E);
    idt_set_descriptor(8, isr8_stub, 0x8E);
    idt_set_descriptor(9, isr9_stub, 0x8E);
    idt_set_descriptor(10, isr10_stub, 0x8E);
    idt_set_descriptor(11, isr11_stub, 0x8E);
    idt_set_descriptor(12, isr12_stub, 0x8E);
    idt_set_descriptor(13, isr13_stub, 0x8E);
    idt_set_descriptor(14, isr14_stub, 0x8E);
    idt_set_descriptor(15, isr15_stub, 0x8E);
    idt_set_descriptor(16, isr16_stub, 0x8E);
    idt_set_descriptor(17, isr17_stub, 0x8E);
    idt_set_descriptor(18, isr18_stub, 0x8E);
    idt_set_descriptor(19, isr19_stub, 0x8E);
    idt_set_descriptor(20, isr20_stub, 0x8E);
    idt_set_descriptor(21, isr21_stub, 0x8E);
    idt_set_descriptor(22, isr22_stub, 0x8E);
    idt_set_descriptor(23, isr23_stub, 0x8E);
    idt_set_descriptor(24, isr24_stub, 0x8E);
    idt_set_descriptor(25, isr25_stub, 0x8E);
    idt_set_descriptor(26, isr26_stub, 0x8E);
    idt_set_descriptor(27, isr27_stub, 0x8E);
    idt_set_descriptor(28, isr28_stub, 0x8E);
    idt_set_descriptor(29, isr29_stub, 0x8E);
    idt_set_descriptor(30, isr30_stub, 0x8E);
    idt_set_descriptor(31, isr31_stub, 0x8E);

    idtr.limit = sizeof(idt) - 1;
    idtr.base = (uint64_t)&idt;

    __asm__ volatile ("lidt %0" : : "m"(idtr));
}
