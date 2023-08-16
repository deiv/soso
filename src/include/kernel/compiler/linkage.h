
#pragma once

#include <asm/memory.h>

#ifndef __ASSEMBLY__
#include <kernel/compiler/compiler_attributes.h>
#endif

#define __page_aligned_data	__section(".data..page_aligned") __aligned(PAGE_SIZE)
#define __page_aligned_bss	__section(".bss..page_aligned") __aligned(PAGE_SIZE)

/*
 * __ro_after_init is used to mark things that are read-only after init (i.e.
 * after mark_rodata_ro() has been called). These are effectively read-only,
 * but may get written to during init, so can't live in .rodata (via "const").
 */
#ifndef __ro_after_init
#define __ro_after_init __section(".data..ro_after_init")
#endif

#ifdef __cplusplus
#define CPP_ASMLINKAGE extern "C"
#else
#define CPP_ASMLINKAGE
#endif

#ifndef asmlinkage
#define asmlinkage CPP_ASMLINKAGE
#endif

#ifdef __ASSEMBLY__

/* Some toolchains use other characters (e.g. '`') to mark new line in macro */
#ifndef ASM_NL
#define ASM_NL		 ;
#endif

#ifndef __ALIGN
#define __ALIGN		.align 4,0x90
#define __ALIGN_STR	".align 4,0x90"
#endif

#define ALIGN __ALIGN

/* SYM_A_* -- align the symbol? */
#define SYM_A_ALIGN				ALIGN
#define SYM_A_NONE				/* nothing */

/* SYM_L_* -- linkage of symbols */
#define SYM_L_GLOBAL(name)			.globl name
#define SYM_L_WEAK(name)			.weak name
#define SYM_L_LOCAL(name)			/* nothing */

/* SYM_T_NONE -- type used by assembler to mark entries of unknown type */
#ifndef SYM_T_NONE
#define SYM_T_NONE				STT_NOTYPE
#endif

/* SYM_ENTRY -- use only if you have to for non-paired symbols */
#ifndef SYM_ENTRY
#define SYM_ENTRY(name, linkage, align...)		\
    linkage(name) ASM_NL				\
    align ASM_NL					\
    name:
#endif

/* SYM_START -- use only if you have to */
#ifndef SYM_START
#define SYM_START(name, linkage, align...)		\
    SYM_ENTRY(name, linkage, align)
#endif

/* SYM_CODE_START -- use for non-C (special) functions */
#ifndef SYM_CODE_START
#define SYM_CODE_START(name)				\
    SYM_START(name, SYM_L_GLOBAL, SYM_A_ALIGN)
#endif

/* SYM_END -- use only if you have to */
#ifndef SYM_END
#define SYM_END(name, sym_type)				\
    .type name sym_type ASM_NL			\
    .set .L__sym_size_##name, .-name ASM_NL		\
    .size name, .L__sym_size_##name
#endif

/* SYM_CODE_END -- the end of SYM_CODE_START_LOCAL, SYM_CODE_START, ... */
#ifndef SYM_CODE_END
#define SYM_CODE_END(name)				\
    SYM_END(name, SYM_T_NONE)
#endif
#endif
