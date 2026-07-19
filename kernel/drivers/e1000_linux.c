/* Copyright (C) 2025 Sovereign RTC64 Project. All rights reserved.
 * Licensed under the 'respect people's property' OS license.
 * Bare-metal Intel e1000 (82540EM) Gigabit Ethernet Driver. */
#include "linux_compat.h"
#include "serial.h"
#include "pro_os.h"
#include <string.h>

#define E1000_REG_CTRL      0x0000
#define E1000_REG_STATUS    0x0008
#define E1000_REG_EECD      0x0010
#define E1000_REG_IMS       0x00D0
#define E1000_REG_IMC       0x00D8
#define E1000_REG_RCTL      0x0100
#define E1000_REG_TCTL      0x0400
#define E1000_REG_TIPG      0x0410
#define E1000_REG_RDBAL     0x2800
#define E1000_REG_RDBAH     0x2804
#define E1000_REG_RDLEN     0x2808
#define E1000_REG_RDH       0x2810
#define E1000_REG_RDT       0x2818
#define E1000_REG_TDBAL     0x3800
#define E1000_REG_TDBAH     0x3804
#define E1000_REG_TDLEN     0x3808
#define E1000_REG_TDH       0x3810
#define E1000_REG_TDT       0x3818
#define E1000_REG_RAL       0x5400
#define E1000_REG_RAH       0x5404

struct e1000_tx_desc {
    uint64_t addr;
    uint16_t length;
    uint8_t cso;
    uint8_t cmd;
    uint8_t status;
    uint8_t css;
    uint16_t special;
} __attribute__((packed));

struct e1000_rx_desc {
    uint64_t addr;
    uint16_t length;
    uint16_t checksum;
    uint8_t status;
    uint8_t errors;
    uint16_t special;
} __attribute__((packed));

#define RING_SIZE 64

static uint64_t g_e1000_mmio = 0;
static struct e1000_tx_desc *g_tx_ring = NULL;
static struct e1000_rx_desc *g_rx_ring = NULL;
static uint64_t g_tx_ring_phys = 0;
static uint64_t g_rx_ring_phys = 0;

static void* g_tx_buffers[RING_SIZE];
static void* g_rx_buffers[RING_SIZE];
static uint64_t g_tx_buffers_phys[RING_SIZE];
static uint64_t g_rx_buffers_phys[RING_SIZE];

static uint16_t g_tx_tail = 0;
static uint16_t g_rx_tail = 0;

extern uint64_t hhdm_offset;
extern void* pmm_alloc_blocks(size_t count);

static inline void e1000_write(uint32_t reg, uint32_t val) {
    *(volatile uint32_t*)(g_e1000_mmio + reg) = val;
}

static inline uint32_t e1000_read(uint32_t reg) {
    return *(volatile uint32_t*)(g_e1000_mmio + reg);
}

int e1000_transmit(const void* data, uint32_t len) {
    if (!g_e1000_mmio || !g_tx_ring) return -1;
    if (len > 1518) return -1;

    uint32_t tail = g_tx_tail;
    struct e1000_tx_desc* desc = &g_tx_ring[tail];

    /* Copy frame data directly into pre-allocated contiguous low memory DMA packet buffer */
    memcpy(g_tx_buffers[tail], data, len);

    desc->addr = g_tx_buffers_phys[tail];
    desc->length = len;
    desc->cmd = (1 << 3) | (1 << 0); // RS (Report Status) | EOP (End of Packet)
    desc->status = 0;

    g_tx_tail = (tail + 1) % RING_SIZE;
    e1000_write(E1000_REG_TDT, g_tx_tail);

    /* Wait for transmission completion via descriptor status bit */
    int timeout = 0;
    while (!(desc->status & 0x1) && timeout++ < 1000000) {
        __asm__("pause");
    }

    return (timeout < 1000000) ? 0 : -1;
}

void e1000_poll(void) {
    if (!g_e1000_mmio || !g_rx_ring) return;

    uint32_t next = (g_rx_tail + 1) % RING_SIZE;
    struct e1000_rx_desc* desc = &g_rx_ring[next];

    while (desc->status & 0x1) { // Descriptor Done (DD)
        uint32_t len = desc->length;

        /* Feed packet directly to the network receive stack */
        extern void net_receive(void* packet, size_t len);
        net_receive(g_rx_buffers[next], len);

        desc->status = 0;
        g_rx_tail = next;
        e1000_write(E1000_REG_RDT, next);

        next = (next + 1) % RING_SIZE;
        desc = &g_rx_ring[next];
    }
}

static int e1000_open(struct net_device *dev) {
    (void)dev;
    return 0;
}

static int e1000_close(struct net_device *dev) {
    (void)dev;
    return 0;
}

static netdev_tx_t e1000_xmit_frame(struct sk_buff *skb, struct net_device *dev) {
    (void)dev;
    e1000_transmit(skb->data, skb->len);
    kfree_skb(skb);
    return NETDEV_TX_OK;
}

static const struct net_device_ops e1000_netdev_ops = {
    .ndo_open = e1000_open,
    .ndo_stop = e1000_close,
    .ndo_start_xmit = e1000_xmit_frame,
};

static int e1000_probe(pci_dev_t *pdev, const struct pci_device_id *ent) {
    (void)ent;
    serial_printf("[e1000] Probing Intel Gigabit NIC on %02x:%02x.%x\n", pdev->bus, pdev->slot, pdev->func);

    /* 1. Enable PCI Bus Master, I/O and Memory Space */
    uint32_t cmd = pci_read_config(pdev->bus, pdev->slot, pdev->func, 0x04);
    pci_write_config(pdev->bus, pdev->slot, pdev->func, 0x04, cmd | 0x07);

    /* 2. Map BAR0 registers */
    uint64_t bar0 = pci_get_bar(pdev->bus, pdev->slot, pdev->func, 0);
    g_e1000_mmio = bar0 + hhdm_offset;
    serial_printf("[e1000] MMIO mapped at %p\n", (void*)g_e1000_mmio);

    /* 3. Reset the device and disable interrupts */
    e1000_write(E1000_REG_IMC, 0xFFFFFFFF);
    e1000_write(E1000_REG_CTRL, e1000_read(E1000_REG_CTRL) | (1 << 26)); // Device Reset
    uint32_t status = e1000_read(E1000_REG_STATUS);
    serial_printf("[e1000] Hardware status: 0x%x\n", status);

    /* 4. Allocate Transmit & Receive Descriptor rings (aligned) */
    void* tx_page_phys = pmm_alloc_blocks(1);
    void* rx_page_phys = pmm_alloc_blocks(1);
    g_tx_ring_phys = (uint64_t)tx_page_phys;
    g_rx_ring_phys = (uint64_t)rx_page_phys;
    g_tx_ring = (struct e1000_tx_desc*)((uint64_t)tx_page_phys + hhdm_offset);
    g_rx_ring = (struct e1000_rx_desc*)((uint64_t)rx_page_phys + hhdm_offset);
    memset(g_tx_ring, 0, 4096);
    memset(g_rx_ring, 0, 4096);

    /* 5. Allocate Packet DMA buffers */
    for (int i = 0; i < RING_SIZE; i++) {
        void* tx_buf_phys = pmm_alloc_blocks(1);
        void* rx_buf_phys = pmm_alloc_blocks(1);

        g_tx_buffers_phys[i] = (uint64_t)tx_buf_phys;
        g_rx_buffers_phys[i] = (uint64_t)rx_buf_phys;

        g_tx_buffers[i] = (void*)((uint64_t)tx_buf_phys + hhdm_offset);
        g_rx_buffers[i] = (void*)((uint64_t)rx_buf_phys + hhdm_offset);

        memset(g_tx_buffers[i], 0, 4096);
        memset(g_rx_buffers[i], 0, 4096);
    }

    /* 6. Configure Transmit hardware registers */
    e1000_write(E1000_REG_TDBAL, (uint32_t)g_tx_ring_phys);
    e1000_write(E1000_REG_TDBAH, (uint32_t)(g_tx_ring_phys >> 32));
    e1000_write(E1000_REG_TDLEN, RING_SIZE * sizeof(struct e1000_tx_desc));
    e1000_write(E1000_REG_TDH, 0);
    e1000_write(E1000_REG_TDT, 0);
    e1000_write(E1000_REG_TIPG, 0x0060200A); // Standard Inter-Packet Gap
    e1000_write(E1000_REG_TCTL, (1 << 1) | (1 << 3) | (0xF << 4) | (0x40 << 12)); // TCTL.EN | TCTL.PSP

    /* 7. Configure Receive hardware registers */
    e1000_write(E1000_REG_RDBAL, (uint32_t)g_rx_ring_phys);
    e1000_write(E1000_REG_RDBAH, (uint32_t)(g_rx_ring_phys >> 32));
    e1000_write(E1000_REG_RDLEN, RING_SIZE * sizeof(struct e1000_rx_desc));
    e1000_write(E1000_REG_RDH, 0);
    e1000_write(E1000_REG_RDT, RING_SIZE - 1);
    g_rx_tail = RING_SIZE - 1;

    for (int i = 0; i < RING_SIZE; i++) {
        g_rx_ring[i].addr = g_rx_buffers_phys[i];
        g_rx_ring[i].status = 0;
    }

    /* Enable Receive, Broadcast Accept, strip CRC */
    e1000_write(E1000_REG_RCTL, (1 << 1) | (1 << 15) | (1 << 4) | (0 << 16));

    /* 8. Setup RAL/RAH to declare active hardware MAC */
    uint32_t ral = e1000_read(E1000_REG_RAL);
    uint32_t rah = e1000_read(E1000_REG_RAH);
    uint8_t mac[6];
    if (rah & (1 << 31)) {
        mac[0] = ral & 0xFF;
        mac[1] = (ral >> 8) & 0xFF;
        mac[2] = (ral >> 16) & 0xFF;
        mac[3] = (ral >> 24) & 0xFF;
        mac[4] = rah & 0xFF;
        mac[5] = (rah >> 8) & 0xFF;
    } else {
        /* Fallback MAC */
        mac[0] = 0x00; mac[1] = 0x16; mac[2] = 0xEA;
        mac[3] = 0x12; mac[4] = 0x34; mac[5] = 0x56;
        e1000_write(E1000_REG_RAL, 0x12EA1600);
        e1000_write(E1000_REG_RAH, 0x80005634); // Active valid bit
    }

    struct net_device *netdev = alloc_netdev(sizeof(int), "eth%d", ether_setup);
    if (!netdev) return -ENOMEM;
    memcpy(netdev->dev_addr, mac, 6);
    netdev->netdev_ops = &e1000_netdev_ops;

    if (register_netdev(netdev) != 0) {
        free_netdev(netdev);
        return -1;
    }

    serial_printf("[e1000] NATIVE Bare-Metal Gigabit Driver Initialized. MAC: %02x:%02x:%02x:%02x:%02x:%02x\n",
                  mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

    return 0;
}

static void e1000_remove(pci_dev_t *pdev) {
    (void)pdev;
}

static const struct pci_device_id e1000_pci_tbl[] = {
    { 0x8086, 0x100e, PCI_ANY_ID, PCI_ANY_ID, 0, 0 }, /* 82540EM (QEMU default) */
    { 0x8086, 0x100f, PCI_ANY_ID, PCI_ANY_ID, 0, 0 }, /* 82545EM */
    { 0x8086, 0x10d3, PCI_ANY_ID, PCI_ANY_ID, 0, 0 }, /* 82574L */
    { 0x8086, 0x153a, PCI_ANY_ID, PCI_ANY_ID, 0, 0 }, /* I217-LM */
    { 0x8086, 0x153b, PCI_ANY_ID, PCI_ANY_ID, 0, 0 }, /* I217-V */
    { 0, }
};

static struct pci_driver e1000_driver = {
    .name = "e1000",
    .id_table = e1000_pci_tbl,
    .probe = e1000_probe,
    .remove = e1000_remove,
};

int e1000_init(void) {
    return pci_register_driver(&e1000_driver);
}
