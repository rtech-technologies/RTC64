/* Copyright (C) 2025 Sovereign RTC64 Project. All rights reserved.
 * Licensed under the 'respect people's property' OS license. */
#include "pro_os.h"
#include "serial.h"
#include <string.h>

#define MAX_AUDIT_LOGS 100
static char audit_logs[MAX_AUDIT_LOGS][128];
static int audit_log_head = 0;
static int audit_log_count = 0;

void comprec_log(const char* tag, const char* event) {
    char entry[128];
    int h, m, s;
    rtc_get_time(&h, &m, &s);
    snprintf(entry, sizeof(entry), "[%02d:%02d:%02d] %s: %s", h, m, s, tag, event);

    strncpy(audit_logs[audit_log_head], entry, sizeof(audit_logs[0])-1);
    audit_log_head = (audit_log_head + 1) % MAX_AUDIT_LOGS;
    if (audit_log_count < MAX_AUDIT_LOGS) audit_log_count++;

    serial_printf("[COMPREC:%s] %s (Tasks: %d, Mem: %d KB)\n",
                 tag, event,
                 scheduler_get_task_count(),
                 (int)(hal_malloc_get_used() / 1024));
}

int comprec_get_logs(char* out, size_t sz) {
    int off = 0;
    for (int i = 0; i < audit_log_count; i++) {
        int idx = (audit_log_head - audit_log_count + i + MAX_AUDIT_LOGS) % MAX_AUDIT_LOGS;
        off += snprintf(out + off, sz - off, "%s\n", audit_logs[idx]);
        if (off >= (int)sz - 1) break;
    }
    return 0;
}

void comprec_task(void* arg) {
    (void)arg;
    comprec_log("SCM", "Component Recording Service Started");
    int last_sec = -1;

    while(1) {
        int h, m, s;
        rtc_get_time(&h, &m, &s);

        if (s != last_sec && (s % 10 == 0)) {
            comprec_log("HEALTH", "Periodic Status Update");
            last_sec = s;
        }

        int mx, my;
        hal_input_get_mouse_abs(&mx, &my);
        int sw, sh;
        hal_get_screen_size(&sw, &sh);

        if (mx < 0 || my < 0 || mx >= sw || my >= sh) {
            comprec_log("POLICY", "Mouse out of bounds! Enforcing relocation.");
            if (mx < 0) mx = 0;
            if (my < 0) my = 0;
            if (mx >= sw) mx = sw - 1;
            if (my >= sh) my = sh - 1;
            hal_input_set_mouse_abs(mx, my);
        }

        scheduler_yield();
    }
}
