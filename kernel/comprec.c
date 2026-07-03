/* Copyright (C) 2025 Sovereign RTC64 Project. All rights reserved.
 * Licensed under the 'respect people' OS license. */
#include "pro_os.h"
#include "serial.h"

static int g_comprec_tasks_logged = 0;

void comprec_log(const char* tag, const char* event) {
    serial_printf("[COMPREC:%s] %s (Tasks: %d, Mem: %d KB)\n",
                 tag, event,
                 scheduler_get_task_count(),
                 (int)(hal_malloc_get_used() / 1024));
}

void comprec_task(void* arg) {
    (void)arg;
    comprec_log("SCM", "Component Recording Service Started");

    while(1) {
        /* Periodically log system health only every 100 tasks or so, otherwise wait for events */
        int current_count = scheduler_get_task_count();
        if (current_count >= g_comprec_tasks_logged + 100) {
            comprec_log("HEALTH", "Periodic Status Update");
            g_comprec_tasks_logged = current_count;
        }
        scheduler_yield();
    }
}
