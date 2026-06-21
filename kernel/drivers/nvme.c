/* Copyright (C) 2025 Sovereign RTC64 Project. All rights reserved.
 * Licensed under the 'respect people's property' OS license. */
#include "pro_os.h"
#include <stdint.h>
#include <string.h>
#include "serial.h"
#include "hal.h"

#define NVME_REG_CC      0x14
#define NVME_REG_CSTS    0x1C
#define NVME_REG_AQA     0x24
#define NVME_REG_ASQ     0x28
#define NVME_REG_ACQ     0x30
#define NVME_REG_SQ0TDBL 0x1000

extern uint64_t hhdm_offset;
extern void* pmm_alloc_low(void);

typedef struct {
    uint32_t cdw0, nsid, rsvd2, rsvd3, mptr_l, mptr_h, dptr[2], cdw10, cdw11, cdw12, cdw13, cdw14, cdw15;
} nvme_cmd_t;

static uint64_t nvme_base = 0;
static void* sq0_virt = NULL;
static void* cq0_virt = NULL;
static uint16_t sq0_tail = 0;
static uint16_t cq0_head = 0;

static storage_device_t g_nvme_dev;

static int nvme_io_wrapper(storage_device_t* dev, uint64_t lba, void* buffer, uint32_t count, int write) {
    (void)dev;
    if (!nvme_base || !sq0_virt) return -1;

    nvme_cmd_t* cmd = &((nvme_cmd_t*)sq0_virt)[sq0_tail];
    memset(cmd, 0, sizeof(nvme_cmd_t));
    cmd->cdw0 = write ? 0x01 : 0x02; /* Write or Read */
    cmd->nsid = 1;
    cmd->dptr[0] = (uint32_t)(uintptr_t)buffer;
    cmd->dptr[1] = (uint32_t)((uintptr_t)buffer >> 32);
    cmd->cdw10 = (uint32_t)lba;
    cmd->cdw11 = (uint32_t)(lba >> 32);
    cmd->cdw12 = (count - 1);

    sq0_tail = (sq0_tail + 1) % 64;
    *(volatile uint32_t*)(nvme_base + NVME_REG_SQ0TDBL) = sq0_tail;

    /* Wait for completion */
    volatile uint32_t* cq = (volatile uint32_t*)cq0_virt;
    int timeout = 0;
    while (!(cq[cq0_head * 4 + 3] & 0x1) && timeout++ < 1000000) __asm__("pause");
    cq0_head = (cq0_head + 1) % 64;

    return (timeout < 1000000) ? 0 : -1;
}

static int nvme_read(storage_device_t* dev, uint64_t sector, void* buffer, uint32_t count) {
    return nvme_io_wrapper(dev, sector, buffer, count, 0);
}

static int nvme_write(storage_device_t* dev, uint64_t sector, const void* buffer, uint32_t count) {
    return nvme_io_wrapper(dev, sector, (void*)buffer, count, 1);
}

int nvme_init(uint64_t mmio) {
    if (mmio == 0) return -1;
    nvme_base = mmio + hhdm_offset;
    serial_printf("[NVME] Initializing BAR at %p\n", (void*)nvme_base);

    volatile uint32_t* regs = (volatile uint32_t*)nvme_base;
    regs[NVME_REG_CC/4] &= ~1;
    while ((regs[NVME_REG_CSTS/4] & 1)) __asm__("pause");

    void* asq_phys = pmm_alloc_low();
    void* acq_phys = pmm_alloc_low();
    sq0_virt = (void*)((uint64_t)asq_phys + hhdm_offset);
    cq0_virt = (void*)((uint64_t)acq_phys + hhdm_offset);
    memset(sq0_virt, 0, 4096);
    memset(cq0_virt, 0, 4096);

    regs[NVME_REG_AQA/4] = (63 << 16) | 63;
    *(volatile uint64_t*)(nvme_base + NVME_REG_ASQ) = (uint64_t)asq_phys;
    *(volatile uint64_t*)(nvme_base + NVME_REG_ACQ) = (uint64_t)acq_phys;

    regs[NVME_REG_CC/4] = (0 << 16) | (0 << 14) | (4 << 11) | (0 << 7) | 1;
    while (!(regs[NVME_REG_CSTS/4] & 1)) __asm__("pause");

    g_nvme_dev.name = "Sovereign NVMe Drive";
    g_nvme_dev.type = STORAGE_TYPE_NVME;
    g_nvme_dev.total_blocks = 2000000;
    g_nvme_dev.block_size = 512;
    g_nvme_dev.read = nvme_read;
    g_nvme_dev.write = nvme_write;
    hal_storage_register_device(&g_nvme_dev);

    serial_printf("[NVME] Initialized and registered.\n");
    return 0;
}
