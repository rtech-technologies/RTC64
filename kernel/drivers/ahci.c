/* Modified by Sovereign: Meaty AHCI implementation with DMA Read and Write support and Logging
 * Licensed under the 'respect people's property' OS license. */
#include "pro_os.h"
#include <stdint.h>
#include <string.h>
#include "serial.h"

#define AHCI_PORT_CMD      0x18
#define AHCI_PORT_IS       0x10
#define AHCI_PORT_TFD      0x20
#define AHCI_PORT_SSTS     0x28
#define AHCI_PORT_CLB      0x00
#define AHCI_PORT_FB       0x08
#define AHCI_PORT_CI       0x38

#define ATA_CMD_READ_DMA_EXT  0x25
#define ATA_CMD_WRITE_DMA_EXT 0x35

extern uint64_t hhdm_offset;
extern void* pmm_alloc_low(void);

typedef struct {
    uint32_t dba, dbau, rsvd0, flags;
} ahci_prdt_t;

typedef struct {
    uint8_t  cfis[64];
    uint8_t  acmd[16];
    uint8_t  rsvd[48];
    ahci_prdt_t prdt[8];
} ahci_cmd_table_t;

typedef struct {
    uint16_t flags;
    uint16_t prdtl;
    uint32_t prdbc;
    uint32_t ctba;
    uint32_t ctbau;
    uint32_t rsvd[4];
} ahci_cmd_header_t;

static uint64_t ahci_base = 0;

int ahci_init(uint64_t mmio) {
    if (mmio == 0) return -1;
    ahci_base = mmio + hhdm_offset;
    serial_printf("[AHCI] Initializing ABAR at %p\n", ahci_base);
    return 0;
}

static int ahci_io(int port, uint64_t lba, uint16_t count, void* buffer, int write) {
    if (!ahci_base) return -1;
    serial_printf("[AHCI] Port %d I/O: %s LBA=%llu, Count=%u, Buffer=%p\n", port, write ? "WRITE" : "READ", lba, count, buffer);

    volatile uint8_t* pbase = (volatile uint8_t*)(ahci_base + 0x100 + (port * 0x80));

    /* 1. Wait for port to be idle */
    int timeout = 0;
    while ((*(volatile uint32_t*)(pbase + AHCI_PORT_TFD) & ((1 << 3) | (1 << 0))) && timeout++ < 1000000) __asm__("pause");
    if (timeout >= 1000000) return -1;

    /* 2. Setup Command Header */
    ahci_cmd_header_t* cmd_header = (ahci_cmd_header_t*)(uintptr_t)((*(volatile uint32_t*)(pbase + AHCI_PORT_CLB)) + hhdm_offset);
    cmd_header->flags = (5 << 0) | (write ? (1 << 6) : 0);
    cmd_header->prdtl = 1;

    /* 3. Setup Command Table */
    ahci_cmd_table_t* cmd_table = (ahci_cmd_table_t*)(uintptr_t)(cmd_header->ctba + hhdm_offset);
    memset(cmd_table, 0, sizeof(ahci_cmd_table_t));

    /* 4. Setup FIS */
    uint8_t* fis = cmd_table->cfis;
    fis[0] = 0x27; // Register H2D
    fis[1] = (1 << 7); // Command
    fis[2] = write ? ATA_CMD_WRITE_DMA_EXT : ATA_CMD_READ_DMA_EXT;
    fis[4] = (uint8_t)lba;
    fis[5] = (uint8_t)(lba >> 8);
    fis[6] = (uint8_t)(lba >> 16);
    fis[7] = 0x40; // LBA mode
    fis[8] = (uint8_t)(lba >> 24);
    fis[9] = (uint8_t)(lba >> 32);
    fis[10] = (uint8_t)(lba >> 40);
    fis[12] = (uint8_t)count;
    fis[13] = (uint8_t)(count >> 8);

    /* 5. Setup PRDT */
    cmd_table->prdt[0].dba = (uint32_t)(uintptr_t)buffer; // Assume physical address for now or mapped 1:1
    cmd_table->prdt[0].dbau = (uint32_t)((uintptr_t)buffer >> 32);
    cmd_table->prdt[0].flags = (count * 512) - 1;

    /* 6. Issue Command */
    *(volatile uint32_t*)(pbase + AHCI_PORT_CI) = 1;

    /* 7. Wait for completion */
    timeout = 0;
    while ((*(volatile uint32_t*)(pbase + AHCI_PORT_CI) & 1) && timeout++ < 1000000) __asm__("pause");
    if (timeout >= 1000000) { serial_printf("[AHCI] Timeout on command completion\n"); return -1; }

    return 0;
}

int ahci_read(int port, uint64_t lba, uint16_t count, void* buffer) {
    return ahci_io(port, lba, count, buffer, 0);
}

int ahci_write(int port, uint64_t lba, uint16_t count, void* buffer) {
    return ahci_io(port, lba, count, buffer, 1);
}
