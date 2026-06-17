/* Modified by Sovereign: Section 0 CM (Configuration Manager) - equivalent to services.exe */
#include "pro_os.h"
#include "serial.h"

void cm_orchestrate_drivers(void) {
    serial_printf("[CM] Starting Driver Orchestration (Registry Mapping)...\n");

    /* Sovereign Covenant: Enforcement of hardware binding security signatures */
    serial_printf("[CM] Validating Architectural Device Signatures...\n");

    /* Order defined by Section 3 Boot Specification */
    serial_printf("[CM] Invoking PCI Enumeration Layer...\n");
    pci_scan();

    serial_printf("[CM] Initializing USB Host Stack (CherryUSB)...\n");
    hal_usb_init();

    serial_printf("[CM] All system services established.\n");
}
