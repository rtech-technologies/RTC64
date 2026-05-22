#include "idt.h"
#include <string.h>
static struct idt_entry idt[256]; static struct idtr idtr;
#define ISR_EXT(n) extern void isr##n##_stub(void);
ISR_EXT(0) ISR_EXT(1) ISR_EXT(2) ISR_EXT(3) ISR_EXT(4) ISR_EXT(5) ISR_EXT(6) ISR_EXT(7)
ISR_EXT(8) ISR_EXT(9) ISR_EXT(10) ISR_EXT(11) ISR_EXT(12) ISR_EXT(13) ISR_EXT(14) ISR_EXT(15)
ISR_EXT(16) ISR_EXT(17) ISR_EXT(18) ISR_EXT(19) ISR_EXT(20) ISR_EXT(21) ISR_EXT(22) ISR_EXT(23)
ISR_EXT(24) ISR_EXT(25) ISR_EXT(26) ISR_EXT(27) ISR_EXT(28) ISR_EXT(29) ISR_EXT(30) ISR_EXT(31)
static void idt_set(uint8_t v, void* isr) {
    struct idt_entry* e = &idt[v]; uint64_t a = (uint64_t)isr;
    e->isr_low = a & 0xFFFF; e->kernel_cs = 0x08; e->ist = 0; e->attributes = 0x8E;
    e->isr_mid = (a >> 16) & 0xFFFF; e->isr_high = (a >> 32) & 0xFFFFFFFF; e->reserved = 0;
}
void idt_init(void) {
    memset(idt, 0, sizeof(idt));
    idt_set(0, isr0_stub); idt_set(1, isr1_stub); idt_set(2, isr2_stub); idt_set(3, isr3_stub);
    idt_set(4, isr4_stub); idt_set(5, isr5_stub); idt_set(6, isr6_stub); idt_set(7, isr7_stub);
    idt_set(8, isr8_stub); idt_set(9, isr9_stub); idt_set(10, isr10_stub); idt_set(11, isr11_stub);
    idt_set(12, isr12_stub); idt_set(13, isr13_stub); idt_set(14, isr14_stub); idt_set(15, isr15_stub);
    idt_set(16, isr16_stub); idt_set(17, isr17_stub); idt_set(18, isr18_stub); idt_set(19, isr19_stub);
    idt_set(20, isr20_stub); idt_set(21, isr21_stub); idt_set(22, isr22_stub); idt_set(23, isr23_stub);
    idt_set(24, isr24_stub); idt_set(25, isr25_stub); idt_set(26, isr26_stub); idt_set(27, isr27_stub);
    idt_set(28, isr28_stub); idt_set(29, isr29_stub); idt_set(30, isr30_stub); idt_set(31, isr31_stub);
    idtr.limit = sizeof(idt)-1; idtr.base = (uint64_t)&idt;
    __asm__ volatile ("lidt %0" : : "m"(idtr));
}
