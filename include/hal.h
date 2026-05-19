#ifndef HAL_H
#define HAL_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

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
    STORAGE_TYPE_SATAPI
} storage_type_t;

typedef struct storage_device storage_device_t;

struct storage_device {
    const char *name;
    storage_type_t type;
    uint64_t total_blocks;
    uint32_t block_size;
    int (*read)(storage_device_t* dev, uint64_t lba, void *buffer, uint32_t count);
    int (*write)(storage_device_t* dev, uint64_t lba, const void *buffer, uint32_t count);
};

void hal_storage_init(void);
int hal_storage_register_device(storage_device_t *dev);
int hal_storage_get_device_count(void);
storage_device_t* hal_storage_get_device(int index);

/* Sovereign Block Interface */
int hal_storage_read(storage_device_t* dev, uint64_t sector, void* buffer, uint32_t count);
int hal_storage_write(storage_device_t* dev, uint64_t sector, const void* buffer, uint32_t count);

/* --- USB System --- */
void hal_usb_init(void);
void hal_usb_poll(void);

#endif
