
#include <kernel/std/array.h>
#include <kernel/printk.h>
#include <kernel/irq/irq.h>
#include <kernel/bug/runtime_bug.h>

#include <asm/pic.h>
#include <asm/irq_vector.h>

struct irq_desc irq_desc[NR_IRQS];
extern struct legacy_pic legacy_pic;

void handle_bad_irq(struct irq_desc *desc)
{
    unsigned int irq = desc->irq;

    printk("bad irq %d, desc: %p, not handler\n", irq, desc);
}

void irq_init()
{
    int idx, count;
    struct irq_desc *desc;

    printk("NR_IRQS: %d\n", NR_IRQS);

    count = ARRAY_SIZE(irq_desc);

    for (idx = 0; idx < count; idx++) {
        desc = &irq_desc[idx];
        desc->chip = &legacy_pic;
        desc->irq = idx;
        desc->hwirq = 0;
        desc->handle_irq = &handle_bad_irq;
    }
}

void irq_setup(unsigned int irq, irq_handler_t handler)
{
    struct irq_desc *desc;

    BUG_ON(irq >= NR_IRQS);
    BUG_ON(!handler);

    desc = &irq_desc[irq];

    desc->handle_irq = handler;
    // All irqs comes from legacy chip
    desc->hwirq = VECTOR_ISA_IRQ(irq);
    desc->chip->mask(desc->hwirq);
}
