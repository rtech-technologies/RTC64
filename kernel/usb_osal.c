#include "usb_osal.h"
#include "hal.h"
#include "external/tlsf.h"

/* Sovereign OSAL Implementation for CherryUSB */

size_t usb_osal_enter_critical_section(void) { return 0; }
void usb_osal_leave_critical_section(size_t flag) { (void)flag; }

usb_osal_thread_t usb_osal_thread_create(const char *name, uint32_t stack_size, uint32_t priority, usb_thread_entry_t entry, void *argument) {
    (void)name; (void)stack_size; (void)priority; (void)entry; (void)argument;
    return (usb_osal_thread_t)1;
}

void usb_osal_thread_delete(usb_osal_thread_t thread) { (void)thread; }
void usb_osal_thread_schedule_other(void) { }

usb_osal_sem_t usb_osal_sem_create(uint32_t initial_count) { (void)initial_count; return (usb_osal_sem_t)1; }
usb_osal_sem_t usb_osal_sem_create_counting(uint32_t max_count) { (void)max_count; return (usb_osal_sem_t)1; }
void usb_osal_sem_delete(usb_osal_sem_t sem) { (void)sem; }
int usb_osal_sem_take(usb_osal_sem_t sem, uint32_t timeout) { (void)sem; (void)timeout; return 0; }
int usb_osal_sem_give(usb_osal_sem_t sem) { (void)sem; return 0; }
void usb_osal_sem_reset(usb_osal_sem_t sem) { (void)sem; }

usb_osal_mutex_t usb_osal_mutex_create(void) { return (usb_osal_mutex_t)1; }
void usb_osal_mutex_delete(usb_osal_mutex_t mutex) { (void)mutex; }
int usb_osal_mutex_take(usb_osal_mutex_t mutex) { (void)mutex; return 0; }
int usb_osal_mutex_give(usb_osal_mutex_t mutex) { (void)mutex; return 0; }

usb_osal_mq_t usb_osal_mq_create(uint32_t max_msgs) { (void)max_msgs; return (usb_osal_mq_t)1; }
void usb_osal_mq_delete(usb_osal_mq_t mq) { (void)mq; }
int usb_osal_mq_send(usb_osal_mq_t mq, uintptr_t addr) { (void)mq; (void)addr; return 0; }
int usb_osal_mq_recv(usb_osal_mq_t mq, uintptr_t *addr, uint32_t timeout) { (void)mq; (void)addr; (void)timeout; return 0; }

struct usb_osal_timer *usb_osal_timer_create(const char *name, uint32_t timeout_ms, usb_timer_handler_t handler, void *argument, bool is_period) {
    (void)name; (void)timeout_ms; (void)handler; (void)argument; (void)is_period;
    return (struct usb_osal_timer *)1;
}
void usb_osal_timer_delete(struct usb_osal_timer *timer) { (void)timer; }
void usb_osal_timer_start(struct usb_osal_timer *timer) { (void)timer; }
void usb_osal_timer_stop(struct usb_osal_timer *timer) { (void)timer; }

void usb_osal_msleep(uint32_t delay) { (void)delay; }

void *usb_osal_malloc(size_t size) { return tlsf_malloc(NULL, size); }
void usb_osal_free(void *ptr) { tlsf_free(NULL, ptr); }
