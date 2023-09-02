
#include <asm/memory.h>

.global long_mode_start
.extern kernel_main
.extern boot_p4
.extern boot_p3
.extern gdt_descr

.section .text
.code64
long_mode_start:
    /* jump to virtual address space */
	movabsq	$higher_half, %rax
	jmp	*%rax

higher_half:
	/* point stack to the VMA space */
	movq $KERNEL_OFFSET, %rax
	addq %rax, %rsp
	addq %rax, %rbp

	/* reload VMA gdt */
	lgdt gdt_descr

	/* remove identity mapping */
	movq $boot_p4,%rax
	movq $0x0,(%rax)
	movq $boot_p3,%rax
	movq $0x0,(%rax)
	invlpg	0

    /* point multiboot info to VMA and pass to kernel main */
    addq $(KERNEL_OFFSET), %rbx
    mov %rbx, %rdi

	/* call the kernel */
	callq kernel_main

	cli
1:	hlt
	jmp 1b
	.size long_mode_start, . - long_mode_start
