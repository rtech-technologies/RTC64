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
        /* Periodically log system health */
        int current_count = scheduler_get_task_count();
        if (current_count >= g_comprec_tasks_logged + 100) {
            comprec_log("HEALTH", "Periodic Status Update");
            g_comprec_tasks_logged = current_count;
        }

        /* Sovereign Directive: Monitor and enforce mouse position policy */
        int mx, my;
        hal_input_get_mouse_abs(&mx, &my);

        /* Log mouse position telemetry */
        char mouse_telemetry[64];
        snprintf(mouse_telemetry, sizeof(mouse_telemetry), "Mouse at: %d, %d", mx, my);
        comprec_log("INPUT", mouse_telemetry);

        /* Sovereign Directive: Enforce mouse on-screen policy */
        int sw, sh;
        hal_get_screen_size(&sw, &sh);

        bool out = false;
        int target_x = mx, target_y = my;

        if (mx < 0) { target_x = 0; out = true; }
        if (my < 0) { target_y = 0; out = true; }
        if (mx >= sw) { target_x = sw - 1; out = true; }
        if (my >= sh) { target_y = sh - 1; out = true; }

        if (out) {
            comprec_log("POLICY", "Mouse out of bounds! Relocating to screen.");
            hal_input_set_mouse_abs(target_x, target_y);
        }

        scheduler_yield();
    }
}
