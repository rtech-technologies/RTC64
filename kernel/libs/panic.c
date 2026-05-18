#include <pro_os.h>

#define COM1 0x3F8

static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ( "outb %0, %1" : : "a"(val), "Nd"(port) );
}

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ( "inb %1, %0" : "=a"(ret) : "Nd"(port) );
    return ret;
}

#define XHCI_USBCMD 0x00
#define XHCI_USBSTS 0x04

void quartermaster_panic(const char* msg) {
    // 1. Paint Emerald Green
    // In production we map fb->address, but for forensic mirror we use serial

    // 2. Dump RAX-R15 (captured via assembly stub in real scenario)

    // 3. Mandatory register dump scan of physical XHCI state lines
    // Assuming xhci_base is known
    // uint32_t cmd = *(volatile uint32_t*)(xhci_base + XHCI_USBCMD);
    // uint32_t sts = *(volatile uint32_t*)(xhci_base + XHCI_USBSTS);

    const char* panic_msg = "\n!!! KERNEL PANIC: QUARTERMASTER LOCKDOWN !!!\n";
    const char* p = panic_msg;
    while(*p) {
        while ((inb(COM1 + 5) & 0x20) == 0);
        outb(COM1, *p++);
    }

    p = msg;
    while(*p) {
        while ((inb(COM1 + 5) & 0x20) == 0);
        outb(COM1, *p++);
    }

    for(;;) __asm__("hlt");
}
