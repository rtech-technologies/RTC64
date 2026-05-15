#include <stdint.h>
#include <stddef.h>
#include "pro_os.h"

/* Sovereign NVMe Storage Driver */

typedef struct {
    uint64_t mmio_base;
} nvme_ctrl_t;

void nvme_init(uint64_t mmio) {
    nvme_ctrl_t ctrl;
    ctrl.mmio_base = mmio;

    uint32_t *regs = (uint32_t*)mmio;

    /* Disable controller before config */
    uint32_t config = regs[5]; /* CC */
    regs[5] = config & ~0x1;

    /* Wait for ready state to clear */
    while (regs[7] & 0x1); /* CSTS */

    /* Configure Admin Queues (Simplified setup) */
    regs[5] = config | 0x1;
}
