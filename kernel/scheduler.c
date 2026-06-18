/* Modified by Sovereign: Meaty Preemptive Scheduler with Context Switching and SSE State support */
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

static uint64_t idle_ticks = 0;
static uint64_t total_ticks = 0;
static uint64_t ctx_switches = 0;
static int cpu_load = 0;

/* Modified by Sovereign: Dynamic ID generation counters */
static uint32_t next_uaid = 100;
static uint32_t next_upid = 1000;

static void kernel_idle_task(void* arg) {
    (void)arg;
    uint64_t last_calc = 0;
    while (1) {
        idle_ticks++;

        uint64_t now = hal_get_uptime_ms();
        if (now - last_calc >= 1000) {
            static uint64_t audit_timer = 0;
            audit_timer++;

            uint64_t work_ticks = total_ticks - idle_ticks;
            if (total_ticks > 0) {
                cpu_load = (int)((work_ticks * 100) / total_ticks);
            }

            scheduler_audit_stacks();

            size_t used = hal_malloc_get_used();
            size_t total = hal_malloc_get_total();
            if (used > (total * 90) / 100) {
                serial_printf("[MONITOR] WARNING: Critical Memory Pressure Detected!\n");
            }

            vfs_refresh_mounts();

            if (audit_timer >= 10) {
                serial_printf("[AUDIT] System Integrity: PASS | Ctx:%llu | CPU:%d%%\n",
                               ctx_switches, cpu_load);
                audit_timer = 0;
            }

            idle_ticks = 0;
            total_ticks = 0;
            last_calc = now;
        }

        __asm__("hlt");
    }
}

void scheduler_init(void) {
    task_count = 0;
    current_task_idx = -1;
    /* Add kernel idle task as the absolute fallback (System IDs 0,0) */
    scheduler_add_task("Idle Task", kernel_idle_task, NULL, 0, 0);
}

int scheduler_add_task(const char *name, void (*entry)(void*), void *arg, uint32_t uaid, uint32_t upid) {
    int slot = -1;
    for (int i = 0; i < MAX_TASKS; i++) {
        if (i < task_count && tasks[i].state == TASK_DEAD) {
            slot = i;
            break;
        }
    }
    if (slot == -1 && task_count < MAX_TASKS) {
        slot = task_count++;
    }

    if (slot != -1) {
        tasks[slot].id = slot;
        tasks[slot].uaid = uaid;
        tasks[slot].upid = upid;
        strncpy(tasks[slot].name, name, 31);
        tasks[slot].state = TASK_RUNNING;
        tasks[slot].entry = entry;
        tasks[slot].arg = arg;

        /* POWER: Setup initial context on the stack */
        uint64_t stack_top = (uint64_t)&task_stacks[slot][STACK_SIZE];
        stack_top &= ~15; /* Ensure 16-byte alignment */
        uint64_t *stack = (uint64_t *)stack_top;

        uint64_t current_cr3, current_cr4;
        __asm__ volatile("mov %%cr3, %0" : "=r"(current_cr3));
        __asm__ volatile("mov %%cr4, %0" : "=r"(current_cr4));

        /* iretq frame */
        *(--stack) = 0x10; /* SS */
        *(--stack) = stack_top - 8; /* RSP */
        *(--stack) = 0x202; /* RFLAGS (Interrupts enabled) */
        *(--stack) = 0x08; /* CS */
        *(--stack) = (uint64_t)entry; /* RIP */

        /* interrupt_number and error_code */
        *(--stack) = 0; /* error_code */
        *(--stack) = 0; /* interrupt_number */

        /* GPRs */
        for(int i=0; i<15; i++) {
            if (i == 5) *(--stack) = (uint64_t)arg; /* rdi */
            else *(--stack) = 0;
        }

        /* CRs */
        *(--stack) = 0; /* cr2 */
        *(--stack) = current_cr3;
        *(--stack) = current_cr4;

        /* Segments */
        *(--stack) = 0x10; /* gs */
        *(--stack) = 0x10; /* fs */
        *(--stack) = 0x10; /* es */
        *(--stack) = 0x10; /* ds */

        *(--stack) = 0; /* Padding */

        /* FXSAVE region */
        for(int i=0; i<64; i++) *(--stack) = 0;
        uint32_t *mxcsr = (uint32_t *)((uint8_t *)stack + 24);
        *mxcsr = 0x1F80;

        uint64_t* canary_ptr = (uint64_t*)&task_stacks[slot][0];
        *canary_ptr = STACK_CANARY;

        task_rsps[slot] = (uint64_t)stack;
        return slot;
    }
    return -1;
}

/* Modified by Sovereign: Professional SPAWN (New App) vs FORK (New Process) logic */
int scheduler_spawn(const char* name, void (*entry)(void*), void* arg) {
    uint32_t uaid = next_uaid++;
    uint32_t upid = next_upid++;
    serial_printf("[SCHEDULER] Spawning New App: %s (UAID:%d, UPID:%d)\n", name, uaid, upid);
    return scheduler_add_task(name, entry, arg, uaid, upid);
}

int scheduler_fork(const char* name, void (*entry)(void*), void* arg) {
    uint32_t uaid = scheduler_get_current_uaid();
    uint32_t upid = next_upid++;
    serial_printf("[SCHEDULER] Forking Process: %s (UAID:%d, UPID:%d)\n", name, uaid, upid);
    return scheduler_add_task(name, entry, arg, uaid, upid);
}

typedef struct {
    uint32_t handle_id;
    void* object_ptr;
} iht_entry_t;

static iht_entry_t task_iht[MAX_TASKS][32];

static void scheduler_journaled_finalize(int task_id) {
    serial_printf("[COMPREC] Finalizing task %d (%s)...\n", task_id, tasks[task_id].name);

    for (int i = 0; i < 32; i++) {
        if (task_iht[task_id][i].object_ptr) {
            serial_printf("[COMPREC] Reclaiming IHT handle %d\n", task_iht[task_id][i].handle_id);
            task_iht[task_id][i].object_ptr = NULL;
        }
    }

    memset(&tasks[task_id], 0, sizeof(task_t));
    tasks[task_id].state = TASK_DEAD;

    serial_printf("[COMPREC] Task %d successfully journaled and finalized.\n", task_id);
}

void scheduler_remove_task(int task_id) {
    if (task_id <= 0 || task_id >= MAX_TASKS) return;
    if (task_id < task_count && tasks[task_id].state != TASK_DEAD) {
        scheduler_journaled_finalize(task_id);
    }
}

uint64_t scheduler_switch(uint64_t current_rsp) {
    total_ticks++;
    ctx_switches++;
    if (task_count == 0) return current_rsp;

    if (current_task_idx != -1 && current_task_idx < task_count) {
        task_rsps[current_task_idx] = current_rsp;
    }

    for (int i = 0; i < task_count; i++) {
        current_task_idx = (current_task_idx + 1) % task_count;
        if (tasks[current_task_idx].state != TASK_DEAD) {
            return task_rsps[current_task_idx];
        }
    }

    current_task_idx = 0;
    return task_rsps[0];
}

void scheduler_yield(void) {
    __asm__ volatile("int $32");
}

void scheduler_run(void) {
    __asm__ volatile("sti");
    while(1) { __asm__("hlt"); }
}

int scheduler_get_task_count(void) { return task_count; }
task_t* scheduler_get_task(int index) {
    if (index >= 0 && index < task_count) return &tasks[index];
    return NULL;
}
int scheduler_get_current_task_idx(void) { return current_task_idx; }

uint32_t scheduler_get_current_uaid(void) {
    if (current_task_idx != -1 && current_task_idx < task_count) return tasks[current_task_idx].uaid;
    return 0;
}

uint32_t scheduler_get_current_upid(void) {
    if (current_task_idx != -1 && current_task_idx < task_count) return tasks[current_task_idx].upid;
    return 0;
}

int scheduler_get_cpu_load(void) {
    return cpu_load;
}

uint64_t scheduler_get_ctx_switches(void) {
    return ctx_switches;
}

void scheduler_audit_stacks(void) {
    for (int i = 0; i < task_count; i++) {
        if (tasks[i].state != TASK_DEAD) {
            uint64_t* canary_ptr = (uint64_t*)&task_stacks[i][0];
            if (*canary_ptr != STACK_CANARY) {
                serial_printf("[SECURITY] STACK OVERFLOW DETECTED in task %d (%s)!\n", i, tasks[i].name);
                kpanic("STACK_BUFFER_OVERRUN");
            }
        }
    }
}
