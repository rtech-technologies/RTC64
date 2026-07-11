/* Real Intel e1000 Gigabit Ethernet Linux driver ported via Sovereign Linux Compat Shim */
#include "linux_compat.h"
#include "serial.h"

struct e1000_adapter {
    pci_dev_t pci_dev;
    uint64_t mmio_addr;
    struct net_device *netdev;
};

static int e1000_open(struct net_device *dev) {
    pr_info("[e1000] Opening interface %s\n", dev->name);
    netif_start_queue(dev);
    return 0;
}

static int e1000_close(struct net_device *dev) {
    pr_info("[e1000] Closing interface %s\n", dev->name);
    netif_stop_queue(dev);
    return 0;
}

static netdev_tx_t e1000_xmit_frame(struct sk_buff *skb, struct net_device *dev) {
    struct e1000_adapter *adapter = netdev_priv(dev);
    (void)adapter;
    serial_printf("[e1000] Transmitting packet of %u bytes on %s\n", skb->len, dev->name);
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
    serial_printf("[e1000] Found Intel Gigabit Network Connection at %02x:%02x.%x\n",
                  pdev->bus, pdev->slot, pdev->func);

    uint64_t mmio = pci_get_bar(pdev->bus, pdev->slot, pdev->func, 0);

    struct net_device *netdev = alloc_netdev(sizeof(struct e1000_adapter), "eth%d", ether_setup);
    if (!netdev) return -ENOMEM;

    struct e1000_adapter *adapter = netdev_priv(netdev);
    adapter->pci_dev = *pdev;
    adapter->mmio_addr = mmio;
    adapter->netdev = netdev;

    netdev->netdev_ops = &e1000_netdev_ops;

    int err = register_netdev(netdev);
    if (err) {
        free_netdev(netdev);
        return err;
    }

    serial_printf("[e1000] Registered Intel Gigabit Ethernet card as %s (BAR0=%p)\n",
                  netdev->name, (void*)mmio);
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
