
#pragma once

#include <kernel/serial/serial.h>
#include <kernel/container_of.h>

struct uart_8250_port;

/**
 * 8250 core driver operations
 *
 * @setup_irq()		Setup irq handling. The universal 8250 driver links this
 *			port to the irq chain. Other drivers may @request_irq().
 * @release_irq()	Undo irq handling. The universal 8250 driver unlinks
 *			the port from the irq chain.
 */
struct uart_8250_ops {
    int	 (*setup_irq)(struct uart_8250_port *);
    void (*release_irq)(struct uart_8250_port *);

    /* 8250 specific callbacks */
    int	 (*dl_read)(struct uart_8250_port *);
    void (*dl_write)(struct uart_8250_port *, int);
};

/*
 * All the I/O ports are calculated relative to the data port. This is because
 * all serial ports (COM1, COM2, COM3, COM4) have their ports in the same
 * order, but they start at different values.
 */
#define SERIAL_OFFSET_DATA_PORT 0
#define SERIAL_OFFSET_INTERRUPT_PORT 1
#define SERIAL_OFFSET_FIFO_PORT 2
#define SERIAL_OFFSET_LINE_CONTROL_PORT 3
#define SERIAL_OFFSET_MODEM_PORT 4
#define SERIAL_OFFSET_LINE_STATUS_PORT 5
#define SERIAL_OFFSET_SCRATCH_PORT 7

#define SERIAL_OFFSET_DLL	0	/* Out: Divisor Latch Low */
#define SERIAL_OFFSET_DLM	1	/* Out: Divisor Latch High */

/* The I/O port commands */

/*
 * SERIAL_LINE_ENABLE_DLAB:
 * Tells the serial port to expect first the highest 8 bits on the data port,
 * then the lowest 8 bits will follow
 */
#define SERIAL_LINE_ENABLE_DLAB         0x80

/**
 * 8250 port descriptor
 *
 * This should be used by drivers which want to register
 * their own 8250 ports without registering their own
 * platform device.  Using these will make your driver
 * dependent on the 8250 driver.
 */
struct uart_8250_port {
    struct uart_port	  port;
    struct uart_8250_ops *ops;
};

static inline struct uart_8250_port *up_to_u8250p(struct uart_port *up)
{
    return container_of(up, struct uart_8250_port, port);
}
