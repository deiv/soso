
#include "calling_convention.h"
#include "kernel/compiler/linkage.h"

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

#include "idt_entry.h"
