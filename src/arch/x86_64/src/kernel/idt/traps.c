
#include "idt_entry.h"
#include <kernel/printk.h>
#include <kernel/panic.h>
#include <soso/asm/ptrace.h>

void die_if_kernel(char * str, struct pt_regs * regs, long err)
{
    printk("%s: %04lx\n", str, err & 0xffff);

    printk("RIP: %04x:%pS\n", (int)regs->cs, (void *)regs->ip);
    printk("RSP: %04x:%016lx EFLAGS: %08lx\n", (int)regs->ss, regs->sp, regs->flags);

    printk("RAX: %016lx RBX: %016lx RCX: %016lx\n",
           regs->ax, regs->bx, regs->cx);
    printk("RDX: %016lx RSI: %016lx RDI: %016lx\n",
           regs->dx, regs->si, regs->di);
    printk("RBP: %016lx R08: %016lx R09: %016lx\n",
           regs->bp, regs->r8, regs->r9);
    printk("R10: %016lx R11: %016lx R12: %016lx\n",
           regs->r10, regs->r11, regs->r12);
    printk("R13: %016lx R14: %016lx R15: %016lx\n",
           regs->r13, regs->r14, regs->r15);
    printk("\n");

    panic("kernel exception");
}

#define DO_ERROR(trapnr, signr, str, name) \
DEFINE_IDTENTRY(name) \
{ \
	die_if_kernel(str, regs, 0); \
}

#define DO_ERROR_CODE_TRAP(trapnr, signr, str, name) \
DEFINE_IDTENTRY_ERRORCODE(name) \
{ \
	die_if_kernel(str, regs, error_code); \
}

DEFINE_IDTENTRY(exc_int3) \
{ \
	printk("breakpoint exception\n"); \
}

DO_ERROR(X86_TRAP_DE, SIGFPE,  "divide error", exc_divide_error)
// TODO: should be handled especially
DO_ERROR(X86_TRAP_DB, SIGSEGV, "debug", asm_exc_debug)
// TODO: should be handled especially
//DO_ERROR(X86_TRAP_BP, SIGTRAP, "int3", exc_int3)
DO_ERROR(X86_TRAP_OF, SIGSEGV, "overflow", exc_overflow)
DO_ERROR(X86_TRAP_BR, SIGSEGV, "bounds", exc_bounds)
DO_ERROR(X86_TRAP_UD, SIGILL,  "invalid operand", exc_invalid_op)
DO_ERROR(X86_TRAP_NM, SIGSEGV, "device not available", exc_device_not_available)
// TODO: should be handled especially
DO_ERROR_CODE_TRAP(X86_TRAP_DF, SIGSEGV, "double fault", exc_double_fault)
DO_ERROR(X86_TRAP_OLD_MF, SIGFPE, "coprocessor segment overrun", exc_coproc_segment_overrun)
DO_ERROR_CODE_TRAP(X86_TRAP_TS, SIGSEGV, "invalid TSS", exc_invalid_tss)
DO_ERROR_CODE_TRAP(X86_TRAP_NP, SIGSEGV, "segment not present", exc_segment_not_present)
DO_ERROR_CODE_TRAP(X86_TRAP_SS, SIGSEGV, "stack segment", exc_stack_segment)
DO_ERROR_CODE_TRAP(X86_TRAP_GP, SIGSEGV, "general protection", exc_general_protection)
DO_ERROR_CODE_TRAP( X86_TRAP_PF, SIGSEGV,  "page fault", exc_page_fault)
DO_ERROR(X86_TRAP_SPURIOUS, SIGSEGV, "spurious interrupt bug", exc_spurious_interrupt_bug)
DO_ERROR(X86_TRAP_MF, SIGFPE, "x87 coprocessor error", exc_coprocessor_error)
DO_ERROR_CODE_TRAP(X86_TRAP_AC, SIGSEGV, "alignment check", exc_alignment_check)
DO_ERROR(X86_TRAP_XF, SIGFPE, "SIMD coprocessor error", exc_simd_coprocessor_error)

extern void idt_setup_traps(void);

void traps_init(void)
{
    /* Setup traps as cpu_init() might #GP */
    idt_setup_traps();
}

