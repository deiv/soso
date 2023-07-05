
#pragma once

/*
 * IRQ vector layout.
 *
 * There are 256 IDT entries (per CPU - each entry is 8 bytes):
 *
 *  Vectors   0 ...  31 : system traps and exceptions - hardcoded events
 *  Vectors  32 ... 127 : device interrupts
 *  Vector  128         : legacy int80 syscall interface
 *  Vectors 129 ... LOCAL_TIMER_VECTOR-1
 *  Vectors LOCAL_TIMER_VECTOR ... 255 : special interrupts
 *
 * 64-bit x86 has per CPU IDT tables.
 */

/*
 * IDT vectors usable for external interrupt sources start at 0x20.
 * (0x80 is the syscall vector, 0x30-0x3f are for ISA)
 */
#define FIRST_EXTERNAL_VECTOR		0x20

/*
 * Vectors 0x30-0x3f are used for ISA interrupts.
 *   round up to the next 16-vector boundary
 */
#define ISA_IRQ_VECTOR(irq)		(((FIRST_EXTERNAL_VECTOR + 16) & ~15) + irq)

#define NR_IRQS_LEGACY			16
