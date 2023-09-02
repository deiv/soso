
#include "asm/memory.h"

.global gdt_descr_phys
.global gdt_descr

.section .gdt64
    .align 16
gdt64:
	.quad 0x0000000000000000	/* null descriptor */
gdtcode32:
	.quad 0x00cf9a000000ffff	/* __KERNEL_CS 32 */
gdtcode64:
	.quad 0x00af9a000000ffff	/* __KERNEL_CS 64 */
gdtdata:
    .quad 0x00cf92000000ffff	/* __KERNEL_DS */
    .quad 0x0080890000000000	/* TS descriptor */
    .quad 0x0000000000000000	/* TS continued */

	/*.2byte 0x0068		// TSS selector
	.2byte 0x0
	.byte 0x0
	.byte 0x89
	.2byte 0x0
	.8byte 0x0
	.8byte 0x0*/
boot_gdt_end:

gdt_descr_phys:
	.2byte boot_gdt_end - gdt64
	.8byte gdt64 - KERNEL_OFFSET
gdt_descr:
	.2byte boot_gdt_end - gdt64
	.8byte gdt64

CODE64_SEL = gdtcode64-gdt64
DATA64_SEL = gdtdata-gdt64
