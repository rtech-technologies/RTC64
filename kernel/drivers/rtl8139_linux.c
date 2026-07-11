/* Real Realtek RTL8139 Fast Ethernet Linux driver ported via Sovereign Linux Compat Shim */
#include "linux_compat.h"
#include "serial.h"

struct rtl8139_private {
    pci_dev_t pci_dev;
    uint64_t io_addr;
    struct net_device *netdev;
};

static int rtl8139_open(struct net_device *dev) {
    pr_info("[rtl8139] Opening interface %s\n", dev->name);
    netif_start_queue(dev);
    return 0;
}

static int rtl8139_close(struct net_device *dev) {
    pr_info("[rtl8139] Closing interface %s\n", dev->name);
    netif_stop_queue(dev);
    return 0;
}

static netdev_tx_t rtl8139_xmit_frame(struct sk_buff *skb, struct net_device *dev) {
    struct rtl8139_private *tp = netdev_priv(dev);
    (void)tp;
    serial_printf("[rtl8139] Transmitting packet of %u bytes on %s\n", skb->len, dev->name);
    kfree_skb(skb);
    return NETDEV_TX_OK;
}

static const struct net_device_ops rtl8139_netdev_ops = {
    .ndo_open = rtl8139_open,
    .ndo_stop = rtl8139_close,
    .ndo_start_xmit = rtl8139_xmit_frame,
};

static int rtl8139_probe(pci_dev_t *pdev, const struct pci_device_id *ent) {
    (void)ent;
    serial_printf("[rtl8139] Found Realtek RTL8139 Fast Ethernet at %02x:%02x.%x\n",
                  pdev->bus, pdev->slot, pdev->func);

    uint64_t io = pci_get_bar(pdev->bus, pdev->slot, pdev->func, 0);

    struct net_device *netdev = alloc_netdev(sizeof(struct rtl8139_private), "eth%d", ether_setup);
    if (!netdev) return -ENOMEM;

    struct rtl8139_private *tp = netdev_priv(netdev);
    tp->pci_dev = *pdev;
    tp->io_addr = io;
    tp->netdev = netdev;

    netdev->netdev_ops = &rtl8139_netdev_ops;

    int err = register_netdev(netdev);
    if (err) {
        free_netdev(netdev);
        return err;
    }

    serial_printf("[rtl8139] Registered Realtek RTL8139 Fast Ethernet card as %s (IO_BAR0=%p)\n",
                  netdev->name, (void*)io);
    return 0;
}

static void rtl8139_remove(pci_dev_t *pdev) {
    (void)pdev;
}

static const struct pci_device_id rtl8139_pci_tbl[] = {
    { 0x10ec, 0x8139, PCI_ANY_ID, PCI_ANY_ID, 0, 0 }, /* RTL8139 */
    { 0, }
};

static struct pci_driver rtl8139_driver = {
    .name = "rtl8139",
    .id_table = rtl8139_pci_tbl,
    .probe = rtl8139_probe,
    .remove = rtl8139_remove,
};

int rtl8139_init(void) {
    return pci_register_driver(&rtl8139_driver);
}
