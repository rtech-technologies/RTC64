#ifndef LINUX_IO_H
#define LINUX_IO_H

#include <stdint.h>
#include "hal.h"

static inline void hal_outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" : : "a" (val), "Nd" (port));
}

static inline uint8_t hal_inb(uint16_t port) {
    uint8_t value;
    __asm__ volatile ("inb %1, %0" : "=a" (value) : "Nd" (port));
    return value;
}

static inline void hal_outw(uint16_t port, uint16_t val) {
    __asm__ volatile ("outw %0, %1" : : "a" (val), "Nd" (port));
}

static inline uint16_t hal_inw(uint16_t port) {
    uint16_t value;
    __asm__ volatile ("inw %1, %0" : "=a" (value) : "Nd" (port));
    return value;
}

static inline void hal_outl(uint16_t port, uint32_t val) {
    __asm__ volatile ("outl %0, %1" : : "a" (val), "Nd" (port));
}

static inline uint32_t hal_inl(uint16_t port) {
    uint32_t value;
    __asm__ volatile ("inl %1, %0" : "=a" (value) : "Nd" (port));
    return value;
}

#define outb(value, port) hal_outb((uint16_t)(port), (uint8_t)(value))
#define inb(port) hal_inb((uint16_t)(port))
#define outw(value, port) hal_outw((uint16_t)(port), (uint16_t)(value))
#define inw(port) hal_inw((uint16_t)(port))
#define outl(value, port) hal_outl((uint16_t)(port), (uint32_t)(value))
#define inl(port) hal_inl((uint16_t)(port))

#define writeb(value, addr) (*(volatile uint8_t *)(addr) = (value))
#define writew(value, addr) (*(volatile uint16_t *)(addr) = (value))
#define writel(value, addr) (*(volatile uint32_t *)(addr) = (value))
#define readb(addr) (*(const volatile uint8_t *)(addr))
#define readw(addr) (*(const volatile uint16_t *)(addr))
#define readl(addr) (*(const volatile uint32_t *)(addr))

#endif
