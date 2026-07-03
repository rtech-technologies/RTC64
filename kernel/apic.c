/* Copyright (C) 2025 Sovereign RTC64 Project. All rights reserved.
 * Licensed under the 'respect people's property' OS license. */
/* Modified by Sovereign: Meaty APIC and Timer implementation with Uptime tracking */
#include "pro_os.h"
#include <stdint.h>
#include "serial.h"

#define APIC_BASE 0xFEE00000
#define APIC_EOI   0xB0
#define APIC_SVR   0xF0
#define APIC_TMR   0x320
#define APIC_TDCR  0x3E0
#define APIC_TICR  0x380

extern uint64_t hhdm_offset;
extern uint64_t scheduler_switch(uint64_t current_rsp);

volatile uint64_t g_ticks = 0;

static void apic_write(uint32_t reg, uint32_t val) {
    volatile uint32_t* addr = (volatile uint32_t*)(APIC_BASE + hhdm_offset + reg);
    *addr = val;
}

static uint32_t apic_read(uint32_t reg) {
    volatile uint32_t* addr = (volatile uint32_t*)(APIC_BASE + hhdm_offset + reg);
    return *addr;
}

void apic_eoi(void) {
    apic_write(APIC_EOI, 0);
}

void apic_init(void) {
    /* Step 1: Secure Spurious Vector and Software Enable */
    apic_write(APIC_SVR, apic_read(APIC_SVR) | 0x1FF);

    /* Step 2: Calibrate Divider (Divide by 16) */
    apic_write(APIC_TDCR, 0x03);

    /* Step 3: Configure Timer (Vector 32, Periodic mode, start MASKED) */
    apic_write(APIC_TMR, 32 | 0x20000 | 0x10000);

    /* Step 4: Set Initial Count */
    apic_write(APIC_TICR, 1000000);
}

uint64_t hal_get_uptime_ms(void) {
    return g_ticks * 10; /* Assuming 100Hz timer */
}

void timer_handler(struct cpu_state* state) {
    (void)state;
    /* EOI now handled centrally in exception_handler */
    g_ticks++;
}

void apic_timer_unmask(void) {
    serial_printf("[APIC] Final check: Unmasking Local APIC Timer.\n");
    uint32_t val = apic_read(APIC_TMR);
    val &= ~0x10000; /* Clear Mask bit */
    apic_write(APIC_TMR, val);
}
