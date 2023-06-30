
#pragma once

#define BASE_BAUD ( 1843200 / 16 )

#define STD_COM_FLAGS (SERIAL_PORT_AUTOCONF | SERIAL_PORT_SKIP_TEST)
#define STD_COM4_FLAGS SERIAL_PORT_AUTOCONF

#define ARCH_UNENUMERABLE_SERIAL_PORTS			                            \
	/* UART         CLK	        PORT    IRQ	FLAGS */                        \
	{ .uart = 0,	BASE_BAUD,	0x3F8,	4,	STD_COM_FLAGS	}, /* ttyS0 */	\
	{ .uart = 0,	BASE_BAUD,	0x2F8,	3,	STD_COM_FLAGS	}, /* ttyS1 */	\
	{ .uart = 0,	BASE_BAUD,	0x3E8,	4,	STD_COM_FLAGS	}, /* ttyS2 */	\
	{ .uart = 0,	BASE_BAUD,	0x2E8,	3,	STD_COM4_FLAGS	}, /* ttyS3 */
