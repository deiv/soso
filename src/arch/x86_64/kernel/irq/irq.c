
#include "../idt/idt_entry.h"

#include <kernel/irq/irq.h>

/*
 * common_interrupt() handles all normal device IRQ's.
 */
DEFINE_IDTENTRY_IRQ(common_interrupt)
{
    struct irq_desc *desc;

    desc = &irq_desc[vector];
    desc->handle_irq(desc);
}
