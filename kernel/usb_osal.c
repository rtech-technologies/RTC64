#include "usb_osal.h"
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

/* Simple memory management stubs for kernel mode */
extern void *malloc(size_t size);
extern void free(void *ptr);

void *usb_osal_malloc(size_t size) { return malloc(size); }
void usb_osal_free(void *ptr) { free(ptr); }

/* Threading - Single threaded polling mode for simplicity in this DE */
usb_osal_thread_t usb_osal_thread_create(const char *name, uint32_t stack_size, uint32_t prio, usb_thread_entry_t entry, void *args) {
    (void)name; (void)stack_size; (void)prio; (void)entry; (void)args;
    return (usb_osal_thread_t)1;
}

void usb_osal_thread_delete(usb_osal_thread_t thread) { (void)thread; }
void usb_osal_thread_schedule_other(void) {}

/* Semaphores */
struct usb_osal_sem {
    volatile uint32_t count;
};

usb_osal_sem_t usb_osal_sem_create(uint32_t initial_count) {
    struct usb_osal_sem *sem = usb_osal_malloc(sizeof(struct usb_osal_sem));
    sem->count = initial_count;
    return (usb_osal_sem_t)sem;
}

void usb_osal_sem_delete(usb_osal_sem_t sem) { usb_osal_free(sem); }

int usb_osal_sem_take(usb_osal_sem_t sem, uint32_t timeout) {
    struct usb_osal_sem *s = (struct usb_osal_sem *)sem;
    if (s->count > 0) {
        s->count--;
        return 0;
    }
    // In a real kernel we would sleep, here we just return timeout if not available immediately
    // or we poll in a loop if timeout is large.
    return -1;
}

int usb_osal_sem_give(usb_osal_sem_t sem) {
    struct usb_osal_sem *s = (struct usb_osal_sem *)sem;
    s->count++;
    return 0;
}

/* Mutexes */
usb_osal_mutex_t usb_osal_mutex_create(void) { return (usb_osal_mutex_t)usb_osal_sem_create(1); }
void usb_osal_mutex_delete(usb_osal_mutex_t mutex) { usb_osal_sem_delete((usb_osal_sem_t)mutex); }
int usb_osal_mutex_take(usb_osal_mutex_t mutex) { return usb_osal_sem_take((usb_osal_sem_t)mutex, 0xFFFFFFFF); }
int usb_osal_mutex_give(usb_osal_mutex_t mutex) { return usb_osal_sem_give((usb_osal_sem_t)mutex); }

/* Delays */
void usb_osal_msleep(uint32_t delay) {
    for (volatile uint32_t i = 0; i < delay * 10000; i++);
}

size_t usb_osal_enter_critical_section(void) { return 0; }
void usb_osal_leave_critical_section(size_t flag) { (void)flag; }

/* Empty implementations for remaining OSAL features to satisfy linker */
usb_osal_sem_t usb_osal_sem_create_counting(uint32_t max_count) { return usb_osal_sem_create(0); }
void usb_osal_sem_reset(usb_osal_sem_t sem) { ((struct usb_osal_sem*)sem)->count = 0; }
usb_osal_mq_t usb_osal_mq_create(uint32_t max_msgs) { (void)max_msgs; return NULL; }
void usb_osal_mq_delete(usb_osal_mq_t mq) { (void)mq; }
int usb_osal_mq_send(usb_osal_mq_t mq, uintptr_t addr) { (void)mq; (void)addr; return 0; }
int usb_osal_mq_recv(usb_osal_mq_t mq, uintptr_t *addr, uint32_t timeout) { (void)mq; (void)addr; (void)timeout; return 0; }
struct usb_osal_timer *usb_osal_timer_create(const char *name, uint32_t timeout_ms, usb_timer_handler_t handler, void *argument, bool is_period) { return NULL; }
void usb_osal_timer_delete(struct usb_osal_timer *timer) { (void)timer; }
void usb_osal_timer_start(struct usb_osal_timer *timer) { (void)timer; }
void usb_osal_timer_stop(struct usb_osal_timer *timer) { (void)timer; }
