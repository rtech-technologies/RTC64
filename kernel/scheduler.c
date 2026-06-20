/* Copyright (C) 2025 Sovereign RTC64 Project. All rights reserved.
 * Licensed under the 'respect people's property' OS license. */
#include "pro_os.h"
#include <string.h>
#include "serial.h"

#define STACK_SIZE 16384
#define STACK_CANARY 0xDEADC0DEBEEFC0DEULL

static task_t tasks[MAX_TASKS];
static uint8_t task_stacks[MAX_TASKS][STACK_SIZE] __attribute__((aligned(16)));
static int current_task_idx = -1;
static int task_count = 0;
static uint64_t task_rsps[MAX_TASKS];
static uint64_t idle_ticks = 0, total_ticks = 0, ctx_switches = 0;
static int cpu_load = 0;
static uint32_t next_uaid = 100, next_upid = 1000;

static void kernel_idle_task(void* arg) {
    (void)arg; uint64_t last_calc = 0;
    while (1) {
        idle_ticks++; uint64_t now = hal_get_uptime_ms();
        if (now - last_calc >= 1000) {
            uint64_t work_ticks = total_ticks - idle_ticks;
            if (total_ticks > 0) cpu_load = (int)((work_ticks * 100) / total_ticks);
            scheduler_audit_stacks(); vfs_refresh_mounts();
            idle_ticks = 0; total_ticks = 0; last_calc = now;
        }
        __asm__("hlt");
    }
}

void scheduler_init(void) {
    task_count = 0; current_task_idx = -1;
    scheduler_add_task("Idle Task", kernel_idle_task, NULL, 0, 0);
}

int scheduler_add_task(const char *name, void (*entry)(void*), void *arg, uint32_t uaid, uint32_t upid) {
    int slot = -1;
    for (int i = 0; i < MAX_TASKS; i++) {
        if (i < task_count && tasks[i].state == TASK_DEAD) { slot = i; break; }
    }
    if (slot == -1 && task_count < MAX_TASKS) slot = task_count++;

    if (slot != -1) {
        tasks[slot].id = slot; tasks[slot].uaid = uaid; tasks[slot].upid = upid;
        strncpy(tasks[slot].name, name, 31); tasks[slot].state = TASK_RUNNING;
        tasks[slot].entry = entry; tasks[slot].arg = arg;

        uint64_t stack_top = (uint64_t)&task_stacks[slot][STACK_SIZE];
        stack_top &= ~15;
        uint64_t *stack = (uint64_t *)stack_top;

        uint64_t current_cr3, current_cr4;
        __asm__ volatile("mov %%cr3, %0" : "=r"(current_cr3));
        __asm__ volatile("mov %%cr4, %0" : "=r"(current_cr4));

        /* Order: iretq frame (SS, RSP, RFLAGS, CS, RIP) */
        *(--stack) = 0x10;             /* SS */
        *(--stack) = stack_top - 8;    /* RSP */
        *(--stack) = 0x202;            /* RFLAGS (IF=1) */
        *(--stack) = 0x08;             /* CS */
        *(--stack) = (uint64_t)entry;  /* RIP */

        /* error_code, interrupt_number */
        *(--stack) = 0; *(--stack) = 0;

        /* GPRs (RAX...R15)
         * Assembly pops in order: R15, R14, R13, R12, R11, R10, R9, R8, RBP, RDI, RSI, RDX, RCX, RBX, RAX
         * So we push in reverse: RAX, RBX, RCX, RDX, RSI, RDI, RBP, R8, R9, R10, R11, R12, R13, R14, R15
         */
        *(--stack) = 0;             /* RAX */
        *(--stack) = 0;             /* RBX */
        *(--stack) = 0;             /* RCX */
        *(--stack) = 0;             /* RDX */
        *(--stack) = 0;             /* RSI */
        *(--stack) = (uint64_t)arg;  /* RDI (arg for C entry) */
        *(--stack) = 0;             /* RBP */
        *(--stack) = 0;             /* R8 */
        *(--stack) = 0;             /* R9 */
        *(--stack) = 0;             /* R10 */
        *(--stack) = 0;             /* R11 */
        *(--stack) = 0;             /* R12 */
        *(--stack) = 0;             /* R13 */
        *(--stack) = 0;             /* R14 */
        *(--stack) = 0;             /* R15 */

        /* CRs (CR2, CR3, CR4) - Order: CR2, CR3, CR4 */
        *(--stack) = 0;           /* CR2 */
        *(--stack) = current_cr3; /* CR3 */
        *(--stack) = current_cr4; /* CR4 */

        /* Segments (DS, ES, FS, GS) - Order: DS, ES, FS, GS */
        *(--stack) = 0x10; /* DS */
        *(--stack) = 0x10; /* ES */
        *(--stack) = 0x10; /* FS */
        *(--stack) = 0x10; /* GS */

        /* Padding for FXSAVE alignment */
        *(--stack) = 0;

        /* FXSAVE Region (512 bytes) */
        for(int i=0; i<64; i++) *(--stack) = 0;
        uint32_t *mxcsr = (uint32_t *)((uint8_t *)stack + 24);
        *mxcsr = 0x1F80;

        *(uint64_t*)&task_stacks[slot][0] = STACK_CANARY;
        task_rsps[slot] = (uint64_t)stack;
        return slot;
    }
    return -1;
}

int scheduler_spawn(const char* name, void (*entry)(void*), void* arg) { return scheduler_add_task(name, entry, arg, next_uaid++, next_upid++); }
int scheduler_fork(const char* name, void (*entry)(void*), void* arg) { return scheduler_add_task(name, entry, arg, scheduler_get_current_uaid(), next_upid++); }
void scheduler_remove_task(int task_id) { if (task_id <= 0 || task_id >= MAX_TASKS) return; memset(&tasks[task_id], 0, sizeof(task_t)); tasks[task_id].state = TASK_DEAD; }

uint64_t scheduler_switch(uint64_t current_rsp) {
    total_ticks++; ctx_switches++;
    if (task_count == 0) return current_rsp;
    if (current_task_idx != -1 && current_task_idx < MAX_TASKS) task_rsps[current_task_idx] = current_rsp;

    for (int i = 0; i < MAX_TASKS; i++) {
        current_task_idx = (current_task_idx + 1) % MAX_TASKS;
        if (current_task_idx < task_count && tasks[current_task_idx].state == TASK_RUNNING) {
            return task_rsps[current_task_idx];
        }
    }

    current_task_idx = 0;
    return task_rsps[0];
}

void scheduler_yield(void) { __asm__ volatile("int $0x20"); }
void scheduler_run(void) { __asm__ volatile("sti"); while(1) { __asm__("hlt"); } }
int scheduler_get_task_count(void) { return task_count; }
task_t* scheduler_get_task(int index) { return (index >= 0 && index < MAX_TASKS) ? &tasks[index] : NULL; }
int scheduler_get_current_task_idx(void) { return current_task_idx; }
uint32_t scheduler_get_current_uaid(void) { return (current_task_idx != -1) ? tasks[current_task_idx].uaid : 0; }
uint32_t scheduler_get_current_upid(void) { return (current_task_idx != -1) ? tasks[current_task_idx].upid : 0; }
int scheduler_get_cpu_load(void) { return cpu_load; }
uint64_t scheduler_get_ctx_switches(void) { return ctx_switches; }
void scheduler_audit_stacks(void) { for (int i = 0; i < task_count; i++) { if (tasks[i].state != TASK_DEAD) { if (*(uint64_t*)&task_stacks[i][0] != STACK_CANARY) kpanic("STACK_BUFFER_OVERRUN"); } } }
