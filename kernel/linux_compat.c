/* Linux-compatible driver registration layer for basic PCI driver reuse. */
#include "linux_compat.h"
#include "serial.h"

#define MAX_LINUX_DRV 16

static struct pci_driver *g_linux_drivers[MAX_LINUX_DRV];
static int g_linux_driver_count = 0;

static bool pci_device_id_match(const struct pci_device_id *id,
                                uint16_t vendor, uint16_t device,
                                uint16_t subvendor, uint16_t subdevice,
                                uint32_t dev_class) {
    if (!id) return false;
    if (id->vendor != PCI_ANY_ID && id->vendor != vendor) return false;
    if (id->device != PCI_ANY_ID && id->device != device) return false;
    if (id->subvendor != PCI_ANY_ID && id->subvendor != subvendor) return false;
    if (id->subdevice != PCI_ANY_ID && id->subdevice != subdevice) return false;
    if (id->class != 0) {
        if ((id->class & 0xffffff00) != (dev_class & 0xffffff00)) return false;
    }
    return true;
}

int pci_register_driver(struct pci_driver *driver) {
    if (!driver || !driver->name || !driver->id_table) return -1;
    if (g_linux_driver_count >= MAX_LINUX_DRV) return -1;
    g_linux_drivers[g_linux_driver_count++] = driver;
    pr_info("[LINUX_COMPAT] Registered driver %s\n", driver->name);
    return 0;
}

int pci_unregister_driver(struct pci_driver *driver) {
    for (int i = 0; i < g_linux_driver_count; i++) {
        if (g_linux_drivers[i] == driver) {
            for (int j = i; j < g_linux_driver_count - 1; j++) {
                g_linux_drivers[j] = g_linux_drivers[j + 1];
            }
            g_linux_driver_count--;
            return 0;
        }
    }
    return -1;
}

void linux_compat_probe_pci_device(uint8_t bus, uint8_t slot, uint8_t func,
                                  uint16_t vendor, uint16_t device,
                                  uint16_t subsystem_vendor, uint16_t subsystem_device,
                                  uint8_t class, uint8_t subclass, uint8_t prog_if) {
    pci_dev_t dev = {
        .bus = bus,
        .slot = slot,
        .func = func,
        .vendor = vendor,
        .device = device,
        .subsystem_vendor = subsystem_vendor,
        .subsystem_device = subsystem_device,
        .class = class,
        .subclass = subclass,
        .prog_if = prog_if,
    };

    for (int i = 0; i < g_linux_driver_count; i++) {
        struct pci_driver *drv = g_linux_drivers[i];
        const struct pci_device_id *id = drv->id_table;
        for (; id && !(id->vendor == PCI_ANY_ID && id->device == PCI_ANY_ID &&
                        id->subvendor == PCI_ANY_ID && id->subdevice == PCI_ANY_ID &&
                        id->class == 0); id++) {
            if (pci_device_id_match(id, vendor, device, subsystem_vendor, subsystem_device,
                                    ((uint32_t)class << 16) | ((uint32_t)subclass << 8) | prog_if)) {
                pr_info("[LINUX_COMPAT] Probing %s for %02x:%02x.%x\n", drv->name, bus, slot, func);
                drv->probe(&dev, id);
                return;
            }
        }
    }
}

static struct net_device *g_registered_netdev = NULL;

struct net_device *alloc_netdev(int sizeof_priv, const char *name, void (*setup)(struct net_device *)) {
    if (!name) return NULL;
    size_t alloc_size = sizeof(struct net_device) + sizeof_priv;
    struct net_device *dev = malloc(alloc_size);
    if (!dev) return NULL;
    memset(dev, 0, alloc_size);
    if (setup) setup(dev);
    strncpy(dev->name, name, IFNAMSIZ - 1);
    dev->name[IFNAMSIZ - 1] = '\0';
    return dev;
}

void free_netdev(struct net_device *dev) {
    if (!dev) return;
    free(dev);
}

int register_netdev(struct net_device *dev) {
    if (!dev) return -EINVAL;
    g_registered_netdev = dev;
    return 0;
}

void unregister_netdev(struct net_device *dev) {
    if (g_registered_netdev == dev) {
        g_registered_netdev = NULL;
    }
}

void netif_start_queue(struct net_device *dev) {
    (void)dev;
}

void netif_stop_queue(struct net_device *dev) {
    (void)dev;
}

int netif_carrier_on(struct net_device *dev) {
    (void)dev;
    return 0;
}

int netif_carrier_off(struct net_device *dev) {
    (void)dev;
    return 0;
}

struct sk_buff *alloc_skb(unsigned int size, int priority) {
    (void)priority;
    struct sk_buff *skb = malloc(sizeof(struct sk_buff));
    if (!skb) return NULL;
    skb->data = malloc(size);
    if (!skb->data) {
        free(skb);
        return NULL;
    }
    skb->len = 0;
    skb->headroom = size;
    skb->tailroom = 0;
    skb->dev = NULL;
    return skb;
}

struct sk_buff *skb_clone(const struct sk_buff *skb, int priority) {
    (void)priority;
    if (!skb) return NULL;
    struct sk_buff *copy = malloc(sizeof(struct sk_buff));
    if (!copy) return NULL;
    copy->data = malloc(skb->len);
    if (!copy->data) {
        free(copy);
        return NULL;
    }
    memcpy(copy->data, skb->data, skb->len);
    copy->len = skb->len;
    copy->headroom = skb->headroom;
    copy->tailroom = skb->tailroom;
    copy->dev = skb->dev;
    return copy;
}

void kfree_skb(struct sk_buff *skb) {
    if (!skb) return;
    free(skb->data);
    free(skb);
}

int linux_compat_init(void) {
    /* No global state beyond driver registration required yet. */
    return 0;
}
