#include "pro_os.h"
#include "serial.h"
#include <string.h>

struct rsdp_descriptor {
    char signature[8];
    uint8_t checksum;
    char oem_id[6];
    uint8_t revision;
    uint32_t rsdt_address;
} __attribute__((packed));

struct rsdt_header {
    char signature[4];
    uint32_t length;
    uint8_t revision;
    uint8_t checksum;
    char oem_id[6];
    char oem_table_id[8];
    uint32_t oem_revision;
    uint32_t creator_id;
    uint32_t creator_revision;
} __attribute__((packed));

void acpi_init(void* rsdp_addr) {
    serial_write("[ACPI] Initializing from RSDP...\n");
    if (!rsdp_addr) {
        serial_write("[ACPI] Error: NULL RSDP pointer\n");
        return;
    }

    struct rsdp_descriptor* rsdp = (struct rsdp_descriptor*)rsdp_addr;
    if (strncmp(rsdp->signature, "RSD PTR ", 8) != 0) {
        serial_write("[ACPI] Error: Invalid RSDP signature\n");
        return;
    }

    serial_printf("[ACPI] Revision: %d\n", rsdp->revision);

    // Simple RSDT parsing
    struct rsdt_header* rsdt = (struct rsdt_header*)((uint64_t)rsdp->rsdt_address + hhdm_offset);
    if (strncmp(rsdt->signature, "RSDT", 4) == 0) {
        serial_printf("[ACPI] Found RSDT at %x\n", rsdp->rsdt_address);
    }
}
