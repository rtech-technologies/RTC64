#include "pro_os.h"
#include <stdint.h>

/* Sovereign AHCI Driver - Professional Architectural Skeleton
 * Serial ATA AHCI 1.3.1 specification.
 */

#define AHCI_REG_GHC 0x04
#define AHCI_REG_IS  0x08
#define AHCI_REG_PI  0x0c
#define AHCI_REG_VS  0x10

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
} ahci_port_t;

void ahci_init(uint64_t base_addr) {
    if (base_addr == 0) return;
    volatile uint32_t* ghc = (volatile uint32_t*)(base_addr + hhdm_offset);

    /* 1. Global Host Control Reset */
    /* Set AE (AHCI Enable) bit */

    /* 2. Port Enumeration */
    /* Check PI (Ports Implemented) register */

    /* 3. Port Initialization */
    /* For each implemented port: */
    /*   - Stop CMD/FIS engines */
    /*   - Allocate Command List and FIS area */
    /*   - Start Engines */

    /* 4. Device Detection */
    /* Check SSTS (System Status) for device presence */
}
