#include <stdint.h>
#include <stddef.h>
#include "pro_os.h"

/* Sovereign AHCI SATA Driver */

typedef struct {
    uint32_t clb;
    uint32_t clbu;
    uint32_t fb;
    uint32_t fbu;
    uint32_t is;
    uint32_t ie;
    uint32_t cmd;
    uint32_t reserved0;
    uint32_t tfd;
    uint32_t sig;
    uint32_t ssts;
    uint32_t sctl;
    uint32_t serr;
    uint32_t sact;
    uint32_t ci;
} ahci_port_t;

typedef struct {
    uint32_t cap;
    uint32_t ghc;
    uint32_t is;
    uint32_t pi;
    uint32_t vs;
} ahci_hba_t;

void ahci_init(uint64_t mmio) {
    ahci_hba_t *hba = (ahci_hba_t*)mmio;

    /* Global Host Control Reset */
    hba->ghc |= 0x1;
    while (hba->ghc & 0x1);

    /* Enable AHCI Mode */
    hba->ghc |= 0x80000000;
}
