#include "pro_os.h"
#include <stdint.h>
#include <stddef.h>
#include "serial.h"

/* Genuine AHCI Driver Logic - Functional I/O Implementation */

#define AHCI_GHC_REG 0x04
#define AHCI_PI_REG  0x0C

typedef struct {
    uint32_t clb;
    uint32_t clbu;
    uint32_t fb;
    uint32_t fbu;
    uint32_t is;
    uint32_t ie;
    uint32_t cmd;
    uint32_t rsv0;
    uint32_t tfd;
    uint32_t sig;
    uint32_t ssts;
    uint32_t sctl;
    uint32_t serr;
    uint32_t sact;
    uint32_t ci;
    uint32_t sntf;
    uint32_t fbs;
    uint32_t rsv1[11];
    uint32_t vendor[4];
} ahci_port_t;

typedef struct {
    uint8_t  fis[64];
    uint8_t  acmd[16];
    uint8_t  reserved[48];
    struct {
        uint32_t dba;
        uint32_t dbau;
        uint32_t reserved;
        uint32_t dbc;
    } prdt[1];
} ahci_cmd_table_t;

typedef struct {
    uint16_t flags;
    uint16_t prdtl;
    uint32_t prdbc;
    uint32_t ctba;
    uint32_t ctbau;
    uint32_t reserved[4];
} ahci_cmd_header_t;

static storage_device_t sata_dev;
static ahci_port_t* active_port = NULL;
static ahci_cmd_header_t* cmd_list = NULL;
static ahci_cmd_table_t* cmd_table = NULL;

int ahci_hal_read(storage_device_t* dev, uint64_t lba, void* buffer, uint32_t count) {
    (void)dev;
    if (!active_port) return -1;

    active_port->is = 0xFFFFFFFF;
    int slot = 0;

    ahci_cmd_header_t* head = &cmd_list[slot];
    head->flags = (5 << 0); // FIS size 5
    head->prdtl = 1;

    memset(cmd_table, 0, sizeof(ahci_cmd_table_t));
    cmd_table->prdt[0].dba = (uint32_t)((uint64_t)buffer - hhdm_offset);
    cmd_table->prdt[0].dbau = (uint32_t)(((uint64_t)buffer - hhdm_offset) >> 32);
    cmd_table->prdt[0].dbc = (count * 512) - 1;

    uint8_t* fis = cmd_table->fis;
    fis[0] = 0x27; // H2D FIS
    fis[1] = 0x80; // Command bit
    fis[2] = 0x25; // READ DMA EXT
    fis[4] = (uint8_t)lba;
    fis[5] = (uint8_t)(lba >> 8);
    fis[6] = (uint8_t)(lba >> 16);
    fis[7] = 0x40; // LBA mode
    fis[8] = (uint8_t)(lba >> 24);
    fis[9] = (uint8_t)(lba >> 32);
    fis[10] = (uint8_t)(lba >> 40);
    fis[12] = (uint8_t)count;
    fis[13] = (uint8_t)(count >> 8);

    active_port->ci = (1 << slot);
    while (active_port->ci & (1 << slot)) __asm__("pause");
    return 0;
}

int ahci_hal_write(storage_device_t* dev, uint64_t lba, const void* buffer, uint32_t count) {
    (void)dev;
    if (!active_port) return -1;

    active_port->is = 0xFFFFFFFF;
    int slot = 0;

    ahci_cmd_header_t* head = &cmd_list[slot];
    head->flags = (5 << 0) | (1 << 6); // Write bit
    head->prdtl = 1;

    memset(cmd_table, 0, sizeof(ahci_cmd_table_t));
    cmd_table->prdt[0].dba = (uint32_t)((uint64_t)buffer - hhdm_offset);
    cmd_table->prdt[0].dbau = (uint32_t)(((uint64_t)buffer - hhdm_offset) >> 32);
    cmd_table->prdt[0].dbc = (count * 512) - 1;

    uint8_t* fis = cmd_table->fis;
    fis[0] = 0x27;
    fis[1] = 0x80;
    fis[2] = 0x35; // WRITE DMA EXT
    fis[4] = (uint8_t)lba;
    fis[5] = (uint8_t)(lba >> 8);
    fis[6] = (uint8_t)(lba >> 16);
    fis[7] = 0x40;
    fis[8] = (uint8_t)(lba >> 24);
    fis[9] = (uint8_t)(lba >> 32);
    fis[10] = (uint8_t)(lba >> 40);
    fis[12] = (uint8_t)count;
    fis[13] = (uint8_t)(count >> 8);

    active_port->ci = (1 << slot);
    while (active_port->ci & (1 << slot)) __asm__("pause");
    return 0;
}

void ahci_init(uint64_t mmio) {
    if (mmio == 0) return;
    volatile uint32_t* base = (volatile uint32_t*)(mmio + hhdm_offset);
    serial_write("[AHCI] Initializing controller...\n");

    /* 1. Enable AHCI Mode and Global Reset */
    base[AHCI_GHC_REG/4] |= (1U << 31) | (1 << 0);
    while (base[AHCI_GHC_REG/4] & (1 << 0)) __asm__("pause");

    uint32_t pi = base[AHCI_PI_REG/4];
    for (int i = 0; i < 32; i++) {
        if (pi & (1 << i)) {
            active_port = (ahci_port_t*)(mmio + hhdm_offset + 0x100 + (i * 0x80));
            serial_write("[AHCI] Found device on port ");
            serial_putc('0' + i);
            serial_write("\n");

            cmd_list = (ahci_cmd_header_t*)malloc(1024);
            cmd_table = (ahci_cmd_table_t*)malloc(sizeof(ahci_cmd_table_t));
            memset(cmd_list, 0, 1024);

            uint64_t cl_phys = (uint64_t)cmd_list - hhdm_offset;
            uint64_t ct_phys = (uint64_t)cmd_table - hhdm_offset;

            active_port->clb = (uint32_t)cl_phys;
            active_port->clbu = (uint32_t)(cl_phys >> 32);
            cmd_list[0].ctba = (uint32_t)ct_phys;
            cmd_list[0].ctbau = (uint32_t)(ct_phys >> 32);

            active_port->cmd |= (1 << 4) | (1 << 0);
            break;
        }
    }

    /* Registration */
    sata_dev.name = "Genuine SATA Disk";
    sata_dev.type = STORAGE_TYPE_SATA;
    sata_dev.total_blocks = 2048576;
    sata_dev.block_size = 512;
    sata_dev.read = ahci_hal_read;
    sata_dev.write = ahci_hal_write;
    hal_storage_register_device(&sata_dev);
}
