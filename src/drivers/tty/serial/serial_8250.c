
#include <kernel/std/types.h>
#include <kernel/serial/serial.h>
#include <kernel/serial/serial_8250.h>
#include <kernel/console.h>
#include <kernel/printk.h>

#include <asm/serial.h>

#include "port_8250.h"
#include "common_8250.h"

/*
 * SERIAL_PORT_DFNS tells us about built-in ports that have no
 * standard enumeration mechanism. Platforms that can find all
 * serial ports via mechanisms like ACPI or PCI need not supply it.
 */
#ifndef ARCH_UNENUMERABLE_SERIAL_PORTS
    #define ARCH_UNENUMERABLE_SERIAL_PORTS
#endif


static const struct arch_unenumerable_serial_port
        unenumerable_ports[] = { ARCH_UNENUMERABLE_SERIAL_PORTS };

#define NUM_UNENUMERABLE_PORTS (sizeof(unenumerable_ports) / sizeof(unenumerable_ports[0]))

static const u32 max_num_ports = NUM_UNENUMERABLE_PORTS;
static struct uart_8250_port serial8250_ports[NUM_UNENUMERABLE_PORTS];

static const struct uart_8250_ops univ8250_driver_ops = {
        /*.setup_irq	= univ8250_setup_irq,
        .release_irq	= univ8250_release_irq,*/
};

int serial8250_console_init(struct console *console);
void serial8250_console_put_char(struct console *console, char a);
void serial8250_console_put_string(struct console *console, const char* string);

struct serial8250_console_state {
    const struct console serial8250_console;
    struct uart_8250_port* port;
};

static struct serial8250_console_state serial8250_console_state = {
    .serial8250_console = {
        .name = "ttyS",
        .setup = serial8250_console_init,
        .put_char = serial8250_console_put_char,
        .put_string = serial8250_console_put_string
    },
    .port = NULL
};

const struct console* serial8250_console = &serial8250_console_state.serial8250_console;

static inline struct uart_8250_port
        *console_to_serial8250_port(struct console *console)
{
    struct serial8250_console_state* state =
            container_of(console, struct serial8250_console_state, serial8250_console);

    if (state != NULL) {
        return state->port;
    }

    return NULL;
}

void serial8250_isa_init_ports()
{
    for (u32 i = 0; i < max_num_ports; i++) {

        struct uart_8250_port *up = &serial8250_ports[i];
        struct uart_port *port = &up->port;

        port->line     = i;
        port->uartclk  = unenumerable_ports[i].baud_base * 16;
        port->iobase   = unenumerable_ports[i].port;
        //port->irq      = unenumerable_ports[i].irq;
        //port->irqflags = 0;
        //port->irqflags |= IRQF_SHARED;
        port->flags    = unenumerable_ports[i].flags;
        port->iotype   = unenumerable_ports[i].io_type;

        up->ops = &univ8250_driver_ops;

        serial8250_init_port(up);
    }

    printk("Serial: 8250/16550 driver, %d ports\n", max_num_ports);
}

/**
 * serial_configure_baud_rate:
 *  Sets the speed of the data being sent. The default speed of a serial
 *  port is 115200 bits/s. The argument is a divisor of that number, hence
 *  the resulting speed becomes (115200 / divisor) bits/s.
 *
 *  @param port     The COM port to configure
 *  @param divisor  The divisor
 */
void serial_configure_baud_rate(struct uart_8250_port *port, unsigned short divisor)
{
    s32 control_value = serial_in(port, SERIAL_OFFSET_LINE_CONTROL_PORT);
    serial_out(port,
               SERIAL_OFFSET_LINE_CONTROL_PORT,
               control_value | SERIAL_LINE_ENABLE_DLAB);
    port->ops->dl_write(port, divisor);
    serial_out(port,
               SERIAL_OFFSET_LINE_CONTROL_PORT,
               control_value);
}

/**
 * serial_configure_line:
 *  Configures the line of the given serial port. The port is set to have a
 *  data length of 8 bits, no parity bits, one stop bit and break control
 *  disabled.
 *
 *  @param port  The serial port to configure
 */
void serial_configure_line(struct uart_8250_port *port)
{
    /* Bit:     | 7 | 6 | 5 4 3 | 2 | 1 0 |
     * Content: | d | b | prty  | s | dl  |
     * Value:   | 0 | 0 | 0 0 0 | 0 | 1 1 | = 0x03
     */
    serial_out(port, SERIAL_OFFSET_LINE_CONTROL_PORT, 0x03);

    // Enable FIFO, clear them, with 14-byte threshold
    serial_out(port, SERIAL_OFFSET_FIFO_PORT, 0xC7);

    // IRQs enabled, RTS/DSR set -> 0x0B
    // IRQs disabled, RTS/DSR set
    serial_out(port, SERIAL_OFFSET_MODEM_PORT, 0x03);
}

/**
 * serial_is_transmit_fifo_empty:
 *  Checks whether the transmit FIFO queue is empty or not for the given COM
 *  port.
 *
 *  @param  port The serial port
 *  @return 0 if the transmit FIFO queue is not empty
 *          1 if the transmit FIFO queue is empty
 */
u32 serial_is_transmit_fifo_empty(struct uart_8250_port *port)
{
    return serial_in(port, SERIAL_OFFSET_LINE_STATUS_PORT) & 0x20;
}

void write_serial(struct uart_8250_port *port, char a)
{
    while (serial_is_transmit_fifo_empty(port) == 0);

    serial_out(port, SERIAL_OFFSET_DATA_PORT, a);
}

int serial8250_console_init(struct console *console) {

    serial8250_isa_init_ports();

    struct uart_8250_port* console_port = &serial8250_ports[0];
    struct uart_port* port = &console_port->port;

    /* disable interrupts */
    port->serial_out(port, SERIAL_OFFSET_INTERRUPT_PORT, 0x0);

    serial_configure_baud_rate(console_port, port->uartclk / 16 / 115200);
    serial_configure_line(console_port);

    serial8250_console_state.port = console_port;

    printk("Serial: 8250/16550 console, using port line %i (iobase: 0x%x) as console\n",
           console_port->port.line,
           console_port->port.iobase);

    return 0;
}

void serial8250_console_put_char(struct console *console, char a)
{
    struct uart_8250_port* port = console_to_serial8250_port(console);

    write_serial(port, a);
}

void serial8250_console_put_string(struct console *console, const char* string)
{
    struct uart_8250_port* port = console_to_serial8250_port(console);

    for (size_t i = 0; 1; i++) {
        char character = string[i];

        if (character == '\0')
            break;

        if (character == '\n')
            write_serial(port, '\r');

        write_serial(port, character);
    }

    /* wait for transmission complete */
    while (serial_is_transmit_fifo_empty(port) == 0);
}
