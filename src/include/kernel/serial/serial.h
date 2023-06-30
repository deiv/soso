
#pragma once

#include <kernel/std/types.h>

typedef unsigned int upf_t;

#define SERIAL_PORT_SKIP_TEST	((upf_t) (1 << 0))
#define SERIAL_PORT_AUTOCONF	((upf_t) (1 << 1))

#define SERIAL_IO_TYPE_PORT	0

struct uart_ops {

};

struct uart_port {
    u8  iotype;			/* io access style */
    u64 iobase;			/* SERIAL_IO_TYPE_PORT: in/out[bwl] */
    u32 line;
    u32 uartclk;
    upf_t flags;

    const struct uart_ops* ops;

    int  	(*serial_in)(struct uart_port *, int);
    void	(*serial_out)(struct uart_port *, int, int);
};
