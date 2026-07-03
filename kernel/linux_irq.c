/* Linux-compatible interrupt registration layer for the x86 APIC-based kernel. */
#include <stdint.h>
#include <stddef.h>
#include "linux/interrupt.h"
#include "pro_os.h"
#include "serial.h"

#define MAX_REQUESTED_IRQS 64

struct irq_entry {
    unsigned int irq;
    irq_handler_t handler;
    void *dev_id;
    bool allocated;
};

static struct irq_entry irq_table[MAX_REQUESTED_IRQS];

int request_irq(unsigned int irq, irq_handler_t handler, unsigned long flags,
                const char *name, void *dev_id) {
    (void)flags;
    (void)name;
    if (irq >= MAX_REQUESTED_IRQS || !handler) return -1;
    for (int i = 0; i < MAX_REQUESTED_IRQS; i++) {
        if (!irq_table[i].allocated) {
            irq_table[i].irq = irq;
            irq_table[i].handler = handler;
            irq_table[i].dev_id = dev_id;
            irq_table[i].allocated = true;
            return 0;
        }
    }
    return -1;
}

void free_irq(unsigned int irq, void *dev_id) {
    for (int i = 0; i < MAX_REQUESTED_IRQS; i++) {
        if (irq_table[i].allocated && irq_table[i].irq == irq && irq_table[i].dev_id == dev_id) {
            irq_table[i].allocated = false;
            irq_table[i].handler = NULL;
            irq_table[i].dev_id = NULL;
            return;
        }
    }
}

void handle_registered_irq(unsigned int irq, struct cpu_state *state) {
    (void)state;
    for (int i = 0; i < MAX_REQUESTED_IRQS; i++) {
        if (irq_table[i].allocated && irq_table[i].irq == irq) {
            if (irq_table[i].handler) {
                irq_table[i].handler(irq, irq_table[i].dev_id);
            }
            return;
        }
    }
}
