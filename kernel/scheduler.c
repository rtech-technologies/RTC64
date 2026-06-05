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
    current_task = (current_task + 1) % task_count;
    if (tasks[current_task].entry) {
        tasks[current_task].entry();
    }
}

int scheduler_get_task_count(void) {
    return task_count;
}

task_t* scheduler_get_task(int index) {
    if (index >= 0 && index < task_count) return &tasks[index];
    return NULL;
}
