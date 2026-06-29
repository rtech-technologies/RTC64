/* Copyright (C) 2025 Sovereign RTC64 Project. All rights reserved.
 * Licensed under the 'respect people's property' OS license. */
/* Modified by Sovereign: High-Power Interrupt Descriptor Table (IDT) and Exception Handlers */
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
extern void isr_stub_128(void);

extern int syscall_dispatch(int num, const void* a1, void* a2, size_t a3);

void syscall_handler(struct cpu_state* state) {
    /* SYS_MALLOC and SYS_I18N_TRANSLATE return pointers, others return int.
     * For simplicity in this skeleton, we route all through syscall_dispatch
     * and store result in RAX. */
    state->rax = (uint64_t)syscall_dispatch((int)state->rax, (void*)state->rdi, (void*)state->rsi, (size_t)state->rdx);
}

// Hardware exception gateways from panic.c
extern void handler_divide_by_zero(void);
extern void handler_general_protection_fault(void);
extern void handler_page_fault(void);
extern void handler_double_fault(void);

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
    // Map standard IRQs and generic exceptions
    for (int i = 0; i < 48; i++) {
        idt_set_gate(i, (uint64_t)isr_stub_table[i], 0x08, 0x8E);
    }

    // Override critical hardware exceptions with panic gateways
    idt_set_gate(0,  (uint64_t)handler_divide_by_zero, 0x08, 0x8E);
    idt_set_gate(8,  (uint64_t)handler_double_fault,   0x08, 0x8E);
    idt_set_gate(13, (uint64_t)handler_general_protection_fault, 0x08, 0x8E);
    idt_set_gate(14, (uint64_t)handler_page_fault,    0x08, 0x8E);

    /* System Call Entry: Vector 0x80 */
    idt_set_gate(0x80, (uint64_t)isr_stub_128, 0x08, 0xEE); /* DPL=3 for user-space access */

    idt_ptr.limit = sizeof(idt) - 1;
    idt_ptr.base = (uint64_t)&idt;
    __asm__ volatile ("lidt %0" : : "m"(idt_ptr));
}

typedef void (*irq_handler_t)(struct cpu_state*);
static irq_handler_t irq_handlers[256];

void irq_install_handler(int i, irq_handler_t handler) {
    irq_handlers[i] = handler;
}

extern void apic_eoi(void);
void exception_handler(struct cpu_state *state) {
    if (state->interrupt_number == 128) {
        syscall_handler(state);
        return;
    }
    if (state->interrupt_number >= 32) {
        /* POWER: Centralized IRQ Acknowledgement */
        if (irq_handlers[state->interrupt_number]) {
            irq_handlers[state->interrupt_number](state);
        }
        apic_eoi();
        return;
    }
    serial_printf("[INTERRUPT] Exception %d, Error: %p, RIP: %p\n",
                  (int)state->interrupt_number, (void*)state->error_code, (void*)state->rip);
    kpanic("CPU EXCEPTION TRAP");
}
