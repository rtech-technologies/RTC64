/* Modified by Sovereign: Robust OSAL implementation for CherryUSB with interrupt-safe critical sections */
#include "pro_os.h"
#include "usb_osal.h"
#include "hal.h"
#include "external/tlsf.h"
#include <string.h>
#include "serial.h"

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
    (void)stack_size; (void)priority;
    serial_printf("[USB OSAL] Creating thread: %s\n", name);
    if (entry) {
        /* MEATY: Registering with kernel scheduler for true multitasking */
        /* Sovereign: System tasks for USB are assigned UAID 0, UPID 0 */
        int tid = scheduler_add_task(name, (void (*)(void*))entry, argument, 0, 0);
        /* Return the actual Task ID as the thread handle */
        return (usb_osal_thread_t)(uintptr_t)tid;
    }
    return (usb_osal_thread_t)NULL;
}

void usb_osal_thread_delete(usb_osal_thread_t thread) {
    serial_printf("[USB OSAL] Thread delete requested: %p\n", thread);
    if (thread) {
        scheduler_remove_task((int)(uintptr_t)thread);
    }
}
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

    uint64_t start_time = hal_get_uptime_ms();
    while (s->count == 0) {
        if (timeout != 0xFFFFFFFFU && (hal_get_uptime_ms() - start_time) >= timeout) return -1;
        __asm__("pause");
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
    
    uint64_t start_time = hal_get_uptime_ms();
    while (m->head == m->tail) {
        if (timeout != 0xFFFFFFFFU && (hal_get_uptime_ms() - start_time) >= timeout) return -1;
        __asm__("pause");
    }
    
    size_t flags = usb_osal_enter_critical_section();
    *addr = m->messages[m->tail];
    m->tail = (m->tail + 1) % m->max_msgs;
    usb_osal_leave_critical_section(flags);
    return 0;
}

#define MAX_USB_TIMERS 16
static struct usb_osal_timer *timer_list[MAX_USB_TIMERS];
static uint32_t timer_ticks[MAX_USB_TIMERS];
static bool timer_active[MAX_USB_TIMERS];

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

void usb_osal_timer_delete(struct usb_osal_timer *timer) {
    usb_osal_timer_stop(timer);
    usb_osal_free(timer);
}

void usb_osal_timer_start(struct usb_osal_timer *timer) {
    if (!timer) return;
    size_t flags = usb_osal_enter_critical_section();
    for (int i = 0; i < MAX_USB_TIMERS; i++) {
        if (timer_list[i] == timer) {
            timer_ticks[i] = 0;
            timer_active[i] = true;
            usb_osal_leave_critical_section(flags);
            return;
        }
    }
    for (int i = 0; i < MAX_USB_TIMERS; i++) {
        if (timer_list[i] == NULL) {
            timer_list[i] = timer;
            timer_ticks[i] = 0;
            timer_active[i] = true;
            usb_osal_leave_critical_section(flags);
            return;
        }
    }
    usb_osal_leave_critical_section(flags);
}

void usb_osal_timer_stop(struct usb_osal_timer *timer) {
    if (!timer) return;
    size_t flags = usb_osal_enter_critical_section();
    for (int i = 0; i < MAX_USB_TIMERS; i++) {
        if (timer_list[i] == timer) {
            timer_active[i] = false;
            timer_list[i] = NULL;
            break;
        }
    }
    usb_osal_leave_critical_section(flags);
}

/* Modified by Sovereign: Hook for system tick to drive USB OSAL timers */
void usb_osal_tick_handler(void) {
    for (int i = 0; i < MAX_USB_TIMERS; i++) {
        if (timer_list[i] && timer_active[i]) {
            timer_ticks[i] += 10; // Assuming 100Hz tick (10ms)
            if (timer_ticks[i] >= timer_list[i]->timeout_ms) {
                if (timer_list[i]->handler) {
                    timer_list[i]->handler(timer_list[i]->argument);
                }
                if (timer_list[i]->is_period) {
                    timer_ticks[i] = 0;
                } else {
                    timer_active[i] = false;
                }
            }
        }
    }
}

void usb_osal_msleep(uint32_t delay) {
    uint64_t start = hal_get_uptime_ms();
    while ((hal_get_uptime_ms() - start) < delay) {
        __asm__("pause");
    }
}

void *usb_osal_malloc(size_t size) { return tlsf_malloc(tlsf_get_global(), size); }
void usb_osal_free(void *ptr) { tlsf_free(tlsf_get_global(), ptr); }
