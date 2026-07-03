#ifndef LINUX_INTERRUPT_H
#define LINUX_INTERRUPT_H

#include "linux/irqreturn.h"

#define IRQF_SHARED 0x0001
#define IRQF_TRIGGER_NONE 0x0000

typedef irqreturn_t (*irq_handler_t)(int irq, void *dev_id);

int request_irq(unsigned int irq, irq_handler_t handler, unsigned long flags,
                const char *name, void *dev_id);
void free_irq(unsigned int irq, void *dev_id);

#endif
