/* Minimal Linux-compatible virtio-net driver stub.
 * This uses the lightweight linux_compat layer to register a PCI driver.
 */
#include "linux_compat.h"
#include "serial.h"
#include "pro_os.h"

static const struct pci_device_id virtio_net_pci_ids[] = {
    { 0x1af4, 0x1000, PCI_ANY_ID, PCI_ANY_ID, 0, 0 },
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
