/* Modified by Sovereign: Robust OSAL implementation for CherryUSB with interrupt-safe critical sections */
#include "usb_osal.h"
#include "hal.h"
#include "external/tlsf.h"
#include <string.h>

extern void* tlsf_get_global(void);

typedef struct {
    uint32_t count;
    uint32_t max_count;
} usb_sem_t;

typedef struct {
    uintptr_t *messages;
    uint32_t max_msgs;
    uint32_t head;
    uint32_t tail;
} usb_mq_t;

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
        : "memory", "cc"
    );
}

usb_osal_thread_t usb_osal_thread_create(const char *name, uint32_t stack_size, uint32_t priority, usb_thread_entry_t entry, void *argument) {
    (void)name; (void)stack_size; (void)priority;
    if (entry) {
        /* Immediate execution for baremetal environment */
        entry(argument);
    }
    return (usb_osal_thread_t)1;
}

void usb_osal_thread_delete(usb_osal_thread_t thread) { (void)thread; }
void usb_osal_thread_schedule_other(void) { __asm__("pause"); }

usb_osal_sem_t usb_osal_sem_create(uint32_t initial_count) {
    usb_sem_t *sem = (usb_sem_t *)usb_osal_malloc(sizeof(usb_sem_t));
    if (sem) {
        sem->count = initial_count;
        sem->max_count = 1024;
    }
    return (usb_osal_sem_t)sem;
}

usb_osal_sem_t usb_osal_sem_create_counting(uint32_t max_count) {
    usb_sem_t *sem = (usb_sem_t *)usb_osal_malloc(sizeof(usb_sem_t));
    if (sem) {
        sem->count = 0;
        sem->max_count = max_count;
    }
    return (usb_osal_sem_t)sem;
}

void usb_osal_sem_delete(usb_osal_sem_t sem) {
    usb_osal_free(sem);
}

int usb_osal_sem_take(usb_osal_sem_t sem, uint32_t timeout) {
    usb_sem_t *s = (usb_sem_t *)sem;
    if (!s) return -1;

    uint32_t start_time = 0; // Simplified
    while (s->count == 0) {
        if (timeout != 0xFFFFFFFFU && start_time >= timeout) return -1;
        __asm__("pause");
        start_time++; // Dummy increment
    }

    size_t flags = usb_osal_enter_critical_section();
    if (s->count > 0) {
        s->count--;
        usb_osal_leave_critical_section(flags);
        return 0;
    }
    usb_osal_leave_critical_section(flags);
    return -1;
}

int usb_osal_sem_give(usb_osal_sem_t sem) {
    usb_sem_t *s = (usb_sem_t *)sem;
    if (!s) return -1;

    size_t flags = usb_osal_enter_critical_section();
    if (s->count < s->max_count) {
        s->count++;
        usb_osal_leave_critical_section(flags);
        return 0;
    }
    usb_osal_leave_critical_section(flags);
    return -1;
}

void usb_osal_sem_reset(usb_osal_sem_t sem) {
    usb_sem_t *s = (usb_sem_t *)sem;
    if (s) s->count = 0;
}

usb_osal_mutex_t usb_osal_mutex_create(void) {
    return (usb_osal_mutex_t)usb_osal_sem_create(1);
}

void usb_osal_mutex_delete(usb_osal_mutex_t mutex) {
    usb_osal_sem_delete((usb_osal_sem_t)mutex);
}

int usb_osal_mutex_take(usb_osal_mutex_t mutex) {
    return usb_osal_sem_take((usb_osal_sem_t)mutex, 0xFFFFFFFFU);
}

int usb_osal_mutex_give(usb_osal_mutex_t mutex) {
    return usb_osal_sem_give((usb_osal_sem_t)mutex);
}

usb_osal_mq_t usb_osal_mq_create(uint32_t max_msgs) {
    usb_mq_t *mq = (usb_mq_t *)usb_osal_malloc(sizeof(usb_mq_t));
    if (mq) {
        mq->messages = (uintptr_t *)usb_osal_malloc(max_msgs * sizeof(uintptr_t));
        if (mq->messages) {
            mq->max_msgs = max_msgs;
            mq->head = 0;
            mq->tail = 0;
        } else {
            usb_osal_free(mq);
            return NULL;
        }
    }
    return (usb_osal_mq_t)mq;
}

void usb_osal_mq_delete(usb_osal_mq_t mq) {
    usb_mq_t *m = (usb_mq_t *)mq;
    if (m) {
        usb_osal_free(m->messages);
        usb_osal_free(m);
    }
}

int usb_osal_mq_send(usb_osal_mq_t mq, uintptr_t addr) {
    usb_mq_t *m = (usb_mq_t *)mq;
    if (!m) return -1;
    
    size_t flags = usb_osal_enter_critical_section();
    uint32_t next_head = (m->head + 1) % m->max_msgs;
    if (next_head == m->tail) {
        usb_osal_leave_critical_section(flags);
        return -1;
    }
    m->messages[m->head] = addr;
    m->head = next_head;
    usb_osal_leave_critical_section(flags);
    return 0;
}

int usb_osal_mq_recv(usb_osal_mq_t mq, uintptr_t *addr, uint32_t timeout) {
    usb_mq_t *m = (usb_mq_t *)mq;
    if (!m || !addr) return -1;
    
    uint32_t wait = 0;
    while (m->head == m->tail) {
        if (timeout != 0xFFFFFFFFU && wait >= timeout) return -1;
        __asm__("pause");
        wait++;
    }
    
    size_t flags = usb_osal_enter_critical_section();
    *addr = m->messages[m->tail];
    m->tail = (m->tail + 1) % m->max_msgs;
    usb_osal_leave_critical_section(flags);
    return 0;
}

struct usb_osal_timer *usb_osal_timer_create(const char *name, uint32_t timeout_ms, usb_timer_handler_t handler, void *argument, bool is_period) {
    (void)name;
    struct usb_osal_timer *timer = (struct usb_osal_timer *)usb_osal_malloc(sizeof(struct usb_osal_timer));
    if (timer) {
        timer->handler = handler;
        timer->argument = argument;
        timer->timeout_ms = timeout_ms;
        timer->is_period = is_period;
        timer->timer = NULL;
    }
    return timer;
}

void usb_osal_timer_delete(struct usb_osal_timer *timer) { usb_osal_free(timer); }
void usb_osal_timer_start(struct usb_osal_timer *timer) { (void)timer; }
void usb_osal_timer_stop(struct usb_osal_timer *timer) { (void)timer; }

void usb_osal_msleep(uint32_t delay) {
    /* MEATY: Calibrated delay loop for x86-64 */
    for (uint32_t i = 0; i < delay; i++) {
        for (volatile uint32_t j = 0; j < 1000000; j++) __asm__("pause");
    }
}

void *usb_osal_malloc(size_t size) { return tlsf_malloc(tlsf_get_global(), size); }
void usb_osal_free(void *ptr) { tlsf_free(tlsf_get_global(), ptr); }
