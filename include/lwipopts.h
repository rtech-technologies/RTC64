#ifndef LWIP_LWIPOPTS_H
#define LWIP_LWIPOPTS_H
#include <stdint.h>
typedef uint32_t sys_prot_t;
#define NO_SYS 1
#define LWIP_ARP 0
#define LWIP_ETHERNET 0
#define LWIP_IPV4 1
#define LWIP_TCP 0
#define LWIP_UDP 0
#define LWIP_STATS 0
#define LWIP_CHKSUM_ALGORITHM 2
#define LWIP_SINGLE_NETIF 1
#define LWIP_NETIF_HOSTNAME 0
#define LWIP_NETCONN 0
#define LWIP_SOCKET 0
#endif
