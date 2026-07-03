/* Copyright (C) 2025 Sovereign RTC64 Project. All rights reserved.
 * Licensed under the 'respect people's property' OS license. */
#include "pro_os.h"
#include <stdint.h>
#include <string.h>
#include "serial.h"
#include "hal.h"

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
static uint64_t ahci_base = 0;

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

static storage_device_t g_ahci_devices[32];

static int ahci_io_wrapper(storage_device_t* dev, uint64_t lba, void* buffer, uint32_t count, int write) {
    int port = (int)(uintptr_t)dev->priv;
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
    fis[0] = 0x27; fis[1] = (1 << 7);
    fis[2] = write ? ATA_CMD_WRITE_DMA_EXT : ATA_CMD_READ_DMA_EXT;
    fis[4] = (uint8_t)lba; fis[5] = (uint8_t)(lba >> 8); fis[6] = (uint8_t)(lba >> 16);
    fis[7] = 0x40; fis[8] = (uint8_t)(lba >> 24); fis[9] = (uint8_t)(lba >> 32); fis[10] = (uint8_t)(lba >> 40);
    fis[12] = (uint8_t)count; fis[13] = (uint8_t)(count >> 8);

    /* 5. Setup PRDT (Direct mapping assumed for DMA below 4GB) */
    cmd_table->prdt[0].dba = (uint32_t)(uintptr_t)buffer;
    cmd_table->prdt[0].dbau = (uint32_t)((uintptr_t)buffer >> 32);
    cmd_table->prdt[0].flags = (count * 512) - 1;

    /* 6. Issue Command */
    *(volatile uint32_t*)(pbase + AHCI_PORT_CI) = 1;

    /* 7. Wait for completion */
    timeout = 0;
    while ((*(volatile uint32_t*)(pbase + AHCI_PORT_CI) & 1) && timeout++ < 1000000) __asm__("pause");
    return (timeout < 1000000) ? 0 : -1;
}

static int ahci_read(storage_device_t* dev, uint64_t sector, void* buffer, uint32_t count) {
    return ahci_io_wrapper(dev, sector, buffer, count, 0);
}

static int ahci_write(storage_device_t* dev, uint64_t sector, const void* buffer, uint32_t count) {
    return ahci_io_wrapper(dev, sector, (void*)buffer, count, 1);
}

int ahci_init(uint64_t mmio) {
    if (mmio == 0) return -1;
    ahci_base = mmio + hhdm_offset;
    serial_printf("[AHCI] Initializing ABAR at %p\n", (void*)ahci_base);

    /* Discover implemented ports */
    uint32_t pi = *(volatile uint32_t*)(ahci_base + 0x0C);
    for (int i = 0; i < 32; i++) {
        if (pi & (1 << i)) {
            uint32_t ssts = *(volatile uint32_t*)(ahci_base + 0x100 + (i * 0x80) + AHCI_PORT_SSTS);
            if ((ssts & 0x0F) == 0x03) {
                storage_device_t* dev = &g_ahci_devices[i];
                dev->name = "Sovereign SATA Disk";
                dev->type = STORAGE_TYPE_SATA;
                dev->total_blocks = 1000000;
                dev->block_size = 512;
                dev->read = ahci_read;
                dev->write = ahci_write;
                dev->priv = (void*)(uintptr_t)i;
                hal_storage_register_device(dev);
                serial_printf("[AHCI] Registered SATA device on port %d\n", i);
            }
        }
    }
    return 0;
}
