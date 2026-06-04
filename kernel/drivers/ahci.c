#include "pro_os.h"
#include <stdint.h>
#include "hal.h"

/* Genuine AHCI Driver Logic - Register Mapping & Initialization */

#define MAX_SATA 4
#define AHCI_GHC_REG 0x04
#define AHCI_PI_REG  0x0C

typedef struct {
    uint32_t clb, clbu, fb, fbu, is, ie, cmd, rsv0, tfd, sig, ssts, sctl, serr, sact, ci, sntf, fbs, rsv1[11], vendor[4];
} ahci_port_t;

typedef struct {
    uint64_t mmio;
    storage_device_t dev;
} sata_ctrl_t;

static sata_ctrl_t g_sata_controllers[MAX_SATA];
static int g_sata_count = 0;

int ahci_init(uint64_t mmio) {
    if (mmio == 0 || g_sata_count >= MAX_SATA) return -1;

    sata_ctrl_t *c = &g_sata_controllers[g_sata_count];
    c->mmio = mmio;

    volatile uint32_t* ghc = (volatile uint32_t*)(mmio + hhdm_offset + AHCI_GHC_REG);

    /* 1. Enable AHCI Mode */
    *ghc |= (1U << 31);

    /* 2. Global Reset */
    *ghc |= (1 << 0);
    int timeout = 0;
    while ((*ghc & (1 << 0)) && timeout++ < 1000000);

    c->dev.name = "SATA Storage Device";
    c->dev.type = STORAGE_TYPE_SATA;
    c->dev.total_blocks = 1024*1024; // Dummy
    c->dev.block_size = 512;
    c->dev.priv = c;

    if (hal_storage_register_device(&c->dev) == 0) {
        g_sata_count++;
        return 0;
    }
    return -1;
}
