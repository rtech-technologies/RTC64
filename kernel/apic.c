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

volatile uint64_t g_ticks = 0;

void apic_write(uint32_t reg, uint32_t val) {
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

static void apic_spurious_handler(struct cpu_state* state) {
    (void)state;
    /* Spurious interrupts do not require EOI */
}

void apic_init(void) {
    /* Install spurious handler on vector 255 */
    irq_install_handler(255, apic_spurious_handler);

    apic_write(APIC_SVR, apic_read(APIC_SVR) | 0x1FF);
    apic_write(APIC_TDCR, 0x03);
    apic_write(APIC_TMR, 32 | 0x20000);
    apic_write(APIC_TICR, 1000000);
}

uint64_t hal_get_uptime_ms(void) {
    return g_ticks * 10; /* Assuming 100Hz timer */
}

void timer_handler(struct cpu_state* state) {
    apic_eoi();
    g_ticks++;

    if (g_ticks % 1 == 0) {
        usb_osal_tick_handler();
    }

    /* Preemptive context switch is handled by isr_stubs.s common IRQ path
       The scheduler_switch returns the new stack pointer to isr_stubs.s */
    if (state->rip == 0) {
        serial_printf("[APIC] Unexpected architectural state in timer.\n");
    }
}
