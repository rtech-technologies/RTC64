/* Copyright (C) 2025 Sovereign RTC64 Project. All rights reserved.
 * Licensed under the 'respect people's property' OS license. */
#include "pro_os.h"
#include "serial.h"
#include <stddef.h>
#include <string.h>

extern struct limine_framebuffer *primary_fb;
void draw_glyph(int x, int y, char c, uint32_t color);

static void draw_string(int x, int y, const char* str, uint32_t color) {
    while (*str) {
        draw_glyph(x, y, *str, color);
        x += 8;
        str++;
    }
}

static void osod_render(const char* message, struct cpu_state *state) {
    if (!primary_fb) return;

    /* Background: Orange (#FF4500) */
    uint32_t *fb = (uint32_t*)primary_fb->address;
    for (uint64_t i = 0; i < primary_fb->width * primary_fb->height; i++) {
        fb[i] = 0xFFFF4500;
    }

    int y = 50;
    draw_string(50, y, "::::::::::::::::::::::::::::::::::::::::::::::::", 0xFFFFFFFF); y += 20;
    draw_string(50, y, "::                                            ::", 0xFFFFFFFF); y += 20;
    draw_string(50, y, "::       SOVEREIGN RTC64 CRITICAL ERROR       ::", 0xFFFFFFFF); y += 20;
    draw_string(50, y, "::                                            ::", 0xFFFFFFFF); y += 20;
    draw_string(50, y, "::::::::::::::::::::::::::::::::::::::::::::::::", 0xFFFFFFFF); y += 40;

    draw_string(50, y, "MESSAGE: ", 0xFFFFFFFF);
    draw_string(130, y, message, 0xFFFFFF00); y += 40;

    if (state) {
        char buf[128];
        snprintf(buf, sizeof(buf), "EXCEPTION: %d  ERR: %016llx", (int)state->interrupt_number, (unsigned long long)state->error_code);
        draw_string(50, y, buf, 0xFFFFFFFF); y += 20;
        snprintf(buf, sizeof(buf), "RIP: %016llx  CS: %llx  FLG: %016llx", (unsigned long long)state->rip, (unsigned long long)state->cs, (unsigned long long)state->rflags);
        draw_string(50, y, buf, 0xFFFFFFFF); y += 30;

        snprintf(buf, sizeof(buf), "RAX: %016llx  RBX: %016llx", (unsigned long long)state->rax, (unsigned long long)state->rbx);
        draw_string(50, y, buf, 0xFFFFFFFF); y += 20;
        snprintf(buf, sizeof(buf), "RCX: %016llx  RDX: %016llx", (unsigned long long)state->rcx, (unsigned long long)state->rdx);
        draw_string(50, y, buf, 0xFFFFFFFF); y += 20;
        snprintf(buf, sizeof(buf), "RSI: %016llx  RDI: %016llx", (unsigned long long)state->rsi, (unsigned long long)state->rdi);
        draw_string(50, y, buf, 0xFFFFFFFF); y += 20;
        snprintf(buf, sizeof(buf), "RBP: %016llx  RSP: %016llx", (unsigned long long)state->rbp, (unsigned long long)state->rsp);
        draw_string(50, y, buf, 0xFFFFFFFF); y += 40;
    }

    draw_string(50, y, "THE SYSTEM HAS BEEN HALTED TO PREVENT DAMAGE.", 0xFFFFFFFF); y += 20;
    draw_string(50, y, "PLEASE REBOOT YOUR MACHINE.", 0xFFFFFFFF);
}

void kpanic(const char* message) {
    serial_printf("\n\n!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
    serial_printf("!!! KERNEL PANIC: %s\n", message);
    serial_printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");

    osod_render(message, NULL);

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

    osod_render("CPU EXCEPTION TRAP", state);

    __asm__ volatile("cli");
    while(1) { __asm__ volatile("hlt"); }
}

void handler_divide_by_zero(void) { kpanic("DIVIDE_BY_ZERO"); }
void handler_double_fault(void) { kpanic("DOUBLE_FAULT"); }
void handler_general_protection_fault(void) { kpanic("GENERAL_PROTECTION_FAULT"); }
void handler_page_fault(void) { kpanic("PAGE_FAULT"); }
