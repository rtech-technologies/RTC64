/* Copyright (C) 2025 Sovereign RTC64 Project. All rights reserved.
 * Licensed under the 'respect people's property' OS license. */
#include "pro_os.h"
#include "serial.h"

void comprec_task(void* arg) {
    (void)arg;
    serial_printf("[SCM] Starting Component Recording (COMPREC) service...\n");

    uint64_t last_report = 0;
    while(1) {
        uint64_t now = hal_get_uptime_ms();
        if (now - last_report >= 5000) {
            serial_printf("[COMPREC] System Health: CPU Load %d%%, Context Switches %llu, Memory Used %d KB\n",
                         scheduler_get_cpu_load(),
                         scheduler_get_ctx_switches(),
                         (int)(hal_malloc_get_used() / 1024));
            last_report = now;
        }
        scheduler_yield();
    }
}
