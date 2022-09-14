.section .multiboot_header
header_start:
	/* magic number */
	.long 0xe85250d6 /* multiboot2 */
	/* architecture */
	.long 0 /* protected mode i386 */
	/* header length */
	.long header_end - header_start
	/* checksum */
	.long 0x100000000 - (0xe85250d6 + 0 + (header_end - header_start))

	/* end tag */
	.word 0
	.word 0
	.long 8
header_end:
