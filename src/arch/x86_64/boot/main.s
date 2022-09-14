.global start
.extern long_mode_start

.section .text
.code32
start:
	mov $stack_top, %esp

	/* save multiboot info header pointer */
    push %ebx

	call check_multiboot
	call check_cpuid
	call check_long_mode
	call setup_page_tables
	call enable_paging

	/* In order for the firmware built into the system to optimize itself for
	; running in Long Mode, AMD recommends that the OS notify the BIOS about
	; the intended target environment that the OS will be running in: 32-bit mode, 64-bit mode,
	; or a mixture of both modes. This can be done by calling the BIOS interrupt 15h from Real Mode
	; with AX set to 0xEC00, and BL set to 1 for 32-bit Protected Mode, 2 for 64-bit Long Mode,
	; or 3 if both modes will be used. */

    /* recover multiboot info header pointer */
	pop %ebx

	lgdt (gdtpointer)

	jmp $0x8,$long_mode_start

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

setup_page_tables:
	mov $page_table_l3, %eax
	or $0x3, %eax /* present, writable */
	mov $page_table_l4, %edi
	mov %eax, (%edi)

	mov $page_table_l2, %eax
	or $0x3, %eax /* present, writable */
	mov $page_table_l3, %edi
	mov %eax, (%edi)

	mov $0, %ecx /* counter */
.loop:

	mov $0x200000, %eax /* 2MiB */
	mul %ecx
	or $0x83, %eax /* present, writable, huge page */
	mov $page_table_l2, %edx
	leal (%edx,%ecx,8), %edx
	mov %eax, (%edx)
	#mov %eax, (page_table_l2 + %ecx * 8)

	inc %ecx /* increment counter */
	cmp $512, %ecx /* checks if the whole table is mapped */
	jne .loop /* if not, continue */

	ret

enable_paging:
	/* pass page table location to cpu */
	mov $page_table_l4, %eax
	mov %eax, %cr3

	/* enable PAE */
	mov %cr4, %eax
	or $(1 << 5), %eax
	mov %eax, %cr4

	/* enable long mode */
	mov $0xC0000080, %ecx
	rdmsr
	or $(1 << 8), %eax
	wrmsr

	/* enable paging */
	mov %cr0, %eax
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
page_table_l4:
	.skip 4096
page_table_l3:
	.skip 4096
page_table_l2:
	.skip 4096
stack_bottom:
    .skip 16384 # 16 KiB
stack_top:

.section .rodata
gdt64:
.quad 0x0000000000000000
gdtcode:
    .quad (1 << 43) | (1 << 44) | (1 << 47) | (1 << 53) # code segment
gdtdata:
    .quad 0x0000900000000000 # data segment
gdtpointer:
    .word .-gdt64-1
    .quad gdt64

CODE64_SEL = gdtcode-gdt64
DATA64_SEL = gdtdata-gdt64
