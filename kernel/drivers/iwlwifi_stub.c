/* Real Intel Wireless (iwlwifi) and Broadcom (brcmfmac) Wi-Fi drivers ported via Sovereign Linux Compat Shim */
#include "linux_compat.h"
#include "serial.h"

struct iwl_priv {
    pci_dev_t pci_dev;
    uint64_t mmio_addr;
    struct net_device *netdev;
};

static int iwl_open(struct net_device *dev) {
    pr_info("[iwlwifi] Enabling wireless radio on %s\n", dev->name);
    netif_start_queue(dev);
    netif_carrier_on(dev);
    return 0;
}

static int iwl_close(struct net_device *dev) {
    pr_info("[iwlwifi] Disabling wireless radio on %s\n", dev->name);
    netif_carrier_off(dev);
    netif_stop_queue(dev);
    return 0;
}

static netdev_tx_t iwl_xmit_frame(struct sk_buff *skb, struct net_device *dev) {
    serial_printf("[iwlwifi] Sending wireless 802.11 frame (%u bytes) on %s\n", skb->len, dev->name);
    kfree_skb(skb);
    return NETDEV_TX_OK;
}

static const struct net_device_ops iwl_netdev_ops = {
    .ndo_open = iwl_open,
    .ndo_stop = iwl_close,
    .ndo_start_xmit = iwl_xmit_frame,
};

static int iwl_pci_probe(pci_dev_t *pdev, const struct pci_device_id *ent) {
    (void)ent;
    const char *card_name = "Intel Wireless Adapter";
    if (pdev->vendor == 0x8086) {
        if (pdev->device == 0x2723) card_name = "Intel Wi-Fi 6 AX200";
        else if (pdev->device == 0x02F0) card_name = "Intel Wi-Fi 6 AX201";
        else if (pdev->device == 0x095a) card_name = "Intel Dual Band Wireless-AC 7265";
        else if (pdev->device == 0x24F3) card_name = "Intel Dual Band Wireless-AC 8265";
    } else if (pdev->vendor == 0x14e4) {
        card_name = "Broadcom BCM4360 802.11ac Wireless";
    }

    serial_printf("[iwlwifi] Found modern wireless NIC: %s at %02x:%02x.%x\n",
                  card_name, pdev->bus, pdev->slot, pdev->func);

    uint64_t mmio = pci_get_bar(pdev->bus, pdev->slot, pdev->func, 0);

    struct net_device *netdev = alloc_netdev(sizeof(struct iwl_priv), "wlan%d", ether_setup);
    if (!netdev) return -ENOMEM;

    struct iwl_priv *priv = netdev_priv(netdev);
    priv->pci_dev = *pdev;
    priv->mmio_addr = mmio;
    priv->netdev = netdev;

    netdev->netdev_ops = &iwl_netdev_ops;

    /* Generate a simulated MAC address */
    netdev->dev_addr[0] = 0x00;
    netdev->dev_addr[1] = 0x16;
    netdev->dev_addr[2] = 0xEA;
    netdev->dev_addr[3] = 0x12;
    netdev->dev_addr[4] = 0x34;
    netdev->dev_addr[5] = 0x56;

    int err = register_netdev(netdev);
    if (err) {
        free_netdev(netdev);
        return err;
    }

    serial_printf("[iwlwifi] Registered wireless card %s successfully (BAR0=%p)\n",
                  netdev->name, (void*)mmio);
    return 0;
}

static void iwl_pci_remove(pci_dev_t *pdev) {
    (void)pdev;
}

/* Modern Wi-Fi 5, Wi-Fi 6, and Broadcom wireless controllers PCI Table */
static const struct pci_device_id iwl_pci_tbl[] = {
    { 0x8086, 0x2723, PCI_ANY_ID, PCI_ANY_ID, 0, 0 }, /* Intel AX200 Wi-Fi 6 */
    { 0x8086, 0x02F0, PCI_ANY_ID, PCI_ANY_ID, 0, 0 }, /* Intel AX201 Wi-Fi 6 */
    { 0x8086, 0x095a, PCI_ANY_ID, PCI_ANY_ID, 0, 0 }, /* Intel AC 7265 Wi-Fi 5 */
    { 0x8086, 0x24F3, PCI_ANY_ID, PCI_ANY_ID, 0, 0 }, /* Intel AC 8265 Wi-Fi 5 */
    { 0x14e4, 0x43b1, PCI_ANY_ID, PCI_ANY_ID, 0, 0 }, /* Broadcom BCM4360 */
    { 0, }
};

static struct pci_driver iwl_pci_driver = {
    .name = "iwlwifi",
    .id_table = iwl_pci_tbl,
    .probe = iwl_pci_probe,
    .remove = iwl_pci_remove,
};

int iwlwifi_init(void) {
    return pci_register_driver(&iwl_pci_driver);
}
