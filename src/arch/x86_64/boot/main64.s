.global long_mode_start
.extern kernel_main

.section .text
.code64
long_mode_start:
    /* load null into all data segment registers */
    mov $0, %ax
    mov %ax, %ss
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs

    mov %rbx, %rdi
	call kernel_main

    hlt
