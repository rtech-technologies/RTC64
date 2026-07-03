#ifndef LINUX_ETHERDEVICE_H
#define LINUX_ETHERDEVICE_H

#include <stdint.h>
#include "linux/netdevice.h"

static inline bool is_broadcast_ether_addr(const uint8_t *addr) {
    return addr[0] == 0xff && addr[1] == 0xff && addr[2] == 0xff &&
           addr[3] == 0xff && addr[4] == 0xff && addr[5] == 0xff;
}

static inline bool is_zero_ether_addr(const uint8_t *addr) {
    return addr[0] == 0 && addr[1] == 0 && addr[2] == 0 &&
           addr[3] == 0 && addr[4] == 0 && addr[5] == 0;
}

#endif
