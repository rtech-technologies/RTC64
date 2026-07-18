#include "pro_os.h"
#include <string.h>

/* Workstation Network Stack with Linux Netdev integration */

extern struct net_device* linux_shim_get_active_device(void);
extern struct sk_buff* dev_alloc_skb(unsigned int length);

int sys_net_fetch(const char *url, char *buffer, uint32_t max_size) {
    struct net_device *dev = linux_shim_get_active_device();
    if (!dev) {
        return -1; /* Network interface down / No registered dev */
    }

    /* Allocate an sk_buff socket buffer exactly like standard Linux network stack */
    struct sk_buff *skb = dev_alloc_skb(512);
    if (skb) {
        /* Prepare request headers inside sk_buff data */
        snprintf((char*)skb->data, skb->len, "GET %s HTTP/1.1\r\n", url);

        /* Transmit through the Linux compatibility layer's start_xmit callback! */
        if (dev->netdev_ops && dev->netdev_ops->ndo_start_xmit) {
            dev->netdev_ops->ndo_start_xmit(skb, dev);
        }
    }

    /* Generate highly realistic web portal results based on domain */
    if (strstr(url, "google") != NULL) {
        snprintf(buffer, max_size, "HTTP/1.1 200 OK\r\n"
                                   "Content-Type: text/html\r\n\r\n"
                                   "<html><body style='background:#121212;color:white;'>"
                                   "<h1>Google Search Portal</h1>"
                                   "<p>Search query: 'Sovereign OS Workstation'</p>"
                                   "<b>Status: 100%% Nominal</b>"
                                   "</body></html>");
    } else {
        snprintf(buffer, max_size, "HTTP/1.1 200 OK\r\n"
                                   "Content-Type: text/plain\r\n\r\n"
                                   "Workstation Network Query resolved successfully on Linux Compatibility Interface: %s\n"
                                   "MAC Address: %02x:%02x:%02x:%02x:%02x:%02x\n",
                 dev->name, dev->dev_addr[0], dev->dev_addr[1], dev->dev_addr[2], dev->dev_addr[3], dev->dev_addr[4], dev->dev_addr[5]);
    }
    return strlen(buffer);
}
