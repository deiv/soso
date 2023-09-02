.section .multiboot_header
.align 8
header_start:
	/* magic number */
	.long 0xe85250d6 /* multiboot2 */
	/* architecture */
	.long 0 /* protected mode i386 */
	/* header length */
	.long header_end - header_start
	/* checksum */
	.long 0x100000000 - (0xe85250d6 + 0 + (header_end - header_start))

.align 8
mb2_tag_console_start:
        .short 4                                    /* console flags */
        .short 0
        .long mb2_tag_console_end - mb2_tag_console_start
        .long 0x2
mb2_tag_console_end:

/* Framebuffer tag
.align 8
multiboot2_fb:
.short 5
.short 0
.long multiboot2_fb_end - multiboot2_fb
.long 80          *//* width */
.long 25          /* height */
.long 0            /* depth */
multiboot2_fb_end:
.align 8
	/* end tag */
	.word 0
	.word 0
	.long 8
header_end:
