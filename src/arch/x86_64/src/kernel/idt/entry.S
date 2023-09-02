
#include "calling_convention.h"
#include "kernel/compiler/linkage.h"
#include <asm/irq_vector.h>

.code64
.section .entry.text, "ax"

/**
 * idtentry_body - Macro to emit code calling the C function
 * @cfunc:		C function to be called
 * @has_error_code:	Hardware pushed error code on stack
 */
.macro idtentry_body cfunc has_error_code:req

	PUSH_AND_CLEAR_REGS /*save_ret=1*/
	/*leaq    1(%rsp), %rbp  ENCODE_FRAME_POINTER */

	movq	%rsp, %rdi			/* pt_regs pointer into 1st argument*/

	.if \has_error_code == 1
		movq	ORIG_RAX(%rsp), %rsi	/* get error code into 2nd argument*/
		movq	$-1, ORIG_RAX(%rsp)	    /* no syscall to restart */
	.endif

	call	\cfunc

	POP_REGS
    leaq	8(%rsp), %rdi
    movq	%rdi, %rsp
	movq	%rsp, %rdi

	iretq
.endm

/**
 * idtentry - Macro to generate entry stubs for simple IDT entries
 * @vector:		Vector number
 * @asmsym:		ASM symbol for the entry point
 * @cfunc:		C function to be called
 * @has_error_code:	Hardware pushed error code on stack
 *
 * The macro emits code to set up the kernel context for straight forward
 * and simple IDT entries. No IST stack, no paranoid entry checks.
 */
.macro idtentry vector asmsym cfunc has_error_code:req
SYM_CODE_START(\asmsym)
	cld

	.if \has_error_code == 0
		pushq	$-1			/* ORIG_RAX: no syscall to restart */
	.endif

	idtentry_body \cfunc \has_error_code
SYM_CODE_END(\asmsym)
.endm

/*
 * Interrupt entry/exit.
 *
 + The interrupt stubs push (vector) onto the stack, which is the error_code
 * position of idtentry exceptions, and jump to one of the two idtentry points
 * (common/spurious).
 *
 * common_interrupt is a hotpath, align it to a cache line
 */
.macro idtentry_irq vector cfunc
	idtentry \vector asm_\cfunc \cfunc has_error_code=1
.endm

#include "idt_entry.h"

.align IDT_ALIGN
SYM_CODE_START(irq_entries_start)
    vector=FIRST_EXTERNAL_VECTOR
    .rept NR_EXTERNAL_VECTORS
0 :
	.byte	0x6a, vector
	jmp	asm_common_interrupt
	/* Ensure that the above is IDT_ALIGN bytes max */
	.fill 0b + IDT_ALIGN - ., 1, 0xcc
	vector = vector+1
    .endr
SYM_CODE_END(irq_entries_start)
