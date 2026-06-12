#include "usb_osal.h"
#include "hal.h"
#include "pro_os.h"
#include "external/tlsf.h"
#include <string.h>

/* Sovereign OSAL Implementation for CherryUSB */

extern void* tlsf_get_global(void);

size_t usb_osal_enter_critical_section(void) {
    size_t flags;
    __asm__ volatile(
        "pushfq\n\t"
        "pop %0\n\t"
        "cli"
        : "=rm"(flags)
        :
        : "memory"
    );
    return flags;
}

void usb_osal_leave_critical_section(size_t flag) {
    __asm__ volatile(
        "push %0\n\t"
        "popfq"
        :
        : "rm"(flag)
        : "memory"
    );
}

usb_osal_thread_t usb_osal_thread_create(const char *name, uint32_t stack_size, uint32_t priority, usb_thread_entry_t entry, void *argument) {
    (void)stack_size; (void)priority; (void)argument;
    // Sovereign's scheduler uses a simple entry function pointer.
    // In a meatier implementation, we would use a wrapper to pass 'argument'.
    scheduler_add_task(name, (void (*)(void))entry);
    return (usb_osal_thread_t)1;
}

void usb_osal_thread_delete(usb_osal_thread_t thread) { (void)thread; }
void usb_osal_thread_schedule_other(void) { }

/* Semaphores */
struct usb_osal_sem {
    volatile uint32_t count;
};

usb_osal_sem_t usb_osal_sem_create(uint32_t initial_count) {
    struct usb_osal_sem *sem = (struct usb_osal_sem *)usb_osal_malloc(sizeof(struct usb_osal_sem));
    if (sem) sem->count = initial_count;
    return (usb_osal_sem_t)sem;
}

usb_osal_sem_t usb_osal_sem_create_counting(uint32_t max_count) {
    (void)max_count;
    return usb_osal_sem_create(0);
}

void usb_osal_sem_delete(usb_osal_sem_t sem) {
    if (sem) usb_osal_free(sem);
}

int usb_osal_sem_take(usb_osal_sem_t sem, uint32_t timeout) {
    if (!sem) return -1;
    struct usb_osal_sem *s = (struct usb_osal_sem *)sem;
    uint32_t start = 0;
    while (s->count == 0) {
        if (timeout != 0xFFFFFFFF) {
            if (start++ > timeout * 1000) return -1;
        }
        __asm__ volatile("pause");
    }
    size_t flags = usb_osal_enter_critical_section();
    if (s->count > 0) s->count--;
    usb_osal_leave_critical_section(flags);
    return 0;
}

int usb_osal_sem_give(usb_osal_sem_t sem) {
    if (!sem) return -1;
    struct usb_osal_sem *s = (struct usb_osal_sem *)sem;
    size_t flags = usb_osal_enter_critical_section();
    s->count++;
    usb_osal_leave_critical_section(flags);
    return 0;
}

void usb_osal_sem_reset(usb_osal_sem_t sem) {
    if (sem) ((struct usb_osal_sem *)sem)->count = 0;
}

/* Mutexes */
usb_osal_mutex_t usb_osal_mutex_create(void) {
    return (usb_osal_mutex_t)usb_osal_sem_create(1);
}

void usb_osal_mutex_delete(usb_osal_mutex_t mutex) {
    usb_osal_sem_delete((usb_osal_sem_t)mutex);
}

int usb_osal_mutex_take(usb_osal_mutex_t mutex) {
    return usb_osal_sem_take((usb_osal_sem_t)mutex, 0xFFFFFFFF);
}

int usb_osal_mutex_give(usb_osal_mutex_t mutex) {
    return usb_osal_sem_give((usb_osal_sem_t)mutex);
}

/* Message Queues */
usb_osal_mq_t usb_osal_mq_create(uint32_t max_msgs) { (void)max_msgs; return (usb_osal_mq_t)1; }
void usb_osal_mq_delete(usb_osal_mq_t mq) { (void)mq; }
int usb_osal_mq_send(usb_osal_mq_t mq, uintptr_t addr) { (void)mq; (void)addr; return 0; }
int usb_osal_mq_recv(usb_osal_mq_t mq, uintptr_t *addr, uint32_t timeout) { (void)mq; (void)addr; (void)timeout; return 0; }

struct usb_osal_timer *usb_osal_timer_create(const char *name, uint32_t timeout_ms, usb_timer_handler_t handler, void *argument, bool is_period) {
    (void)name;
    struct usb_osal_timer *timer = (struct usb_osal_timer *)usb_osal_malloc(sizeof(struct usb_osal_timer));
    if (timer) {
        timer->timeout_ms = timeout_ms;
        timer->handler = handler;
        timer->argument = argument;
        timer->is_period = is_period;
        timer->timer = NULL;
    }
    return timer;
}
void usb_osal_timer_delete(struct usb_osal_timer *timer) { if (timer) usb_osal_free(timer); }
void usb_osal_timer_start(struct usb_osal_timer *timer) { (void)timer; }
void usb_osal_timer_stop(struct usb_osal_timer *timer) { (void)timer; }

void usb_osal_msleep(uint32_t delay) {
    for (volatile uint32_t i = 0; i < delay * 10000; i++) {
        __asm__ volatile("pause");
    }
}

void *usb_osal_malloc(size_t size) { return tlsf_malloc(tlsf_get_global(), size); }
void usb_osal_free(void *ptr) { tlsf_free(tlsf_get_global(), ptr); }
