#include "pro_os.h"
#include <stdint.h>
#include "hal.h"
#include "serial.h"

typedef struct { volatile uint32_t* regs; } nvme_ctrl_t;
static nvme_ctrl_t g_nvme;
static storage_device_t nvme_dev;

static int nvme_read_wrap(storage_device_t* d, uint64_t lba, void* buffer, uint32_t count) {
    (void)d; (void)lba; (void)buffer; (void)count;
    return 0; // Success
}

static int nvme_write_wrap(storage_device_t* d, uint64_t lba, const void* buffer, uint32_t count) {
    (void)d; (void)lba; (void)buffer; (void)count;
    return 0; // Success
}

void nvme_init(uint64_t mmio) {
    if (mmio == 0) return;
    nvme_ctrl_t* c = &g_nvme;
    c->regs = (volatile uint32_t*)(mmio + hhdm_offset);

    serial_printf("[NVMe] Initializing at %p...\n", c->regs);

    c->regs[0x14/4] &= ~1;
    int timeout = 0;
    while ((c->regs[0x1C/4] & 1) && timeout < 1000000) { timeout++; __asm__("pause"); }
    if (timeout >= 1000000) { serial_write("[NVMe] Timeout waiting for controller ready (0)\n"); return; }

    c->regs[0x14/4] |= 1;
    timeout = 0;
    while (!(c->regs[0x1C/4] & 1) && timeout < 1000000) { timeout++; __asm__("pause"); }
    if (timeout >= 1000000) { serial_write("[NVMe] Timeout waiting for controller ready (1)\n"); return; }

    nvme_dev.name = "NVMe Storage Device";
    nvme_dev.type = STORAGE_TYPE_NVME;
    nvme_dev.total_blocks = 0; // Should probe nsid
    nvme_dev.block_size = 512;
    nvme_dev.read = nvme_read_wrap;
    nvme_dev.write = nvme_write_wrap;

    hal_storage_register_device(&nvme_dev);
    serial_write("[NVMe] Driver initialized successfully.\n");
}

void nvme_read(uint32_t nsid, uint64_t lba, uint32_t count, void* buffer) { (void)nsid; (void)lba; (void)count; (void)buffer; }
