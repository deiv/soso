
#include <asm/pic.h>
#include <asm/io.h>
#include <asm/irq_vector.h>
#include <kernel/printk.h>

/*
 * http://www.brokenthorn.com/Resources/OSDevPic.html
 */

/*
 * i8259A PIC registers
 */
#define PIC_MASTER_BASE_ADDR	0x20 /* IO base address for master PIC */
#define PIC_SLAVE_BASE_ADDR		0xa0 /* IO base address for slave PIC */
#define PIC_MASTER_COMMAND      (PIC_MASTER_BASE_ADDR)
#define PIC_MASTER_DATA         (PIC_MASTER_BASE_ADDR + 1)
#define PIC_SLAVE_COMMAND       (PIC_SLAVE_BASE_ADDR)
#define PIC_SLAVE_DATA          (PIC_SLAVE_BASE_ADDR + 1)

/*
 * i8259A PIC related values
 */
#define PIC_ICW1_INIT	0x10		/* Initialization - required */
#define PIC_ICW1_ICW4	0x01		/* Indicates that ICW4 will be present */
#define PIC_ICW4_8086	0x01		/* 8086/88 (MCS-80/85) mode */
#define PIC_ICW4_AUTO	0x02		/* Auto (normal) EOI */
#define PIC_CASCADE_IR	2           /* Cascade on IRQ 2 */

static int probe_8259A(void)
{
    unsigned char probe_val = ~(1 << PIC_CASCADE_IR);
    unsigned char new_val;

    /*
     * Check to see if we have a PIC.
     * Mask all except the cascade and read
     * back the value we just wrote. If we don't
     * have a PIC, we will read 0xff as opposed to the
     * value we wrote.
     */

    /* mask all of 8259A-2 */
    outb(0xff, PIC_SLAVE_DATA);
    outb(probe_val, PIC_MASTER_DATA);
    new_val = inb(PIC_MASTER_DATA);

    if (new_val != probe_val) {
        printk("Using NULL legacy PIC\n");

    } else {
        /*
         * TODO: Use dummy legacy pic
         */
        printk("Using 8259A legacy PIC\n");
    }

    return NR_IRQS_LEGACY;
}

static void init_8259A(int auto_eoi)
{
    unsigned long flags;

    u8 master_mask, slave_mask;

    master_mask = inb(PIC_MASTER_DATA);
    slave_mask = inb(PIC_SLAVE_DATA);

    /* mask all of 8259A-1 */
    outb(0xff, PIC_MASTER_DATA);

    /* ICW1: select 8259A-1 init */
    outb(PIC_ICW1_INIT | PIC_ICW1_ICW4, PIC_MASTER_COMMAND);

    /* ICW2: 8259A-1 IR0-7 mapped to ISA_IRQ_VECTOR(0) */
    outb(ISA_IRQ_VECTOR(0), PIC_MASTER_DATA);

    /* ICW3: 8259A-1 (the master) has a slave on IR2 on IRQ 2 */
    outb(1U << PIC_CASCADE_IR, PIC_MASTER_DATA);

    if (auto_eoi)	/* master does Auto EOI */
        outb(PIC_ICW4_8086 | PIC_ICW4_AUTO, PIC_MASTER_DATA);
    else		/* master expects normal EOI */
        outb(PIC_ICW4_8086, PIC_MASTER_DATA);

    /* ICW1: select 8259A-2 init */
    outb(PIC_ICW1_INIT | PIC_ICW1_ICW4, PIC_SLAVE_COMMAND);

    /* ICW2: 8259A-2 IR0-7 mapped to ISA_IRQ_VECTOR(8) */
    outb(ISA_IRQ_VECTOR(8), PIC_SLAVE_DATA);

    /* ICW3: 8259A-2 is a slave on master's IR2 IRQ 2 */
    outb(PIC_CASCADE_IR, PIC_SLAVE_DATA);

    outb(PIC_ICW4_8086, PIC_SLAVE_DATA);

    /*
     * TODO: In AEOI mode we just have to mask the interrupt when acking.
     */

    /* restore master and salIRQ mask */
    outb(master_mask, PIC_MASTER_DATA);
    outb(slave_mask, PIC_SLAVE_DATA);
}

struct legacy_pic legacy_pic = {
        .init = init_8259A,
        .probe = probe_8259A
};
