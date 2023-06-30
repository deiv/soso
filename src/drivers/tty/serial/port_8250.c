
#include <kernel/serial/serial_8250.h>
#include <asm/io.h>

#include "port_8250.h"
#include "common_8250.h"

#define SCRATCH_TEST_VALUE 0x2A

static void autoconfig(struct uart_8250_port *up)
{
    struct uart_port *port = &up->port;
    u8 scratch_test;
    u8 fifo_test;

    serial_out(up, SERIAL_OFFSET_FIFO_PORT, 0xE7);
    fifo_test = serial_in(up, SERIAL_OFFSET_FIFO_PORT);

    switch (fifo_test >> 6) {
        /* ports without FIFO */
        case 0:
            /* 8250 have the scratch broken */
            serial_out(up, SERIAL_OFFSET_SCRATCH_PORT, SCRATCH_TEST_VALUE);
            scratch_test = serial_in(up, SERIAL_OFFSET_SCRATCH_PORT);
            if (SCRATCH_TEST_VALUE == scratch_test)
                up->port.type = PORT_16450;
            else
                up->port.type = PORT_8250;
            break;

        /* reserved, unexpected */
        case 1:
            port->type = PORT_UNKNOWN;
            break;

        /* FIFO broken */
        case 2:
            port->type = PORT_16550;
            break;

        /* FIFO enabled */
        case 3:
            /* 64 Byte FIFO Enabled (16750 only) */
            if ((fifo_test & 0x20) > 0)
                up->port.type = PORT_16750;
            else
                up->port.type = PORT_16550A;
            break;
    }
}

static void serial8250_config_port(struct uart_port *port, u32 flags)
{
    struct uart_8250_port *up = up_to_u8250p(port);

    if (flags & SERIAL_PORT_AUTOCONF)
        autoconfig(up);

    /* TODO: now we can enable FIFO */
}

static const struct uart_ops serial8250_ops = {
    .config_port	= serial8250_config_port
};

/* Uart divisor latch read */
static int default_serial_dl_read(struct uart_8250_port *up)
{
    /* Assign these in pieces to truncate any bits above 7.  */
    unsigned char dll = serial_in(up, SERIAL_OFFSET_DLL);
    unsigned char dlm = serial_in(up, SERIAL_OFFSET_DLM);

    return dll | dlm << 8;
}

/* Uart divisor latch write */
static void default_serial_dl_write(struct uart_8250_port *up, int value)
{
    serial_out(up, SERIAL_OFFSET_DLL, value & 0xff);
    serial_out(up, SERIAL_OFFSET_DLM, value >> 8 & 0xff);
}

static unsigned int io_serial_in(struct uart_port *p, int offset)
{
    return inb(p->iobase + offset);
}

static void io_serial_out(struct uart_port *p, int offset, int value)
{
    outb(value, p->iobase + offset);
}

static void set_io_from_upio(struct uart_port *p)
{
    struct uart_8250_port *up = up_to_u8250p(p);

    up->ops->dl_read = default_serial_dl_read;
    up->ops->dl_write = default_serial_dl_write;

    switch (p->iotype) {
        case SERIAL_IO_TYPE_PORT:
            fallthrough;

        default:
            p->serial_in = io_serial_in;
            p->serial_out = io_serial_out;
            break;
    }
}

void serial8250_init_port(struct uart_8250_port *up)
{
    struct uart_port *port = &up->port;

    port->ops = &serial8250_ops;
    set_io_from_upio(port);
}
