/* Copyright (C) 2025 Sovereign RTC64 Project. All rights reserved.
 * Licensed under the 'respect people's property' OS license. */
#include "pro_os.h"
#include "serial.h"
#include <string.h>

extern struct limine_framebuffer *primary_fb;
extern uint64_t hhdm_offset;
extern void draw_glyph(int x, int y, char c, uint32_t color);

static void osod_render(const char* msg, struct cpu_state *state) {
    if (!primary_fb) return;
    uint32_t *fb = (uint32_t*)((uint64_t)primary_fb->address + hhdm_offset);
    uint32_t orange = 0xFF4500;

    for (uint64_t i = 0; i < primary_fb->width * primary_fb->height; i++) fb[i] = orange;

    int x = 20, y = 20;
    const char *title = "!!! SOVEREIGN KERNEL PANIC !!!";
    while (*title) { draw_glyph(x, y, *title++, 0xFFFFFF); x += 8; }

    x = 20; y += 30;
    while (*msg) { draw_glyph(x, y, *msg++, 0xFFFFFF); x += 8; }

    if (state) {
        char buf[128];
        y += 40; x = 20;
        snprintf(buf, sizeof(buf), "RIP: %p  CS: %p  RFLAGS: %p", (void*)state->rip, (void*)state->cs, (void*)state->rflags);
        const char *p = buf; while (*p) { draw_glyph(x, y, *p++, 0xFFFFFF); x += 8; }

        y += 20; x = 20;
        snprintf(buf, sizeof(buf), "RAX: %p  RBX: %p  RCX: %p", (void*)state->rax, (void*)state->rbx, (void*)state->rcx);
        p = buf; while (*p) { draw_glyph(x, y, *p++, 0xFFFFFF); x += 8; }

        y += 20; x = 20;
        snprintf(buf, sizeof(buf), "CR2: %p (Fault Address)", (void*)state->cr2);
        p = buf; while (*p) { draw_glyph(x, y, *p++, 0xFFFFFF); x += 8; }
    }
}

void kpanic(const char* message) {
    serial_printf("\n\n[FATAL] KERNEL PANIC: %s\n", message);
    osod_render(message, NULL);
    __asm__ volatile("cli; hlt");
}

void exception_handler(struct cpu_state *state) {
    if (state->interrupt_number >= 32) {
        extern void timer_handler(struct cpu_state*);
        if (state->interrupt_number == 32) timer_handler(state);
        extern void apic_eoi(void);
        apic_eoi();
        return;
    }

    const char* exc_name = "CPU EXCEPTION";
    if (state->interrupt_number == 14) exc_name = "PAGE FAULT";
    else if (state->interrupt_number == 13) exc_name = "GENERAL PROTECTION FAULT";
    else if (state->interrupt_number == 0) exc_name = "DIVIDE BY ZERO";

    serial_printf("\n[EXCEPTION] %s (%d) at RIP: %p\n", exc_name, (int)state->interrupt_number, (void*)state->rip);
    osod_render(exc_name, state);

    /* Audit for page fault: trigger if unrecoverable or debug */
    if (state->interrupt_number == 14) {
        serial_printf("[AUDIT] Page Fault captured. CR2: %p\n", (void*)state->cr2);
    }

    while(1) { __asm__ volatile("cli; hlt"); }
}
