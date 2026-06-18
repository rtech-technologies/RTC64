/* Modified by Sovereign: Meaty AHCI implementation with DMA Read and Write support and Logging */
#include "pro_os.h"
#include <stdint.h>
#include <string.h>
#include "serial.h"

#define AHCI_PORT_CMD_LIST_ADDR 0x00
#define AHCI_PORT_FIS_ADDR      0x08
#define AHCI_PORT_IS            0x10
#define AHCI_PORT_IE            0x14
#define AHCI_PORT_CMD           0x18
#define AHCI_PORT_TFD           0x20
#define AHCI_PORT_SIG           0x24
#define AHCI_PORT_SSTS          0x28
#define AHCI_PORT_SCTL          0x2C
#define AHCI_PORT_SERR          0x30
#define AHCI_PORT_SACT          0x34
#define AHCI_PORT_CI            0x38

#define AHCI_CMD_WRITE          (1 << 6)
#define AHCI_CMD_ST             (1 << 0)
#define AHCI_CMD_FRE            (1 << 4)
#define AHCI_CMD_FR             (1 << 14)
#define AHCI_CMD_CR             (1 << 15)

#define ATA_CMD_READ_DMA_EX     0x25
#define ATA_CMD_WRITE_DMA_EX    0x35

typedef struct {
    uint32_t dba;
    uint32_t dbau;
    uint32_t rsvd0;
    uint32_t dw3; // DBC (bits 0-21), I bit (bit 31)
} ahci_prdt_t;

typedef struct {
    uint8_t  cfis[64];
    uint8_t  acmd[16];
    uint8_t  rsvd[48];
    ahci_prdt_t prdt[8]; // Support up to 8 PRD entries
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
static ahci_cmd_header_t* cmd_headers[32];
static ahci_cmd_table_t* cmd_tables[32];

int ahci_init(uint64_t mmio) {
    if (mmio == 0) return -1;
    ahci_base = mmio + hhdm_offset;
    serial_printf("[AHCI] Initializing ABAR at %p (Phys: %p)\n", (void*)ahci_base, (void*)mmio);

    /* Enable AHCI Mode (GHC.AE bit 31) */
    *(volatile uint32_t*)(ahci_base + 0x04) |= (1U << 31);

    /* Initialize ports (simple loop for first 4 ports) */
    for (int i = 0; i < 32; i++) {
        uint32_t pi_mask = *(volatile uint32_t*)(ahci_base + 0x0C);
        if (!(pi_mask & (1 << i))) continue;

        uint64_t port_base = ahci_base + 0x100 + (i * 0x80);
        uint32_t ssts = *(volatile uint32_t*)(port_base + AHCI_PORT_SSTS);
        uint8_t ipm = (ssts >> 8) & 0x0F;
        uint8_t det = ssts & 0x0F;

        if (det != 3 || ipm != 1) continue; // Device not present or not active

        serial_printf("[AHCI] Found SATA device on port %d\n", i);

        /* Stop command engine */
        *(volatile uint32_t*)(port_base + AHCI_PORT_CMD) &= ~AHCI_CMD_ST;
        *(volatile uint32_t*)(port_base + AHCI_PORT_CMD) &= ~AHCI_CMD_FRE;
        while (*(volatile uint32_t*)(port_base + AHCI_PORT_CMD) & (AHCI_CMD_CR | AHCI_CMD_FR)) __asm__("pause");

        /* Allocate memory for port structures */
        void* cl_phys = pmm_alloc_low();
        void* fis_phys = pmm_alloc_low();
        void* ct_phys = pmm_alloc_low();

        *(volatile uint64_t*)(port_base + AHCI_PORT_CMD_LIST_ADDR) = (uint64_t)cl_phys;
        *(volatile uint64_t*)(port_base + AHCI_PORT_FIS_ADDR) = (uint64_t)fis_phys;

        cmd_headers[i] = (ahci_cmd_header_t*)((uint64_t)cl_phys + hhdm_offset);
        cmd_tables[i] = (ahci_cmd_table_t*)((uint64_t)ct_phys + hhdm_offset);

        memset(cmd_headers[i], 0, 1024);
        memset(cmd_tables[i], 0, sizeof(ahci_cmd_table_t));

        /* Setup command header for slot 0 */
        cmd_headers[i][0].ctba = (uint32_t)(uint64_t)ct_phys;
        cmd_headers[i][0].ctbau = (uint32_t)((uint64_t)ct_phys >> 32);
        cmd_headers[i][0].prdtl = 1;

        /* Start command engine */
        while (*(volatile uint32_t*)(port_base + AHCI_PORT_CMD) & AHCI_CMD_CR) __asm__("pause");
        *(volatile uint32_t*)(port_base + AHCI_PORT_CMD) |= AHCI_CMD_FRE;
        *(volatile uint32_t*)(port_base + AHCI_PORT_CMD) |= AHCI_CMD_ST;
    }

    return 0;
}

static int ahci_io(int port, uint64_t lba, uint16_t count, void* buffer, int write) {
    if (!ahci_base || !cmd_headers[port]) return -1;
    uint64_t port_base = ahci_base + 0x100 + (port * 0x80);

    /* Clear pending interrupts */
    *(volatile uint32_t*)(port_base + AHCI_PORT_IS) = 0xFFFFFFFF;

    /* Build CFIS (Register H2D) */
    uint8_t* cfis = cmd_tables[port]->cfis;
    memset(cfis, 0, 64);
    cfis[0] = 0x27; // Register H2D
    cfis[1] = (1 << 7); // Command
    cfis[2] = write ? ATA_CMD_WRITE_DMA_EX : ATA_CMD_READ_DMA_EX;
    cfis[4] = (uint8_t)lba;
    cfis[5] = (uint8_t)(lba >> 8);
    cfis[6] = (uint8_t)(lba >> 16);
    cfis[7] = 0x40; // LBA mode
    cfis[8] = (uint8_t)(lba >> 24);
    cfis[9] = (uint8_t)(lba >> 32);
    cfis[10] = (uint8_t)(lba >> 40);
    cfis[12] = (uint8_t)count;
    cfis[13] = (uint8_t)(count >> 8);

    /* Build PRDT (Physical Region Descriptor Table) */
    uint64_t phys_buffer = (uint64_t)buffer - hhdm_offset;
    cmd_tables[port]->prdt[0].dba = (uint32_t)phys_buffer;
    cmd_tables[port]->prdt[0].dbau = (uint32_t)(phys_buffer >> 32);
    cmd_tables[port]->prdt[0].dw3 = ((count * 512) - 1) | (1U << 31); // Interrupt on completion

    /* Setup Command Header */
    cmd_headers[port][0].flags = (5 << 0) | (write ? AHCI_CMD_WRITE : 0); // FIS length 5 dwords
    cmd_headers[port][0].prdtl = 1;
    cmd_headers[port][0].prdbc = 0;

    /* Issue command */
    *(volatile uint32_t*)(port_base + AHCI_PORT_CI) = 1;

    /* Wait for completion */
    int timeout = 0;
    while (timeout++ < 1000000) {
        if (!(*(volatile uint32_t*)(port_base + AHCI_PORT_CI) & 1)) break;
        if (*(volatile uint32_t*)(port_base + AHCI_PORT_TFD) & (1 << 0)) { // Error bit
            serial_printf("[AHCI] Port %d Error during %s! TFD=%08x\n", port, write ? "WRITE" : "READ", *(volatile uint32_t*)(port_base + AHCI_PORT_TFD));
            return -1;
        }
        __asm__("pause");
    }

    if (timeout >= 1000000) {
        serial_printf("[AHCI] Port %d Timeout during %s!\n", port, write ? "WRITE" : "READ");
        return -1;
    }

    return 0;
}

int ahci_read(int port, uint64_t lba, uint16_t count, void* buffer) {
    return ahci_io(port, lba, count, buffer, 0);
}

int ahci_write(int port, uint64_t lba, uint16_t count, void* buffer) {
    return ahci_io(port, lba, count, buffer, 1);
}
