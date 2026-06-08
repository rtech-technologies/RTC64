/* Modified by Sovereign: Meaty Preemptive Scheduler with Context Switching and SSE State support */
#include "pro_os.h"
#include <string.h>

#define MAX_TASKS 5
#define STACK_SIZE 16384

static task_t tasks[MAX_TASKS];
static uint8_t task_stacks[MAX_TASKS][STACK_SIZE] __attribute__((aligned(16)));
static int current_task_idx = -1;
static int task_count = 0;
static uint64_t task_rsps[MAX_TASKS];

void scheduler_init(void) {
    task_count = 0;
    current_task_idx = -1;
}

void scheduler_add_task(const char *name, void (*entry)(void)) {
    if (task_count < MAX_TASKS) {
        tasks[task_count].id = task_count;
        tasks[task_count].name = name;
        tasks[task_count].state = TASK_RUNNING;
        tasks[task_count].entry = entry;

        /* POWER: Setup initial context on the stack */
        uint64_t stack_top = (uint64_t)&task_stacks[task_count][STACK_SIZE];
        stack_top &= ~15; /* Ensure 16-byte alignment for fxsave */
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

        /* GPRs: rax, rbx, rcx, rdx, rsi, rdi, rbp, r8, r9, r10, r11, r12, r13, r14, r15 (15 regs) */
        for(int i=0; i<15; i++) *(--stack) = 0;

        /* CRs: cr2, cr3, cr4 */
        *(--stack) = 0; /* cr2 */
        *(--stack) = current_cr3;
        *(--stack) = current_cr4;

        /* Segments: gs, fs, es, ds */
        *(--stack) = 0x10; /* gs */
        *(--stack) = 0x10; /* fs */
        *(--stack) = 0x10; /* es */
        *(--stack) = 0x10; /* ds */

        /* FXSAVE region (512 bytes = 64 uint64_t) */
        for(int i=0; i<64; i++) *(--stack) = 0;
        /* Initialize MXCSR to default if needed, or just zero it */
        uint32_t *mxcsr = (uint32_t *)((uint8_t *)stack + 24);
        *mxcsr = 0x1F80; /* Default MXCSR value */

        task_rsps[task_count] = (uint64_t)stack;
        task_count++;
    }
}

/* POWER: The meaty preemptive context switch */
uint64_t scheduler_switch(uint64_t current_rsp) {
    if (task_count == 0) return current_rsp;

    if (current_task_idx != -1) {
        task_rsps[current_task_idx] = current_rsp;
    }

    current_task_idx = (current_task_idx + 1) % task_count;
    return task_rsps[current_task_idx];
}

void scheduler_run(void) {
    /* Preemptive scheduler is driven by timer interrupt */
    while(1) { __asm__("hlt"); }
}

int scheduler_get_task_count(void) { return task_count; }
task_t* scheduler_get_task(int index) {
    if (index >= 0 && index < task_count) return &tasks[index];
    return NULL;
}
