#include "pro_os.h"
#include <string.h>

/*
 * Sovereign Linux Network Driver Compatibility Shim (netdev shim)
 * Provides emulation for standard Linux kernel driver structs:
 * sk_buff, net_device, net_device_ops, and ethernet device allocators
 */

extern void* malloc(size_t size);
extern void free(void *ptr);

struct sk_buff* dev_alloc_skb(unsigned int length) {
    struct sk_buff *skb = (struct sk_buff*)malloc(sizeof(struct sk_buff));
    if (skb) {
        skb->data = (uint8_t*)malloc(length);
        skb->len = length;
    }
    return skb;
}

void dev_kfree_skb(struct sk_buff *skb) {
    if (skb) {
        if (skb->data) free(skb->data);
        free(skb);
    }
}

struct net_device* alloc_etherdev(int sizeof_priv) {
    struct net_device *dev = (struct net_device*)malloc(sizeof(struct net_device));
    if (dev) {
        memset(dev, 0, sizeof(struct net_device));
        strcpy(dev->name, "eth0");
        if (sizeof_priv > 0) {
            dev->priv = malloc(sizeof_priv);
            memset(dev->priv, 0, sizeof_priv);
        }
    }
    return dev;
}

void free_netdev(struct net_device *dev) {
    if (dev) {
        if (dev->priv) free(dev->priv);
        free(dev);
    }
}

static struct net_device *active_linux_netdev = NULL;

int register_netdev(struct net_device *dev) {
    active_linux_netdev = dev;
    if (dev->netdev_ops && dev->netdev_ops->ndo_open) {
        dev->netdev_ops->ndo_open(dev);
    }
    return 0; /* SUCCESS */
}

void unregister_netdev(struct net_device *dev) {
    if (dev->netdev_ops && dev->netdev_ops->ndo_stop) {
        dev->netdev_ops->ndo_stop(dev);
    }
    if (active_linux_netdev == dev) {
        active_linux_netdev = NULL;
    }
}

/* Simulated Linux VirtIO-PCI network card driver hooked to shim */

static int virtio_net_open(struct net_device *dev) {
    (void)dev;
    return 0;
}

static int virtio_net_stop(struct net_device *dev) {
    (void)dev;
    return 0;
}

static int virtio_net_start_xmit(struct sk_buff *skb, struct net_device *dev) {
    (void)dev;
    /* Transmit the packet via virtual hardware ports/MMIO */
    dev_kfree_skb(skb);
    return 0; /* NETDEV_TX_OK */
}

static const struct net_device_ops virtio_net_device_ops = {
    .ndo_open = virtio_net_open,
    .ndo_stop = virtio_net_stop,
    .ndo_start_xmit = virtio_net_start_xmit
};

void hal_virtio_net_probe(uint64_t mmio) {
    (void)mmio;
    /* Create a Linux-style netdev, attach ops, and register it! */
    struct net_device *dev = alloc_etherdev(256); // 256 bytes private driver data
    if (dev) {
        dev->netdev_ops = &virtio_net_device_ops;
        dev->dev_addr[0] = 0x52;
        dev->dev_addr[1] = 0x54;
        dev->dev_addr[2] = 0x00;
        dev->dev_addr[3] = 0x12;
        dev->dev_addr[4] = 0x34;
        dev->dev_addr[5] = 0x56;

        register_netdev(dev);
    }
}

struct net_device* linux_shim_get_active_device(void) {
    return active_linux_netdev;
}
