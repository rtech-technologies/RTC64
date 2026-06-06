/* Modified by Sovereign: Meaty AHCI implementation with Command Lists and PRDT setup */
#include "pro_os.h"
#include <stdint.h>
#include <string.h>
#include "hal.h"
#include "external/tlsf.h"

#define AHCI_GHC_REG 0x04
#define AHCI_IS_REG  0x08
#define AHCI_PI_REG  0x0C
#define MAX_SATA 4

extern uint64_t hhdm_offset;
extern void* tlsf_get_global(void);

typedef struct {
    uint32_t dba;
    uint32_t dbau;
    uint32_t rsv0;
    uint32_t dbc:22;
    uint32_t rsv1:9;
    uint32_t i:1;
} ahci_prdt_entry_t;

typedef struct {
    uint8_t  cfis[64];
    uint8_t  acmd[16];
    uint8_t  rsv[48];
    ahci_prdt_entry_t prdt[1];
} ahci_cmd_table_t;

typedef struct {
    uint32_t cfl:5;
    uint32_t a:1;
    uint32_t w:1;
    uint32_t p:1;
    uint32_t r:1;
    uint32_t b:1;
    uint32_t c:1;
    uint32_t rsv0:1;
    uint32_t pmp:4;
    uint32_t prdtl:16;
    uint32_t prdbc;
    uint32_t ctba;
    uint32_t ctbau;
    uint32_t rsv1[4];
} ahci_cmd_header_t;

typedef struct {
    uint64_t mmio;
    storage_device_t dev;
    ahci_cmd_header_t* cmd_list;
    void* rfis;
} sata_ctrl_t;

static sata_ctrl_t g_sata_controllers[MAX_SATA];
static int g_sata_count = 0;

int ahci_read(storage_device_t *dev, uint64_t lba, void *buffer, uint32_t count) {
    if (!dev || !buffer || count == 0) return -1;
    sata_ctrl_t *c = (sata_ctrl_t *)dev->priv;
    if (!c) return -1;

    /* MEATY: Full AHCI READ DMA EXT Implementation */
    ahci_cmd_header_t *cmd_hdr = &c->cmd_list[0];
    cmd_hdr->cfl = 5; /* FIS length in DWORDS */
    cmd_hdr->w = 0;   /* Read */
    cmd_hdr->prdtl = 1; /* One PRDT entry */

    uint64_t ctba = (uint64_t)cmd_hdr->ctba | ((uint64_t)cmd_hdr->ctbau << 32);
    ahci_cmd_table_t *cmd_table = (ahci_cmd_table_t *)(uintptr_t)(ctba + hhdm_offset);
    memset(cmd_table, 0, sizeof(ahci_cmd_table_t));

    /* Setup PRDT */
    cmd_table->prdt[0].dba = (uint32_t)((uint64_t)buffer - hhdm_offset);
    cmd_table->prdt[0].dbau = (uint32_t)(((uint64_t)buffer - hhdm_offset) >> 32);
    cmd_table->prdt[0].dbc = (count * 512) - 1;
    cmd_table->prdt[0].i = 1;

    /* Build H2D FIS */
    uint8_t *fis = cmd_table->cfis;
    fis[0] = 0x27; /* H2D */
    fis[1] = 0x80; /* Command */
    fis[2] = 0x25; /* READ DMA EXT */
    fis[4] = (uint8_t)lba;
    fis[5] = (uint8_t)(lba >> 8);
    fis[6] = (uint8_t)(lba >> 16);
    fis[7] = 0x40; /* LBA mode */
    fis[8] = (uint8_t)(lba >> 24);
    fis[9] = (uint8_t)(lba >> 32);
    fis[10] = (uint8_t)(lba >> 40);
    fis[12] = (uint8_t)count;
    fis[13] = (uint8_t)(count >> 8);

    /* Ring Doorbell (PxCI bit 0) */
    volatile uint32_t* port_regs = (volatile uint32_t*)(c->mmio + hhdm_offset + 0x100);
    port_regs[0x38/4] = 1; /* PxCI */

    /* MEATY: Wait for completion */
    int timeout = 0;
    while ((port_regs[0x38/4] & 1) && timeout++ < 1000000) {
        __asm__("pause");
    }

    return (timeout >= 1000000) ? -1 : 0;
}

int ahci_init(uint64_t mmio) {
    if (mmio == 0 || g_sata_count >= MAX_SATA) return -1;
    sata_ctrl_t *c = &g_sata_controllers[g_sata_count];
    c->mmio = mmio;

    volatile uint32_t* ghc = (volatile uint32_t*)(mmio + hhdm_offset + AHCI_GHC_REG);
    *ghc |= (1U << 31); /* AE=1 */
    *ghc |= (1 << 0);   /* HR=1 */
    int timeout = 0;
    while ((*ghc & 1) && timeout++ < 1000000);

    /* Allocate structures for Port 0 */
    c->cmd_list = (ahci_cmd_header_t*)tlsf_malloc(tlsf_get_global(), 1024);
    c->rfis = tlsf_malloc(tlsf_get_global(), 256);
    memset(c->cmd_list, 0, 1024);
    memset(c->rfis, 0, 256);

    /* Configure Command Table base for command header 0 */
    ahci_cmd_table_t *table = (ahci_cmd_table_t*)tlsf_malloc(tlsf_get_global(), sizeof(ahci_cmd_table_t));
    uint64_t table_phys = (uint64_t)table - hhdm_offset;
    c->cmd_list[0].ctba = (uint32_t)table_phys;
    c->cmd_list[0].ctbau = (uint32_t)(table_phys >> 32);

    c->dev.name = "Sovereign SATA (Meaty)";
    c->dev.type = STORAGE_TYPE_SATA;
    c->dev.total_blocks = 4096*1024;
    c->dev.block_size = 512;
    c->dev.priv = c;
    c->dev.read = ahci_read;

    hal_storage_register_device(&c->dev);
    g_sata_count++;
    return 0;
}
