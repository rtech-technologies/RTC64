#include "pro_os.h"
#include <stdint.h>
#include <string.h>
#include "external/tlsf.h"

/* Genuine NVMe Driver Logic - Register Mapping & Initialization */

#define NVME_REG_CAP 0x00
#define NVME_REG_VS  0x08
#define NVME_REG_INTMS 0x0C
#define NVME_REG_INTMC 0x10
#define NVME_REG_CC  0x14
#define NVME_REG_CSTS 0x1C
#define NVME_REG_NSSR 0x20
#define NVME_REG_AQA 0x24
#define NVME_REG_ASQ 0x28
#define NVME_REG_ACQ 0x30

#define MAX_NVME 4
#define NVME_AQ_DEPTH 32

extern void* tlsf_get_global(void);

typedef struct {
    uint64_t mmio;
    uint64_t *admin_sq;
    uint64_t *admin_cq;
    storage_device_t dev;
} nvme_ctrl_t;

static nvme_ctrl_t g_nvme_controllers[MAX_NVME];
static int g_nvme_count = 0;

int nvme_init(uint64_t mmio) {
    if (mmio == 0 || g_nvme_count >= MAX_NVME) return -1;

    nvme_ctrl_t *c = &g_nvme_controllers[g_nvme_count];
    c->mmio = mmio;

    volatile uint32_t* regs = (volatile uint32_t*)(mmio + hhdm_offset);
    volatile uint64_t* regs64 = (volatile uint64_t*)(mmio + hhdm_offset);

    /* 1. Reset Controller: CC.EN = 0 */
    regs[NVME_REG_CC/4] &= ~1;
    int timeout = 0;
    while ((regs[NVME_REG_CSTS/4] & 1) && timeout++ < 1000000);

    /* 2. Allocate Admin Queues */
    c->admin_sq = (uint64_t *)tlsf_malloc(tlsf_get_global(), NVME_AQ_DEPTH * 64); /* 64 bytes per entry */
    c->admin_cq = (uint64_t *)tlsf_malloc(tlsf_get_global(), NVME_AQ_DEPTH * 16); /* 16 bytes per entry */
    
    if (!c->admin_sq || !c->admin_cq) {
        return -1;
    }
    
    memset(c->admin_sq, 0, NVME_AQ_DEPTH * 64);
    memset(c->admin_cq, 0, NVME_AQ_DEPTH * 16);

    /* 3. Configure Admin Queues */
    regs64[NVME_REG_ASQ/8] = (uint64_t)c->admin_sq - hhdm_offset; /* Admin Submit Queue */
    regs64[NVME_REG_ACQ/8] = (uint64_t)c->admin_cq - hhdm_offset; /* Admin Completion Queue */
    
    regs[NVME_REG_AQA/4] = ((NVME_AQ_DEPTH - 1) << 16) | (NVME_AQ_DEPTH - 1); /* Queue depth */

    /* 4. Enable Controller: CC.EN = 1 */
    regs[NVME_REG_CC/4] |= 1;
    timeout = 0;
    while (!(regs[NVME_REG_CSTS/4] & 1) && timeout++ < 1000000);

    /* 5. Register as storage device */
    c->dev.name = "NVMe Storage Device";
    c->dev.type = STORAGE_TYPE_NVME;
    c->dev.total_blocks = 1024*1024; /* Dummy - would be read from namespace */
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
    /* Full NVMe read would:
     * 1. Format Read command in admin queue
     * 2. Ring doorbell to submit command
     * 3. Wait for completion in completion queue
     * 4. Transfer data to buffer
     */
}
