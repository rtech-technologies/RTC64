/* Copyright (C) 2025 Sovereign RTC64 Project. All rights reserved.
 * Licensed under the 'respect people's property' OS license. */
#include "pro_os.h"
#include "serial.h"

void comprec_task(void* arg) {
    (void)arg;
    serial_printf("[SCM] Starting Component Recording (COMPREC) service...\n");
    serial_printf("[SCM] System compliance monitoring active.\n");
    while(1) {
        scheduler_yield();
    }
}
