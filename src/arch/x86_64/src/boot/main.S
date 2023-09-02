
#include <asm/memory.h>

.global start
.extern long_mode_start
.extern gdt_descr_phys
.extern boot_p4

.section .text
.code32
start:
	/* point to our stack */
	movl $(virtual_to_physical(stack_top)), %esp
	movl $(virtual_to_physical(stack_bottom)), %ebp

	call check_multiboot

	/* setup 32 gdt and code segment */
	lgdt virtual_to_physical(gdt_descr_phys)
	ljmp $8, $(virtual_to_physical(cs_set))
cs_set:

	/* setup data segment */
	movw $0,%ax
	movw %ax,%ds
	movw %ax,%es
	movw %ax,%fs
	movw %ax,%gs
	movw $24,%ax
	movw %ax,%ds
	movw %ax,%es
	movw %ax,%fs
	movw %ax,%gs

	/* save multiboot info header pointer */
    push %ebx

	call check_cpuid
	call check_long_mode
	call enable_paging

	/* In order for the firmware built into the system to optimize itself for
	  running in Long Mode, AMD recommends that the OS notify the BIOS about
	  the intended target environment that the OS will be running in: 32-bit mode, 64-bit mode,
	  or a mixture of both modes. This can be done by calling the BIOS interrupt 15h from Real Mode
	  with AX set to 0xEC00, and BL set to 1 for 32-bit Protected Mode, 2 for 64-bit Long Mode,
	  or 3 if both modes will be used. */

    /* recover multiboot info header pointer */
	pop %ebx

    ljmp $0x10, $(virtual_to_physical(long_mode_start))

	hlt

check_multiboot:
	cmp $0x36d76289, %eax
	jne .no_multiboot
	ret
.no_multiboot:
	mov $'M', %al
	jmp error

check_cpuid:
	pushf
	popl %eax
	mov %eax, %ecx
	xorl $(1 << 21), %eax
	push %eax
	popf
	pushf
	pop %eax
	push %ecx
	popf
	cmp %ecx, %eax
	je .no_cpuid
	ret
.no_cpuid:
	mov $'C', %al
	jmp error

check_long_mode:
	mov $0x80000000, %eax
	cpuid
	cmp $0x80000001, %eax
	jb .no_long_mode

	mov $0x80000001, %eax
	cpuid
	test $(1 << 29), %edx
	jz .no_long_mode

	ret
.no_long_mode:
	mov $'L', %al
	jmp error

enable_paging:
	/* pass page table location to cpu */
	movl $(virtual_to_physical(boot_p4)), %eax
	movl %eax, %cr3

	/* enable PAE */
	movl %cr4, %eax
    orl $1 << 5, %eax
    mov %eax, %cr4

	/* enable long mode */
	mov $0xC0000080, %ecx
	rdmsr
	or $1 << 8, %eax
	wrmsr

	/* enable paging */
	movl %cr0, %eax
	or $(1 << 31), %eax
	mov %eax, %cr0

	ret

error:
	/* print "ERR: X" where X is the error code */
	movl $0x4f524f45, (0xb8000)
	movl $0x4f3a4f52, (0xb8004)
	movl $0x4f204f20, (0xb8008)
	movb %al, (0xb800a)
	hlt

.section .bss, "aw", @nobits
    .align 4096
stack_bottom:
    .skip 16384 // 16 KiB
stack_top:
