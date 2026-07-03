/* Copyright (C) 2025 Sovereign RTC64 Project. All rights reserved.
 * Licensed under the 'respect people's property' OS license. */
#include <stdint.h>
#include "pro_os.h"

typedef struct {
    uint16_t offset_low, selector;
    uint8_t ist, type_attr;
    uint16_t offset_mid;
    uint32_t offset_high, zero;
} __attribute__((packed)) idt_entry_t;

typedef struct { uint16_t limit; uint64_t base; } __attribute__((packed)) idt_ptr_t;

static idt_entry_t idt[256];
static idt_ptr_t idt_ptr;
extern void* isr_stub_table[];
extern void isr_stub_128(void);

void idt_set_gate(uint8_t num, uint64_t base, uint16_t sel, uint8_t flags) {
    idt[num].offset_low = base & 0xFFFF;
    idt[num].selector = sel; idt[num].ist = 0; idt[num].type_attr = flags;
    idt[num].offset_mid = (base >> 16) & 0xFFFF;
    idt[num].offset_high = (base >> 32) & 0xFFFFFFFF;
    idt[num].zero = 0;
}

void idt_init(void) {
    for (int i = 0; i < 48; i++) idt_set_gate(i, (uint64_t)isr_stub_table[i], 0x08, 0x8E);
    idt_set_gate(128, (uint64_t)isr_stub_128, 0x08, 0x8E);
    idt_ptr.limit = sizeof(idt) - 1;
    idt_ptr.base = (uint64_t)&idt;
    __asm__ volatile ("lidt %0" : : "m"(idt_ptr));
}
