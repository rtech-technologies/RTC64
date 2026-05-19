#include <pro_os.h>

uint32_t sys_now(void) { return 0; }
void sys_arch_protect(void) {}
void sys_arch_unprotect(uint32_t p) { (void)p; }
void sys_timeouts_init(void) {}

void ethernet_input(void* p, void* n) { (void)p; (void)n; }
void* ethbroadcast = (void*)0x1234;
void* ethzero = (void*)0x5678;

uint32_t lwip_htonl(uint32_t x) { return ((((x) & 0x000000ff) << 24) | (((x) & 0x0000ff00) << 8) | (((x) & 0x00ff0000) >> 8) | (((x) & 0xff000000) >> 24)); }
uint16_t lwip_htons(uint16_t x) { return ((((x) & 0x00ff) << 8) | (((x) & 0xff00) >> 8)); }

uint16_t ip_chksum_pseudo(void* p, void* proto, void* proto2, void* src, void* dest) { (void)p; (void)proto; (void)proto2; (void)src; (void)dest; return 0; }
uint16_t inet_chksum_pbuf(void* p) { (void)p; return 0; }
uint16_t inet_chksum(void* p, uint16_t l) { (void)p; (void)l; return 0; }

void ip4_reass(void) {}
void icmp_input(void) {}
void icmp_dest_unreach(void) {}
void ip4_frag(void) {}

char* lwip_itoa(char* b, int l, int v) { (void)b; (void)l; (void)v; return b; }
