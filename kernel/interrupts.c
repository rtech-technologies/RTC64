/* Copyright (C) 2025 Sovereign RTC64 Project. All rights reserved.
 * Licensed under the 'respect people's property' OS license. */
#include <stdint.h>
#include "pro_os.h"
#include "serial.h"

typedef struct {
    uint16_t offset_low;
    uint16_t selector;
    uint8_t  ist;
    uint8_t  type_attr;
    uint16_t offset_mid;
    uint32_t offset_high;
    uint32_t zero;
} __attribute__((packed)) idt_entry_t;

typedef struct {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed)) idt_ptr_t;

static idt_entry_t idt[256];
static idt_ptr_t idt_ptr;

extern void* isr_stub_table[];

void idt_set_gate(uint8_t num, uint64_t base, uint16_t sel, uint8_t flags) {
    idt[num].offset_low = base & 0xFFFF;
    idt[num].selector = sel;
    idt[num].ist = 0;
    idt[num].type_attr = flags;
    idt[num].offset_mid = (base >> 16) & 0xFFFF;
    idt[num].offset_high = (base >> 32) & 0xFFFFFFFF;
    idt[num].zero = 0;
}

void idt_init(void) {
    for (int i = 0; i < 48; i++) {
        idt_set_gate(i, (uint64_t)isr_stub_table[i], 0x08, 0x8E);
    }

    idt_ptr.limit = sizeof(idt) - 1;
    idt_ptr.base = (uint64_t)&idt;
    __asm__ volatile ("lidt %0" : : "m"(idt_ptr));
}

typedef void (*irq_handler_t)(struct cpu_state*);
static irq_handler_t irq_handlers[256];

void irq_install_handler(int i, irq_handler_t handler) {
    irq_handlers[i] = handler;
}

extern void exception_handler_panic(struct cpu_state *state);

void exception_handler(struct cpu_state *state) {
    if (state->interrupt_number >= 32) {
        if (irq_handlers[state->interrupt_number]) {
            irq_handlers[state->interrupt_number](state);
        }
        return;
    }

    /* Dedicated panic handler for CPU exceptions */
    exception_handler_panic(state);
}
