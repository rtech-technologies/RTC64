#include "pro_os.h"
#include <string.h>

static task_t tasks[MAX_TASKS];
static int current_task = -1;
static int task_count = 0;

void scheduler_init(void) {
    task_count = 0;
    current_task = -1;
}

void scheduler_add_task(const char *name, void (*entry)(void)) {
    if (task_count < MAX_TASKS) {
        tasks[task_count].id = task_count;
        tasks[task_count].name = name;
        tasks[task_count].state = TASK_RUNNING;
        tasks[task_count].entry = entry;
        task_count++;
    }
}

void scheduler_run(void) {
    if (task_count == 0) return;

    // Simple round robin with state awareness
    for (int i = 0; i < task_count; i++) {
        current_task = (current_task + 1) % task_count;
        if (tasks[current_task].state == TASK_RUNNING && tasks[current_task].entry) {
            tasks[current_task].entry();
            break;
        }
    }
}

void scheduler_set_task_state(int id, task_state_t state) {
    if (id >= 0 && id < task_count) {
        tasks[id].state = state;
    }
}
