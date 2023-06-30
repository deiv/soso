
#pragma once

#include <kernel/std/types.h>
#include <kernel/compiler/compiler_types.h>
#include <kernel/serial/serial_8250.h>

struct arch_unenumerable_serial_port {
    u32         uart;
    u32         baud_base;
    u32         port;
    u32         irq;
    upf_t       flags;
    u8          io_type;
    u8 __iomem *iomem_base;
    u16         iomem_reg_shift;
};

static inline int serial_in(struct uart_8250_port *up, int offset)
{
    return up->port.serial_in(&up->port, offset);
}

static inline void serial_out(struct uart_8250_port *up, int offset, int value)
{
    up->port.serial_out(&up->port, offset, value);
}
