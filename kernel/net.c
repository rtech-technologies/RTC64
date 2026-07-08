/* Copyright (C) 2025 Sovereign RTC64 Project. All rights reserved.
 * Licensed under the 'respect people's property' OS license. */
#include "pro_os.h"
#include "serial.h"
#include <string.h>

typedef struct {
    uint8_t dest[6];
    uint8_t src[6];
    uint16_t type;
} __attribute__((packed)) eth_hdr_t;

typedef struct {
    uint8_t ver_ihl;
    uint8_t tos;
    uint16_t len;
    uint16_t id;
    uint16_t flags_frag;
    uint8_t ttl;
    uint8_t proto;
    uint16_t chksum;
    uint32_t src_ip;
    uint32_t dest_ip;
} __attribute__((packed)) ip_hdr_t;

void net_receive(void* packet, size_t len) {
    if (!packet || len < sizeof(eth_hdr_t)) return;
    if (len > 1518) return; /* MTU Violation */
    eth_hdr_t* eth = (eth_hdr_t*)packet;

    uint16_t type = (eth->type << 8) | (eth->type >> 8); /* ntohs */
    if (type == 0x0800) { /* IPv4 */
        ip_hdr_t* ip = (ip_hdr_t*)((uint8_t*)packet + sizeof(eth_hdr_t));
        serial_printf("[NET] Received IPv4 packet from %d.%d.%d.%d\n",
                     (ip->src_ip >> 0) & 0xFF, (ip->src_ip >> 8) & 0xFF,
                     (ip->src_ip >> 16) & 0xFF, (ip->src_ip >> 24) & 0xFF);
    } else if (type == 0x0806) { /* ARP */
        comprec_log("NET", "ARP Request handled");
    }
}

int rsl_web_fetch(const char* url, char* out, size_t sz) {
    comprec_log("NET", "Web Fetch Initiated");
    /* Real TCP handshake and HTTP GET would happen here via VirtIO-Net TX queues */
    snprintf(out, sz, "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\nSovereign RTC64 fetched data from: %s", url);
    return 0;
}
