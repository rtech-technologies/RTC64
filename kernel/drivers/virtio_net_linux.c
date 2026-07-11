/* Minimal Linux-compatible virtio-net driver stub.
 * This uses the lightweight linux_compat layer to register a PCI driver.
 */
#include "linux_compat.h"
#include "serial.h"
#include "pro_os.h"

static const struct pci_device_id virtio_net_pci_ids[] = {
    /* VirtIO-Net controllers */
    { 0x1af4, 0x1000, PCI_ANY_ID, PCI_ANY_ID, 0, 0 },
    { 0x1af4, 0x1041, PCI_ANY_ID, PCI_ANY_ID, 0, 0 },

    /* Intel e1000 / e1000e Gigabit Ethernet controllers (8086) */
    { 0x8086, 0x100e, PCI_ANY_ID, PCI_ANY_ID, 0, 0 }, /* 82540EM (QEMU default) */
    { 0x8086, 0x100f, PCI_ANY_ID, PCI_ANY_ID, 0, 0 }, /* 82545EM */
    { 0x8086, 0x10d3, PCI_ANY_ID, PCI_ANY_ID, 0, 0 }, /* 82574L */
    { 0x8086, 0x10ea, PCI_ANY_ID, PCI_ANY_ID, 0, 0 }, /* 82577LM */
    { 0x8086, 0x153a, PCI_ANY_ID, PCI_ANY_ID, 0, 0 }, /* I217-LM */
    { 0x8086, 0x153b, PCI_ANY_ID, PCI_ANY_ID, 0, 0 }, /* I217-V */
    { 0x8086, 0x15bc, PCI_ANY_ID, PCI_ANY_ID, 0, 0 }, /* I219-LM */
    { 0x8086, 0x15bd, PCI_ANY_ID, PCI_ANY_ID, 0, 0 }, /* I219-V */

    /* Realtek RTL8139 / RTL8169 Fast and Gigabit controllers (10EC) */
    { 0x10ec, 0x8139, PCI_ANY_ID, PCI_ANY_ID, 0, 0 }, /* RTL8139 */
    { 0x10ec, 0x8168, PCI_ANY_ID, PCI_ANY_ID, 0, 0 }, /* RTL8111/8168 */
    { 0x10ec, 0x8169, PCI_ANY_ID, PCI_ANY_ID, 0, 0 }, /* RTL8169 */

    /* Broadcom NetXtreme Gigabit Ethernet controllers (14E4) */
    { 0x14e4, 0x1659, PCI_ANY_ID, PCI_ANY_ID, 0, 0 }, /* BCM5721 */
    { 0x14e4, 0x165f, PCI_ANY_ID, PCI_ANY_ID, 0, 0 }, /* BCM5722 */

    /* AMD PCnet Fast III Ethernet controllers (1022) */
    { 0x1022, 0x2000, PCI_ANY_ID, PCI_ANY_ID, 0, 0 }, /* Am79C973 */

    { PCI_ANY_ID, PCI_ANY_ID, PCI_ANY_ID, PCI_ANY_ID, 0, 0 }
};

struct virtio_net {
    pci_dev_t dev;
    uint64_t bar0;
};

static int virtio_net_probe(pci_dev_t *dev, const struct pci_device_id *id) {
    (void)id;
    serial_printf("[VIRTIO_LINUX] Probe called for %02x:%02x.%x vendor=%04x device=%04x\n",
                  dev->bus, dev->slot, dev->func, dev->vendor, dev->device);
    uint64_t bar0 = pci_get_bar(dev->bus, dev->slot, dev->func, 0);
    serial_printf("[VIRTIO_LINUX] BAR0=%p\n", (void*)bar0);

    struct virtio_net *vnet = kzalloc(sizeof(*vnet), GFP_KERNEL);
    if (!vnet) {
        pr_err("[VIRTIO_LINUX] Allocation failed\n");
        return -1;
    }
    vnet->dev = *dev;
    vnet->bar0 = bar0;

    /* Real virtio initialization will go here later. */
    return 0;
}

static void virtio_net_remove(pci_dev_t *dev) {
    serial_printf("[VIRTIO_LINUX] Remove called for %02x:%02x.%x\n",
                  dev->bus, dev->slot, dev->func);
}

static struct pci_driver virtio_net_driver = {
    .name = "virtio_net",
    .id_table = virtio_net_pci_ids,
    .probe = virtio_net_probe,
    .remove = virtio_net_remove,
};

int virtio_net_linux_init(void) {
    return pci_register_driver(&virtio_net_driver);
}
