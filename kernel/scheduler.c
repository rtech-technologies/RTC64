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
            scheduler_audit_stacks();

            /* Critical Section Guard for global VFS operations */
            __asm__ volatile("cli");
            vfs_refresh_mounts();
            __asm__ volatile("sti");

            idle_ticks = 0; total_ticks = 0; last_calc = now;
        }
        __asm__ volatile("hlt");
    }
}

void scheduler_init(void) {
    task_count = 0;
    current_task_idx = -1;
    for (int i = 0; i < MAX_TASKS; i++) {
        tasks[i].state = TASK_DEAD;
    }
    scheduler_add_task("Idle Task", kernel_idle_task, NULL, 0, 0);
}

int scheduler_add_task(const char *name, void (*entry)(void*), void *arg, uint32_t uaid, uint32_t upid) {
    int slot = -1;
    for (int i = 0; i < MAX_TASKS; i++) {
        if (tasks[i].state == TASK_DEAD) { slot = i; break; }
    }
    if (slot == -1) return -1;
    if (slot >= task_count) task_count = slot + 1;

    tasks[slot].id = slot; tasks[slot].uaid = uaid; tasks[slot].upid = upid;
    strncpy(tasks[slot].name, name, 31);
    tasks[slot].name[31] = '\0'; /* Mandatory termination safety */
    tasks[slot].state = TASK_RUNNING;
    tasks[slot].entry = entry; tasks[slot].arg = arg;

    uint64_t stack_top = (uint64_t)&task_stacks[slot][STACK_SIZE];
    stack_top &= ~15; /* 16-byte aligned point */

    uint64_t current_cr3, current_cr4;
    __asm__ volatile("mov %%cr3, %0" : "=r"(current_cr3));
    __asm__ volatile("mov %%cr4, %0" : "=r"(current_cr4));

    uint64_t *p = (uint64_t *)stack_top;

    /* 1. iretq frame (SS, RSP, RFLAGS, CS, RIP) */
    *(--p) = 0x10;             /* SS */
    *(--p) = stack_top;        /* Target RSP for the task: clean high point */
    *(--p) = 0x202;            /* RFLAGS (IF=1) */
    *(--p) = 0x08;             /* CS */
    *(--p) = (uint64_t)entry;  /* RIP */

    /* 2. error_code, interrupt_number */
    *(--p) = 0; *(--p) = 0;

    /* 3. GPRs (RAX...R15) */
    *(--p) = 0;             /* RAX */
    *(--p) = 0;             /* RBX */
    *(--p) = 0;             /* RCX */
    *(--p) = 0;             /* RDX */
    *(--p) = 0;             /* RSI */
    *(--p) = (uint64_t)arg;  /* RDI (arg for entry) */
    *(--p) = 0;             /* RBP */
    for(int i=0; i<8; i++) *(--p) = 0; /* R8-R15 */

    /* 4. CRs (CR2, CR3, CR4) */
    *(--p) = 0;           /* CR2 */
    *(--p) = current_cr3; /* CR3 */
    *(--p) = current_cr4; /* CR4 */

    /* 5. Segments (DS, ES, FS, GS) */
    *(--p) = 0x10; /* GS */
    *(--p) = 0x10; /* FS */
    *(--p) = 0x10; /* ES */
    *(--p) = 0x10; /* DS */

    *(--p) = 0; /* Padding for FXSAVE alignment */

    /* 6. FXSAVE Region (512 bytes) */
    for(int i=0; i<64; i++) *(--p) = 0;
    uint32_t *mxcsr = (uint32_t *)((uint8_t *)p + 24);
    *mxcsr = 0x1F80;

    *(uint64_t*)&task_stacks[slot][0] = STACK_CANARY;
    task_rsps[slot] = (uint64_t)p; /* Restoration begins at the start of saved state */
    return slot;
}

int scheduler_spawn(const char* name, void (*entry)(void*), void* arg) {
    return scheduler_add_task(name, entry, arg, next_uaid++, next_upid++);
}

int scheduler_fork(const char* name, void (*entry)(void*), void* arg) {
    return scheduler_add_task(name, entry, arg, scheduler_get_current_uaid(), next_upid++);
}

void scheduler_remove_task(int task_id) {
    if (task_id <= 0 || task_id >= MAX_TASKS) return;
    memset(&tasks[task_id], 0, sizeof(task_t));
    tasks[task_id].state = TASK_DEAD;
}

uint64_t scheduler_switch(uint64_t current_rsp) {
    total_ticks++; ctx_switches++;
    if (task_count == 0) return current_rsp;
    if (current_task_idx != -1 && current_task_idx < MAX_TASKS) task_rsps[current_task_idx] = current_rsp;

    for (int i = 0; i < MAX_TASKS; i++) {
        current_task_idx = (current_task_idx + 1) % MAX_TASKS;
        if (tasks[current_task_idx].state == TASK_RUNNING) {
            return task_rsps[current_task_idx];
        }
    }

    current_task_idx = 0;
    return task_rsps[0];
}

void scheduler_yield(void) { __asm__ volatile("int $0x20"); }
void scheduler_run(void) { __asm__ volatile("sti"); while(1) { __asm__ volatile("hlt"); } }
int scheduler_get_task_count(void) { return task_count; }
task_t* scheduler_get_task(int index) { return (index >= 0 && index < MAX_TASKS) ? &tasks[index] : NULL; }
int scheduler_get_current_task_idx(void) { return current_task_idx; }
uint32_t scheduler_get_current_uaid(void) { return (current_task_idx != -1) ? tasks[current_task_idx].uaid : 0; }
uint32_t scheduler_get_current_upid(void) { return (current_task_idx != -1) ? tasks[current_task_idx].upid : 0; }
int scheduler_get_cpu_load(void) { return cpu_load; }
uint64_t scheduler_get_ctx_switches(void) { return ctx_switches; }
void scheduler_audit_stacks(void) { for (int i = 0; i < MAX_TASKS; i++) { if (tasks[i].state != TASK_DEAD) { if (*(uint64_t*)&task_stacks[i][0] != STACK_CANARY) kpanic("STACK_BUFFER_OVERRUN"); } } }
