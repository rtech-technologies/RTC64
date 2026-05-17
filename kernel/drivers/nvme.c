#include "pro_os.h"
#include <stdint.h>

/* Sovereign NVMe Driver - Native implementation */

#define NVME_REG_CC 0x14
#define NVME_REG_CSTS 0x1C

void nvme_init(uint64_t base_addr) {
    if (base_addr == 0) return;
    volatile uint32_t* regs = (volatile uint32_t*)(base_addr + hhdm_offset);

    /* 1. Controller Reset and Configuration */
    /* NVMe specification requires disabling CC.EN before configuration */
    regs[NVME_REG_CC/4] &= ~0x01;

    /* 2. Setup Admin Queues (Admin Submission/Completion Queues) */
    /* Initialization of memory-backed queues deferred to filesystem-service */

    /* 3. Enable controller */
    regs[NVME_REG_CC/4] |= 0x01;
}
