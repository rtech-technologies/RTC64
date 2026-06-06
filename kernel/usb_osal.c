#include "usb_osal.h"
#include "hal.h"
#include "external/tlsf.h"
#include <string.h>

/* Sovereign OSAL Implementation for CherryUSB
 * Baremetal kernel - no real threading, just stub implementations
 * that allow CherryUSB to initialize without errors */

extern void* tlsf_get_global(void);

/* Simple semaphore structure for basic synchronization */
typedef struct {
    uint32_t count;
    uint32_t max_count;
} usb_sem_t;

/* Simple message queue structure */
typedef struct {
    uintptr_t *messages;
    uint32_t max_msgs;
    uint32_t head;
    uint32_t tail;
} usb_mq_t;

size_t usb_osal_enter_critical_section(void) { 
    return 0; 
}

void usb_osal_leave_critical_section(size_t flag) { 
    (void)flag; 
}

usb_osal_thread_t usb_osal_thread_create(const char *name, uint32_t stack_size, uint32_t priority, usb_thread_entry_t entry, void *argument) {
    (void)name; (void)stack_size; (void)priority; 
    /* In a baremetal kernel, we can't create real threads. Just return a dummy handle. */
    /* In a real system, this would create a thread and call entry(argument) */
    if (entry) {
        entry(argument);
    }
    return (usb_osal_thread_t)1;
}

void usb_osal_thread_delete(usb_osal_thread_t thread) { (void)thread; }
void usb_osal_thread_schedule_other(void) { }

usb_osal_sem_t usb_osal_sem_create(uint32_t initial_count) { 
    usb_sem_t *sem = (usb_sem_t *)usb_osal_malloc(sizeof(usb_sem_t));
    if (sem) {
        sem->count = initial_count;
        sem->max_count = 1;
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
    (void)timeout;
    usb_sem_t *s = (usb_sem_t *)sem;
    if (s && s->count > 0) {
        s->count--;
        return 0;
    }
    return -1;
}

int usb_osal_sem_give(usb_osal_sem_t sem) { 
    usb_sem_t *s = (usb_sem_t *)sem;
    if (s && s->count < s->max_count) {
        s->count++;
        return 0;
    }
    return -1;
}

void usb_osal_sem_reset(usb_osal_sem_t sem) { 
    usb_sem_t *s = (usb_sem_t *)sem;
    if (s) s->count = 0;
}

usb_osal_mutex_t usb_osal_mutex_create(void) { 
    usb_sem_t *mutex = (usb_sem_t *)usb_osal_malloc(sizeof(usb_sem_t));
    if (mutex) {
        mutex->count = 1;
        mutex->max_count = 1;
    }
    return (usb_osal_mutex_t)mutex;
}

void usb_osal_mutex_delete(usb_osal_mutex_t mutex) { 
    usb_osal_free(mutex);
}

int usb_osal_mutex_take(usb_osal_mutex_t mutex) { 
    usb_sem_t *m = (usb_sem_t *)mutex;
    if (m && m->count > 0) {
        m->count--;
        return 0;
    }
    return -1;
}

int usb_osal_mutex_give(usb_osal_mutex_t mutex) { 
    usb_sem_t *m = (usb_sem_t *)mutex;
    if (m && m->count == 0) {
        m->count = 1;
        return 0;
    }
    return -1;
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
    
    uint32_t next_head = (m->head + 1) % m->max_msgs;
    if (next_head == m->tail) {
        return -1; /* Queue full */
    }
    
    m->messages[m->head] = addr;
    m->head = next_head;
    return 0;
}

int usb_osal_mq_recv(usb_osal_mq_t mq, uintptr_t *addr, uint32_t timeout) { 
    (void)timeout;
    usb_mq_t *m = (usb_mq_t *)mq;
    if (!m || !addr) return -1;
    
    if (m->head == m->tail) {
        return -1; /* Queue empty */
    }
    
    *addr = m->messages[m->tail];
    m->tail = (m->tail + 1) % m->max_msgs;
    return 0;
}

struct usb_osal_timer *usb_osal_timer_create(const char *name, uint32_t timeout_ms, usb_timer_handler_t handler, void *argument, bool is_period) {
    (void)name; (void)timeout_ms; (void)is_period;
    struct usb_osal_timer *timer = (struct usb_osal_timer *)usb_osal_malloc(sizeof(struct usb_osal_timer));
    if (timer) {
        memset(timer, 0, sizeof(struct usb_osal_timer));
        timer->handler = handler;
        timer->argument = argument;
        timer->timeout_ms = timeout_ms;
        timer->is_period = is_period;
    }
    return timer;
}

void usb_osal_timer_delete(struct usb_osal_timer *timer) { 
    usb_osal_free(timer);
}

void usb_osal_timer_start(struct usb_osal_timer *timer) { 
    (void)timer;
    /* In a baremetal system, we'd need an actual timer interrupt handler */
}

void usb_osal_timer_stop(struct usb_osal_timer *timer) { 
    (void)timer;
}

void usb_osal_msleep(uint32_t delay) { 
    (void)delay;
    /* Without a real timer, we just return immediately */
}

void *usb_osal_malloc(size_t size) { 
    return tlsf_malloc(tlsf_get_global(), size); 
}

void usb_osal_free(void *ptr) { 
    tlsf_free(tlsf_get_global(), ptr); 
}
