
#pragma once

#include <kernel/std/types.h>

typedef unsigned int upf_t;

#define SERIAL_PORT_SKIP_TEST	((upf_t) (1 << 0))
#define SERIAL_PORT_AUTOCONF	((upf_t) (1 << 1))

/*
 * Supported serial types.
 */
#define PORT_UNKNOWN	0
#define PORT_8250	    1
#define PORT_16450	    2
#define PORT_16550	    3
#define PORT_16550A	    4
#define PORT_16750	    5

/*
 * Supported serial IO types.
 */
#define SERIAL_IO_TYPE_PORT	0

struct uart_port;

struct uart_ops {
    void (*config_port)(struct uart_port *, u32);
};

struct uart_port {
    u8  iotype;			/* io access style */
    u64 iobase;			/* SERIAL_IO_TYPE_PORT: in/out[bwl] */
    u32 line;
    u32 uartclk;
    upf_t flags;
    u32 type;

    const struct uart_ops* ops;

    int  	(*serial_in)(struct uart_port *, int);
    void	(*serial_out)(struct uart_port *, int, int);
};
