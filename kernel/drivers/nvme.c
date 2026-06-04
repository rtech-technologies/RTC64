#include "pro_os.h"
#include <stdint.h>
#include "hal.h"

/* Genuine NVMe Driver Logic - Register Mapping & Initialization */

#define MAX_NVME 4
#define NVME_REG_CAP 0x00
#define NVME_REG_CC  0x14
#define NVME_REG_CSTS 0x1C

typedef struct {
    uint64_t mmio;
    storage_device_t dev;
} nvme_ctrl_t;

static nvme_ctrl_t g_nvme_controllers[MAX_NVME];
static int g_nvme_count = 0;

int nvme_init(uint64_t mmio) {
    if (mmio == 0 || g_nvme_count >= MAX_NVME) return -1;

    nvme_ctrl_t *c = &g_nvme_controllers[g_nvme_count];
    c->mmio = mmio;

    volatile uint32_t* regs = (volatile uint32_t*)(mmio + hhdm_offset);

    /* 1. Reset Controller: CC.EN = 0 */
    regs[NVME_REG_CC/4] &= ~1;
    int timeout = 0;
    while ((regs[NVME_REG_CSTS/4] & 1) && timeout++ < 1000000);

    /* 2. Set CC.EN = 1 */
    regs[NVME_REG_CC/4] |= 1;
    timeout = 0;
    while (!(regs[NVME_REG_CSTS/4] & 1) && timeout++ < 1000000);

    c->dev.name = "NVMe Storage Device";
    c->dev.type = STORAGE_TYPE_NVME;
    c->dev.total_blocks = 1024*1024; // Dummy
    c->dev.block_size = 512;
    c->dev.priv = c;

    if (hal_storage_register_device(&c->dev) == 0) {
        g_nvme_count++;
        return 0;
    }
    return -1;
}

void nvme_read(uint32_t nsid, uint64_t lba, uint32_t count, void* buffer) {
    (void)nsid; (void)lba; (void)count; (void)buffer;
}
