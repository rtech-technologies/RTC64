#ifndef USB_OSAL_H
#define USB_OSAL_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef void (*usb_thread_entry_t)(void *argument);
typedef void (*usb_timer_handler_t)(void *argument);

typedef void *usb_osal_thread_t;
typedef void *usb_osal_sem_t;
typedef void *usb_osal_mutex_t;
typedef void *usb_osal_mq_t;

struct usb_osal_timer {
    void *handler;
    void *argument;
    uint32_t timeout_ms;
    bool is_period;
};

/* OSAL Prototypes */
size_t usb_osal_enter_critical_section(void);
void usb_osal_leave_critical_section(size_t flag);

usb_osal_thread_t usb_osal_thread_create(const char *name, uint32_t stack_size, uint32_t priority, usb_thread_entry_t entry, void *argument);
void usb_osal_thread_delete(usb_osal_thread_t thread);
void usb_osal_thread_schedule_other(void);

usb_osal_sem_t usb_osal_sem_create(uint32_t initial_count);
usb_osal_sem_t usb_osal_sem_create_counting(uint32_t max_count);
void usb_osal_sem_delete(usb_osal_sem_t sem);
int usb_osal_sem_take(usb_osal_sem_t sem, uint32_t timeout);
int usb_osal_sem_give(usb_osal_sem_t sem);
void usb_osal_sem_reset(usb_osal_sem_t sem);

usb_osal_mutex_t usb_osal_mutex_create(void);
void usb_osal_mutex_delete(usb_osal_mutex_t mutex);
int usb_osal_mutex_take(usb_osal_mutex_t mutex);
int usb_osal_mutex_give(usb_osal_mutex_t mutex);

usb_osal_mq_t usb_osal_mq_create(uint32_t max_msgs);
void usb_osal_mq_delete(usb_osal_mq_t mq);
int usb_osal_mq_send(usb_osal_mq_t mq, uintptr_t addr);
int usb_osal_mq_recv(usb_osal_mq_t mq, uintptr_t *addr, uint32_t timeout);

struct usb_osal_timer *usb_osal_timer_create(const char *name, uint32_t timeout_ms, usb_timer_handler_t handler, void *argument, bool is_period);
void usb_osal_timer_delete(struct usb_osal_timer *timer);
void usb_osal_timer_start(struct usb_osal_timer *timer);
void usb_osal_timer_stop(struct usb_osal_timer *timer);

void usb_osal_msleep(uint32_t delay);

void *usb_osal_malloc(size_t size);
void usb_osal_free(void *ptr);

#endif
