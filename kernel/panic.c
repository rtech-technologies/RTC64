/* Copyright (C) 2025 Sovereign RTC64 Project. All rights reserved.
 * Licensed under the 'respect people's property' OS license. */
#include <stdint.h>
#include <stddef.h>
#include "pro_os.h"
#include "serial.h"

extern struct limine_framebuffer *primary_fb;

/* Static recursion guard */
static int in_panic = 0;

/* Self-contained minimal font for the panic screen */
static const uint8_t panic_font[128][8] = {
    ['!']={0x18,0x18,0x18,0x18,0x00,0x00,0x18,0x00},
    [':']={0x00,0x18,0x18,0x00,0x18,0x18,0x00,0x00},
    [' ']={0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
    ['-']={0x00,0x00,0x00,0x7E,0x00,0x00,0x00,0x00},
    ['(']={0x0C,0x18,0x30,0x30,0x30,0x18,0x0C,0x00},
    [')']={0x30,0x18,0x0C,0x0C,0x0C,0x18,0x30,0x00},
    ['0']={0x3C,0x66,0x6E,0x76,0x66,0x66,0x3C,0x00},
    ['1']={0x18,0x38,0x18,0x18,0x18,0x18,0x7E,0x00},
    ['2']={0x3C,0x66,0x06,0x0C,0x30,0x60,0x7E,0x00},
    ['3']={0x3C,0x66,0x06,0x1C,0x06,0x66,0x3C,0x00},
    ['4']={0x0C,0x1C,0x2C,0x4C,0x7E,0x0C,0x0C,0x00},
    ['5']={0x7E,0x60,0x7C,0x06,0x06,0x66,0x3C,0x00},
    ['6']={0x3C,0x66,0x60,0x7C,0x66,0x66,0x3C,0x00},
    ['7']={0x7E,0x06,0x0C,0x18,0x30,0x30,0x30,0x00},
    ['8']={0x3C,0x66,0x66,0x3C,0x66,0x66,0x3C,0x00},
    ['9']={0x3C,0x66,0x66,0x3E,0x06,0x66,0x3C,0x00},
    ['A']={0x18,0x3C,0x66,0x7E,0x66,0x66,0x66,0x00},
    ['B']={0x7C,0x66,0x66,0x7C,0x66,0x66,0x7C,0x00},
    ['C']={0x3C,0x66,0x60,0x60,0x60,0x66,0x3C,0x00},
    ['D']={0x78,0x6C,0x66,0x66,0x66,0x6C,0x78,0x00},
    ['E']={0x7E,0x60,0x60,0x78,0x60,0x60,0x7E,0x00},
    ['F']={0x7E,0x60,0x60,0x78,0x60,0x60,0x60,0x00},
    ['G']={0x3C,0x66,0x60,0x6E,0x66,0x66,0x3C,0x00},
    ['H']={0x66,0x66,0x66,0x7E,0x66,0x66,0x66,0x00},
    ['I']={0x3C,0x18,0x18,0x18,0x18,0x18,0x3C,0x00},
    ['J']={0x06,0x06,0x06,0x06,0x06,0x66,0x3C,0x00},
    ['K']={0x66,0x6C,0x78,0x70,0x78,0x6C,0x66,0x00},
    ['L']={0x60,0x60,0x60,0x60,0x60,0x60,0x7E,0x00},
    ['M']={0x63,0x77,0x7F,0x6B,0x63,0x63,0x63,0x00},
    ['N']={0x66,0x76,0x7E,0x7E,0x6E,0x66,0x66,0x00},
    ['O']={0x3C,0x66,0x66,0x66,0x66,0x66,0x3C,0x00},
    ['P']={0x7C,0x66,0x66,0x7C,0x60,0x60,0x60,0x00},
    ['Q']={0x3C,0x66,0x66,0x66,0x66,0x3C,0x0E,0x00},
    ['R']={0x7C,0x66,0x66,0x7C,0x6C,0x66,0x66,0x00},
    ['S']={0x3C,0x66,0x60,0x3C,0x06,0x66,0x3C,0x00},
    ['T']={0x7E,0x18,0x18,0x18,0x18,0x18,0x18,0x00},
    ['U']={0x66,0x66,0x66,0x66,0x66,0x66,0x3C,0x00},
    ['V']={0x66,0x66,0x66,0x66,0x66,0x3C,0x18,0x00},
    ['W']={0x63,0x63,0x63,0x6B,0x7F,0x77,0x63,0x00},
    ['X']={0x66,0x66,0x3C,0x18,0x3C,0x66,0x66,0x00},
    ['Y']={0x66,0x66,0x66,0x3C,0x18,0x18,0x18,0x00},
    ['Z']={0x7E,0x06,0x0C,0x18,0x30,0x60,0x7E,0x00},
    ['x']={0x00,0x00,0x66,0x3C,0x18,0x3C,0x66,0x00},
};

static void panic_putc(int x, int y, char c, uint32_t color) {
    if (!primary_fb || !primary_fb->address) return;
    uint32_t *fb = (uint32_t*)primary_fb->address;
    const uint8_t *glyph = panic_font[(uint8_t)c];
    uint32_t pitch = primary_fb->pitch / 4;
    for (int gy = 0; gy < 8; gy++) {
        for (int gx = 0; gx < 8; gx++) {
            if (glyph[gy] & (0x80 >> gx)) {
                fb[(y + gy) * pitch + (x + gx)] = color;
            }
        }
    }
}

static void panic_puts(int x, int y, const char *s, uint32_t color) {
    while (*s) {
        panic_putc(x, y, *s++, color);
        x += 8;
    }
}

static void itohex(uint64_t n, char *buf) {
    const char *hex = "0123456789ABCDEF";
    buf[0] = '0'; buf[1] = 'x';
    for (int i = 0; i < 16; i++) {
        buf[17 - i] = hex[n & 0xF];
        n >>= 4;
    }
    buf[18] = 0;
}

static void draw_osod(const char* msg, struct cpu_state *state) {
    if (!primary_fb || !primary_fb->address) {
        serial_write(" [FB_MISSING] ");
        return;
    }

    uint32_t *fb = (uint32_t*)primary_fb->address;
    uint32_t orange = 0xFF4500;
    uint32_t white = 0xFFFFFF;

    for (uint64_t i = 0; i < primary_fb->width * primary_fb->height; i++) fb[i] = orange;

    int x = 20, y = 20;
    panic_puts(x, y, "!!! SOVEREIGN INDUSTRIAL KERNEL PANIC !!!", white);
    y += 20;
    panic_puts(x, y, "REASON:", white);
    panic_puts(x + 56, y, msg, white);

    if (state) {
        char hexbuf[32];
        y += 40;
        panic_puts(x, y, "ARCHITECTURAL STATE:", white);

        y += 20; panic_puts(x, y, "RIP: ", white); itohex(state->rip, hexbuf); panic_puts(x + 40, y, hexbuf, white);
        y += 10; panic_puts(x, y, "RAX: ", white); itohex(state->rax, hexbuf); panic_puts(x + 40, y, hexbuf, white);
        y += 10; panic_puts(x, y, "RBX: ", white); itohex(state->rbx, hexbuf); panic_puts(x + 40, y, hexbuf, white);
        y += 10; panic_puts(x, y, "RCX: ", white); itohex(state->rcx, hexbuf); panic_puts(x + 40, y, hexbuf, white);
        y += 10; panic_puts(x, y, "CR2: ", white); itohex(state->cr2, hexbuf); panic_puts(x + 40, y, hexbuf, white);
        y += 10; panic_puts(x, y, "ERR: ", white); itohex(state->error_code, hexbuf); panic_puts(x + 40, y, hexbuf, white);
    }
}

void kpanic(const char* message) {
    if (__sync_lock_test_and_set(&in_panic, 1)) {
        serial_write("\n[DOUBLE_PANIC] Halting.\n");
        while(1) __asm__ volatile("cli; hlt");
    }

    scheduler_stop_all();

    serial_write("\n\n[PANIC] ");
    serial_write(message);
    serial_write("\n");

    draw_osod(message, NULL);
    while(1) __asm__ volatile("cli; hlt");
}

extern void timer_handler(struct cpu_state*);
extern void apic_eoi(void);
extern void handle_registered_irq(unsigned int irq, struct cpu_state *state);

void exception_handler(struct cpu_state *state) {
    if (state->interrupt_number >= 32) {
        if (state->interrupt_number == 32) {
            timer_handler(state);
        } else {
            handle_registered_irq((unsigned int)state->interrupt_number, state);
        }
        apic_eoi();
        return;
    }
    /* If a regular task triggered this exception, try to contain it
     * by redirecting execution to a cleanup routine that will remove
     * the task and yield the CPU, instead of hard-panicking the kernel.
     * However, if the task fails because of a critical CPU error (page fault,
     * or faults that could cause triple faults, like #PF, #GP, #DF, #SS, #NP, #TS),
     * we cause a hard panic instead of containing it. */
    int cur = scheduler_get_current_task_idx();
    if (cur > 0) {
        uint64_t i_num = state->interrupt_number;
        if (i_num == 14 || i_num == 13 || i_num == 8 || i_num == 12 || i_num == 11 || i_num == 10) {
            serial_printf("[CRITICAL] Task %d failed due to critical CPU error %llu. Triggering hard panic.\n", cur, i_num);
        } else {
            serial_printf("[TASK_FAULT] int=%llu err=%llu in task id=%d RIP=%p\n",
                          state->interrupt_number, state->error_code, cur, (void*)state->rip);
            /* Attempt to write a crash report to persistent storage for user inspection */
            const char *crash_dir = "/crash_reports";
            vfs_mkdir(crash_dir);
            uint32_t upid = scheduler_get_current_upid();
            uint64_t now = hal_get_uptime_ms();
            char path[128];
            snprintf(path, sizeof(path), "/crash_reports/crash_%u_%llu.txt", (unsigned)upid, (unsigned long long)now);
            char report[1024];
            int r = snprintf(report, sizeof(report),
                             "Crash Report\nINT=%llu ERR=%llu TASK_ID=%d UPID=%u\nRIP=0x%llx CR2=0x%llx RSP=0x%llx RFLAGS=0x%llx\nRAX=0x%llx RBX=0x%llx RCX=0x%llx RDX=0x%llx RSI=0x%llx RDI=0x%llx\n",
                             state->interrupt_number, state->error_code, cur, upid,
                             (unsigned long long)state->rip, (unsigned long long)state->cr2, (unsigned long long)state->rsp, (unsigned long long)state->rflags,
                             (unsigned long long)state->rax, (unsigned long long)state->rbx, (unsigned long long)state->rcx, (unsigned long long)state->rdx,
                             (unsigned long long)state->rsi, (unsigned long long)state->rdi);
            if (r > 0) vfs_write(path, report);
            extern void set_last_crash_path(const char* path);
            set_last_crash_path(path);
            state->rip = (uint64_t)task_crash_cleanup;
            return;
        }
    }

    if (__sync_lock_test_and_set(&in_panic, 1)) {
        serial_write("\n[RECURSIVE_FAULT] Halting.\n");
        while(1) __asm__ volatile("cli; hlt");
    }

    const char *name = "EXCEPTION";
    if (state->interrupt_number == 14) name = "PAGE_FAULT";
    else if (state->interrupt_number == 13) name = "GPF";
    else if (state->interrupt_number == 0) name = "DIVIDE_BY_ZERO";

    serial_printf("\n\n[FATAL] %s (int=%llu err=%llu)\n", name, state->interrupt_number, state->error_code);
    serial_printf("RIP=%p CR2=%p RSP=%p RFLAGS=%p\n", (void*)state->rip, (void*)state->cr2, (void*)state->rsp, (void*)state->rflags);

    scheduler_stop_all();
    draw_osod(name, state);
    while(1) __asm__ volatile("cli; hlt");
}
