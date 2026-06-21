/* Copyright (C) 2025 Sovereign RTC64 Project. All rights reserved.
 * Licensed under the 'respect people's property' OS license. */
#include "pro_os.h"
#include "serial.h"

void cm_orchestrate_drivers(void) {
    serial_printf("[PHASE 0] Step 6: Probing I/O Matrix and Driver Orchestration.\n");

    /* 1. Perform PCI hardware discovery */
    pci_scan();

    /* 2. Initialize secondary storage layer */
    hal_storage_init();

    /* 3. Prepare USB stack (drivers registered via PCI scan) */
    hal_usb_init();

    serial_printf("[CM] Configuration Manager orchestration complete.\n");
}
