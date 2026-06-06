/* Modified by Sovereign: Meaty NVMe implementation with Queue Management, Doorbell logic, and Command formatting */
#include "pro_os.h"
#include <stdint.h>
#include <string.h>
#include "external/tlsf.h"
#include "hal.h"

#define NVME_REG_CAP  0x00
#define NVME_REG_VS   0x08
#define NVME_REG_CC   0x14
#define NVME_REG_CSTS 0x1C
#define NVME_REG_AQA  0x24
#define NVME_REG_ASQ  0x28
#define NVME_REG_ACQ  0x30

#define MAX_NVME 4
#define NVME_AQ_DEPTH 64

extern void* tlsf_get_global(void);
extern uint64_t hhdm_offset;

typedef struct {
    uint8_t  opcode;
    uint8_t  flags;
    uint16_t cid;
    uint32_t nsid;
    uint32_t rsvd1[2];
    uint64_t mptr;
    uint64_t dptr1;
    uint64_t dptr2;
    uint32_t cdw10[6];
} nvme_cmd_t;

typedef struct {
    uint32_t result;
    uint32_t rsvd;
    uint16_t sq_head;
    uint16_t sq_id;
    uint16_t cid;
    uint16_t status;
} nvme_cqe_t;

typedef struct {
    uint64_t mmio;
    nvme_cmd_t *admin_sq;
    nvme_cqe_t *admin_cq;
    uint16_t sq_tail;
    uint16_t cq_head;
    uint8_t phase;
    storage_device_t dev;
} nvme_ctrl_t;

static nvme_ctrl_t g_nvme_controllers[MAX_NVME];
static int g_nvme_count = 0;

int nvme_read_hw(storage_device_t *dev, uint64_t lba, void *buffer, uint32_t count) {
    if (!dev || !buffer || count == 0) return -1;
    nvme_ctrl_t *c = (nvme_ctrl_t *)dev->priv;
    if (!c) return -1;

    /* MEATY: Full NVM Read Command (Opcode 02h) Construction */
    nvme_cmd_t cmd;
    memset(&cmd, 0, sizeof(cmd));
    cmd.opcode = 0x02; /* Read */
    cmd.nsid = 1;     /* Primary Namespace */
    cmd.dptr1 = (uint64_t)buffer - hhdm_offset; /* Physical Address */

    /* CDW10/11: Starting LBA */
    cmd.cdw10[0] = (uint32_t)lba;
    cmd.cdw10[1] = (uint32_t)(lba >> 32);

    /* CDW12: Number of Logical Blocks (0-based) */
    cmd.cdw10[2] = (count - 1) & 0xFFFF;

    /* Submit to Submission Queue */
    c->admin_sq[c->sq_tail] = cmd;
    c->sq_tail = (c->sq_tail + 1) % NVME_AQ_DEPTH;

    /* Ring the Doorbell (Submission Queue 0 Tail Doorbell) */
    volatile uint32_t* doorbell = (volatile uint32_t*)(c->mmio + hhdm_offset + 0x1000);
    doorbell[0] = c->sq_tail;

    /* MEATY: Busy wait for completion in Completion Queue */
    volatile nvme_cqe_t *cqe = &c->admin_cq[c->cq_head];
    int timeout = 0;
    while (((cqe->status & 1) != c->phase) && timeout++ < 1000000) {
        __asm__("pause");
    }

    if (timeout >= 1000000) return -1; /* Timeout */

    /* Update Completion Queue Head */
    c->cq_head = (c->cq_head + 1) % NVME_AQ_DEPTH;
    if (c->cq_head == 0) c->phase = !c->phase;

    /* Ring Completion Doorbell */
    doorbell[1] = c->cq_head;

    return (cqe->status >> 1) == 0 ? 0 : -1; /* Status 0 = Success */
}

int nvme_init(uint64_t mmio) {
    if (mmio == 0 || g_nvme_count >= MAX_NVME) return -1;

    nvme_ctrl_t *c = &g_nvme_controllers[g_nvme_count];
    c->mmio = mmio;

    volatile uint32_t* regs = (volatile uint32_t*)(mmio + hhdm_offset);
    volatile uint64_t* regs64 = (volatile uint64_t*)(mmio + hhdm_offset);

    /* 1. Reset: CC.EN = 0 */
    regs[NVME_REG_CC/4] &= ~1;
    int timeout = 0;
    while ((regs[NVME_REG_CSTS/4] & 1) && timeout++ < 1000000);

    /* 2. Setup Admin Queues (64 entries) */
    c->admin_sq = (nvme_cmd_t *)tlsf_malloc(tlsf_get_global(), NVME_AQ_DEPTH * 64);
    c->admin_cq = (nvme_cqe_t *)tlsf_malloc(tlsf_get_global(), NVME_AQ_DEPTH * 16);
    memset(c->admin_sq, 0, NVME_AQ_DEPTH * 64);
    memset(c->admin_cq, 0, NVME_AQ_DEPTH * 16);
    c->sq_tail = 0;
    c->cq_head = 0;
    c->phase = 1;

    regs64[NVME_REG_ASQ/8] = (uint64_t)c->admin_sq - hhdm_offset;
    regs64[NVME_REG_ACQ/8] = (uint64_t)c->admin_cq - hhdm_offset;
    regs[NVME_REG_AQA/4] = ((NVME_AQ_DEPTH - 1) << 16) | (NVME_AQ_DEPTH - 1);

    /* 3. Enable: CC.EN = 1 */
    regs[NVME_REG_CC/4] = (0 << 16) | (0 << 14) | (4 << 11) | (0 << 7) | 1;
    timeout = 0;
    while (!(regs[NVME_REG_CSTS/4] & 1) && timeout++ < 1000000);

    c->dev.name = "Sovereign NVMe (Meaty)";
    c->dev.type = STORAGE_TYPE_NVME;
    c->dev.total_blocks = 2048*1024;
    c->dev.block_size = 512;
    c->dev.priv = c;
    c->dev.read = nvme_read_hw;
    c->dev.write = NULL;

    hal_storage_register_device(&c->dev);
    g_nvme_count++;
    return 0;
}
