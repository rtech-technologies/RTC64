#ifndef HAL_H
#define HAL_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

/* --- I/O Port Primitives --- */
static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ( "outb %0, %1" : : "a"(val), "Nd"(port) );
}

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ( "inb %1, %0" : "=a"(ret) : "Nd"(port) );
    return ret;
}

static inline void outl(uint16_t port, uint32_t val) {
    __asm__ volatile ( "outl %0, %1" : : "a"(val), "Nd"(port) );
}

static inline uint32_t inl(uint16_t port) {
    uint32_t ret;
    __asm__ volatile ( "inl %1, %0" : "=a"(ret) : "Nd"(port) );
    return ret;
}

/* --- Input System --- */
typedef enum {
    INPUT_TYPE_KEYBOARD,
    INPUT_TYPE_MOUSE
} input_type_t;

typedef struct {
    input_type_t type;
    union {
        struct {
            uint32_t key;
            bool down;
        } kbd;
        struct {
            int x, y;
            int scroll;
            uint32_t buttons;
        } mouse;
    };
} input_event_t;

void hal_input_init(void);
void hal_input_poll(void);
void hal_input_push_event(input_event_t ev);
bool hal_input_pop_event(input_event_t *ev);

/* --- Storage System --- */
typedef enum {
    STORAGE_TYPE_USB,
    STORAGE_TYPE_NVME,
    STORAGE_TYPE_SATA,
    STORAGE_TYPE_SATAPI,
    STORAGE_TYPE_RAMDISK
} storage_type_t;

typedef struct storage_device_s storage_device_t;

struct storage_device_s {
    const char *name;
    storage_type_t type;
    uint64_t total_blocks;
    uint32_t block_size;
    void *priv;
    int (*read)(storage_device_t *dev, uint64_t lba, void *buffer, uint32_t count);
    int (*write)(storage_device_t *dev, uint64_t lba, const void *buffer, uint32_t count);
};

void hal_storage_init(void);
void hal_storage_finish_init(void);
int hal_storage_register_device(storage_device_t *dev);
int hal_storage_get_device_count(void);
storage_device_t* hal_storage_get_device(int index);

int hal_storage_read(storage_device_t* dev, uint64_t sector, void* buffer, uint32_t count);
int hal_storage_write(storage_device_t* dev, uint64_t sector, const void* buffer, uint32_t count);

int hal_nvme_init(uint64_t mmio);
int hal_sata_init(uint64_t mmio);
int ramdisk_init(void);

/* --- USB System --- */
void hal_usb_init(void);
void hal_usb_poll(void);

/* --- RTC System --- */
void rtc_get_time(int *h, int *m, int *s);
uint64_t hal_get_uptime_ms(void);

void apic_eoi(void);

#endif
