
#pragma once

/*
 x86 function call convention, 64-bit:
 -------------------------------------
  arguments           |  callee-saved      | extra caller-saved | return
 [callee-clobbered]   |                    | [callee-clobbered] |
 ---------------------------------------------------------------------------
 rdi rsi rdx rcx r8-9 | rbx rbp [*] r12-15 | r10-11             | rax, rdx [**]

 ( rsp is obviously invariant across normal function calls. (gcc can 'merge'
   functions when it sees tail-call optimization possibilities) rflags is
   clobbered. Leftover arguments are passed over the stack frame.)

 [*]  In the frame-pointers case rbp is fixed to the stack frame.

 [**] for struct return values wider than 64 bits the return convention is a
      bit more complex: up to 128 bits width we return small structures
      straight in rax, rdx. For structures larger than that (3 words or
      larger) the caller puts a pointer to an on-stack return struct
      [allocated in the caller's stack frame] into the first argument - i.e.
      into rdi. All other arguments shift up by one in this case.
      Fortunately this case is rare in the kernel.
*/

#ifdef __ASSEMBLY__

/*
 * On syscall entry, this is syscall#. On CPU exception, this is error code.
 * On hw interrupt, it's IRQ number:
 */
#define ORIG_RAX 120

.macro PUSH_REGS rdx=%rdx rcx=%rcx rax=%rax save_ret=0
    .if \save_ret
        pushq	%rsi		/* pt_regs->si */
        movq	8(%rsp), %rsi	/* temporarily store the return address in %rsi */
        movq	%rdi, 8(%rsp)	/* pt_regs->di (overwriting original return address) */
    .else
        pushq   %rdi		/* pt_regs->di */
        pushq   %rsi		/* pt_regs->si */
    .endif

    pushq	\rdx		/* pt_regs->dx */
    pushq   \rcx		/* pt_regs->cx */
    pushq   \rax		/* pt_regs->ax */
    pushq   %r8		    /* pt_regs->r8 */
    pushq   %r9		    /* pt_regs->r9 */
    pushq   %r10		/* pt_regs->r10 */
    pushq   %r11		/* pt_regs->r11 */
    pushq	%rbx		/* pt_regs->rbx */
    pushq	%rbp		/* pt_regs->rbp */
    pushq	%r12		/* pt_regs->r12 */
    pushq	%r13		/* pt_regs->r13 */
    pushq	%r14		/* pt_regs->r14 */
    pushq	%r15		/* pt_regs->r15 */
    /*UNWIND_HINT_REGS*/

    .if \save_ret
            pushq	%rsi		/* return address on top of stack */
    .endif
.endm

.macro CLEAR_REGS
    /*
     * Sanitize registers of values that a speculation attack might
     * otherwise want to exploit. The lower registers are likely clobbered
     * well be  fore they could be put to use in a speculative execution
     * gadget.
     */
    xorl	%esi,  %esi	/* nospec si  */
    xorl	%edx,  %edx	/* nospec dx  */
    xorl	%ecx,  %ecx	/* nospec cx  */
    xorl	%r8d,  %r8d	/* nospec r8  */
    xorl	%r9d,  %r9d	/* nospec r9  */
    xorl	%r10d, %r10d	/* nospec r10 */
    xorl	%r11d, %r11d	/* nospec r11 */
    xorl	%ebx,  %ebx	/* nospec rbx */
    xorl	%ebp,  %ebp	/* nospec rbp */
    xorl	%r12d, %r12d	/* nospec r12 */
    xorl	%r13d, %r13d	/* nospec r13 */
    xorl	%r14d, %r14d	/* nospec r14 */
    xorl	%r15d, %r15d	/* nospec r15 */

.endm

.macro PUSH_AND_CLEAR_REGS rdx=%rdx rcx=%rcx rax=%rax save_ret=0
    PUSH_REGS rdx=\rdx, rcx=\rcx, rax=\rax, save_ret=\save_ret
    /*CLEAR_REGS*/
.endm

.macro POP_REGS pop_rdi=1
	popq %r15
	popq %r14
	popq %r13
	popq %r12
	popq %rbp
	popq %rbx
	popq %r11
	popq %r10
	popq %r9
	popq %r8
	popq %rax
	popq %rcx
	popq %rdx
	popq %rsi
	.if \pop_rdi
	popq %rdi
	.endif
.endm

#endif
