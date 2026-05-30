#include "pro_os.h"
#include <stdint.h>

/* Genuine AHCI Driver Logic - Register Mapping & Initialization */

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

void ahci_init(uint64_t mmio) {
    if (mmio == 0) {
        return;
    }
    volatile uint32_t* ghc = (volatile uint32_t*)(mmio + hhdm_offset + AHCI_GHC_REG);

    /* 1. Enable AHCI Mode */
    *ghc |= (1U << 31);

    /* 2. Global Reset */
    *ghc |= (1 << 0);
    while (*ghc & (1 << 0));
}
