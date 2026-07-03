#ifndef LINUX_NETDEVICE_H
#define LINUX_NETDEVICE_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define ETH_ALEN 6
#define IFNAMSIZ 16

struct net_device_stats {
    unsigned long rx_packets;
    unsigned long tx_packets;
    unsigned long rx_bytes;
    unsigned long tx_bytes;
    unsigned long rx_errors;
    unsigned long tx_errors;
    unsigned long rx_dropped;
    unsigned long tx_dropped;
    unsigned long multicast;
    unsigned long collisions;
};

struct net_device_ops {
    int (*ndo_open)(struct net_device *dev);
    int (*ndo_stop)(struct net_device *dev);
    int (*ndo_start_xmit)(struct sk_buff *skb, struct net_device *dev);
    void (*ndo_set_rx_mode)(struct net_device *dev);
};

struct net_device {
    char name[IFNAMSIZ];
    uint8_t dev_addr[ETH_ALEN];
    unsigned int mtu;
    unsigned int flags;
    struct net_device_ops *netdev_ops;
    struct net_device_stats stats;
    void *ml_priv;
};

struct net_device *alloc_netdev(int sizeof_priv, const char *name, void (*setup)(struct net_device *));
void free_netdev(struct net_device *dev);
int register_netdev(struct net_device *dev);
void unregister_netdev(struct net_device *dev);
void netif_start_queue(struct net_device *dev);
void netif_stop_queue(struct net_device *dev);
int netif_carrier_on(struct net_device *dev);
int netif_carrier_off(struct net_device *dev);

#endif
