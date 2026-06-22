/* Copyright (C) 2025 Sovereign RTC64 Project. All rights reserved.
 * Licensed under the 'respect people's property' OS license. */
#include "pro_os.h"
#include "serial.h"
#include <stddef.h>

void kpanic(const char* message) {
    serial_printf("\n\n!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
    serial_printf("!!! KERNEL PANIC: %s\n", message);
    serial_printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");

    /* Architectural State securing */
    __asm__ volatile("cli");
    serial_printf("ESTATE SECURED. EXECUTION HALTED SAFELY.\n");
    while(1) { __asm__ volatile("hlt"); }
}

void exception_handler_panic(struct cpu_state *state) {
    serial_printf("\n\n!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
    serial_printf("!!! KERNEL PANIC: EXCEPTION #%d\n", (int)state->interrupt_number);
    serial_printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
    serial_printf("RIP: %p  ERR: %016llx\n", (void*)state->rip, (unsigned long long)state->error_code);
    serial_printf("RAX: %p  RBX: %p  RCX: %p\n", (void*)state->rax, (void*)state->rbx, (void*)state->rcx);
    serial_printf("RDX: %p  RSI: %p  RDI: %p\n", (void*)state->rdx, (void*)state->rsi, (void*)state->rdi);
    serial_printf("RBP: %p  RSP: %p  FLG: %016llx\n", (void*)state->rbp, (void*)state->rsp, (unsigned long long)state->rflags);

    kpanic("CPU EXCEPTION TRAP");
}

void handler_divide_by_zero(void) { kpanic("DIVIDE_BY_ZERO"); }
void handler_double_fault(void) { kpanic("DOUBLE_FAULT"); }
void handler_general_protection_fault(void) { kpanic("GENERAL_PROTECTION_FAULT"); }
void handler_page_fault(void) { kpanic("PAGE_FAULT"); }
