
#pragma once

#define IDT_ALIGN	(8 * (1))

#ifndef __ASSEMBLY__

#include <kernel/compiler/compiler_attributes.h>
#include <kernel/compiler/linkage.h>
#include <kernel/std/types.h>

#include <soso/asm/ptrace.h>

/**
 * DECLARE_IDTENTRY - Declare functions for simple IDT entry points
 *		      No error code pushed by hardware
 * @vector:	Vector number (ignored for C)
 * @func:	Function name of the entry point
 *
 * Declares two functions:
 * - The ASM entry point: asm_##func
 * - The C handler called from the ASM entry point
 *
 * Note: This is the C variant of DECLARE_IDTENTRY(). As the name says it
 * declares the entry points for usage in C code. There is an ASM variant
 * as well which is used to emit the entry stubs in entry_64.S.
 */
#define DECLARE_IDTENTRY(vector, func)				\
	asmlinkage void asm_##func(void);				\
	__visible void func(struct pt_regs *regs)

/**
 * DECLARE_IDTENTRY_ERRORCODE - Declare functions for simple IDT entry points
 *				Error code pushed by hardware
 * @vector:	Vector number (ignored for C)
 * @func:	Function name of the entry point
 *
 * Declares two functions:
 * - The ASM entry point: asm_##func
 * - The C handler called from the ASM entry point
 *
 * Same as DECLARE_IDTENTRY, but has an extra error_code argument for the
 * C-handler.
 */
#define DECLARE_IDTENTRY_ERRORCODE(vector, func)			\
	asmlinkage void asm_##func(void);				\
	__visible void func(struct pt_regs *regs, unsigned long error_code)

/**
 * DECLARE_IDTENTRY_IRQ - Declare functions for device interrupt IDT entry
 *			  points (common/spurious)
 * @vector:	Vector number (ignored for C)
 * @func:	Function name of the entry point
 *
 * Maps to DECLARE_IDTENTRY_ERRORCODE()
 */
#define DECLARE_IDTENTRY_IRQ(vector, func)				\
	DECLARE_IDTENTRY_ERRORCODE(vector, func)

/**
 * DEFINE_IDTENTRY - Emit code for simple IDT entry points
 * @func:	Function name of the entry point
 *
 * @func is called from ASM entry code with interrupts disabled.
 *
 * The macro is written so it acts as function definition. Append the
 * body with a pair of curly brackets.
 *
 */
#define DEFINE_IDTENTRY(func)						\
__visible void func(struct pt_regs *regs)

/**
 * DEFINE_IDTENTRY_ERRORCODE - Emit code for simple IDT entry points
 *			       Error code pushed by hardware
 * @func:	Function name of the entry point
 *
 * Same as DEFINE_IDTENTRY, but has an extra error_code argument
 */

#define DEFINE_IDTENTRY_ERRORCODE(func)						\
__visible void func(struct pt_regs *regs,		\
				     unsigned long error_code)

/**
 * DEFINE_IDTENTRY_IRQ - Emit code for device interrupt IDT entry points
 * @func:	Function name of the entry point
 *
 * The vector number is pushed by the low level entry stub and handed
 * to the function as error_code argument which needs to be truncated
 * to an u8 because the push is sign extending.
 *
 */
#define DEFINE_IDTENTRY_IRQ(func)					            \
static void __##func(struct pt_regs *regs, u32 vector);			\
									                            \
__visible void func(struct pt_regs *regs,			            \
			    unsigned long error_code)			            \
{									                            \
	u32 vector = (u32)(u8)error_code;                           \
    __##func (regs, vector);                                    \
}									                            \
									                            \
static noinline void __##func(struct pt_regs *regs, u32 vector)

#else /* !__ASSEMBLY__ */

/*
 * The ASM variants for DECLARE_IDTENTRY*() which emit the ASM entry stubs.
 */
#define DECLARE_IDTENTRY(vector, func)					    \
	idtentry vector asm_##func func has_error_code=0

#define DECLARE_IDTENTRY_ERRORCODE(vector, func)			\
	idtentry vector asm_##func func has_error_code=1

/* Entries for common/spurious (device) interrupts */
#define DECLARE_IDTENTRY_IRQ(vector, func)                  \
	idtentry_irq vector func

#endif

/* Simple exception entry points. No hardware error code */
DECLARE_IDTENTRY(X86_TRAP_DE,		exc_divide_error);
DECLARE_IDTENTRY(X86_TRAP_OF,		exc_overflow);
DECLARE_IDTENTRY(X86_TRAP_BR,		exc_bounds);
DECLARE_IDTENTRY(X86_TRAP_NM,		exc_device_not_available);
DECLARE_IDTENTRY(X86_TRAP_OLD_MF,	exc_coproc_segment_overrun);
DECLARE_IDTENTRY(X86_TRAP_SPURIOUS,	exc_spurious_interrupt_bug);
DECLARE_IDTENTRY(X86_TRAP_MF,		exc_coprocessor_error);
DECLARE_IDTENTRY(X86_TRAP_XF,		exc_simd_coprocessor_error);

/* Simple exception entries with error code pushed by hardware */
DECLARE_IDTENTRY_ERRORCODE(X86_TRAP_TS,	exc_invalid_tss);
DECLARE_IDTENTRY_ERRORCODE(X86_TRAP_NP,	exc_segment_not_present);
DECLARE_IDTENTRY_ERRORCODE(X86_TRAP_SS,	exc_stack_segment);
DECLARE_IDTENTRY_ERRORCODE(X86_TRAP_GP,	exc_general_protection);
DECLARE_IDTENTRY_ERRORCODE(X86_TRAP_AC,	exc_alignment_check);

/* Raw exception entries which need extra work */
DECLARE_IDTENTRY(X86_TRAP_UD,		exc_invalid_op);
DECLARE_IDTENTRY(X86_TRAP_BP,		exc_int3);
DECLARE_IDTENTRY_ERRORCODE(X86_TRAP_PF,	exc_page_fault);

// TODO: should be handled especially
DECLARE_IDTENTRY_ERRORCODE(X86_TRAP_DF,	exc_double_fault);
DECLARE_IDTENTRY(X86_TRAP_DB,	asm_exc_debug);

/*
 * Dummy trap number so the low level ASM macro vector number checks do not
 * match which results in emitting plain IDTENTRY stubs without bells and
 * whistles.
 */
#define X86_TRAP_OTHER		0xFFFF

/* Device interrupts common/spurious */
DECLARE_IDTENTRY_IRQ(X86_TRAP_OTHER,	common_interrupt);
