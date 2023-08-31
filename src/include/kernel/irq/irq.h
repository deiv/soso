
#pragma once

#include <kernel/compiler/compiler.h>

#include <asm/irq.h>
#include <asm/pic.h>
#include <asm/irq_vector.h>

struct irq_desc;

typedef	void (*irq_handler_t)(struct irq_desc *desc);

struct irq_desc {
    unsigned int		    irq;
    unsigned long		    hwirq;
    struct legacy_pic*      chip;
    irq_handler_t           handle_irq;
};

extern struct irq_desc irq_desc[NR_IRQS];

void irq_init();

static __always_inline void irq_enable()
{
    native_irq_enable();
}

static __always_inline void irq_disable()
{
    native_irq_disable();
}

void irq_setup(unsigned int irq, irq_handler_t handler);
