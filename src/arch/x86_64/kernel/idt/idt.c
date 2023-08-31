
#include <kernel/std/string.h>
#include <kernel/std/stddef.h>
#include <kernel/std/types.h>
#include <kernel/std/array.h>
#include <kernel/bug/runtime_bug.h>
#include <kernel/compiler/linkage.h>
#include <kernel/lib/bitmap.h>
#include <asm/traps.h>
#include <asm/irq_vector.h>

#include "idt_entry.h"

#define IDT_ENTRIES			256
#define NUM_EXCEPTION_VECTORS		32

/*
 * Segment selector values corresponding to the above entries:
 */
#define GDT_ENTRY_KERNEL_CS 2

#define __KERNEL_CS			(GDT_ENTRY_KERNEL_CS*8)
#define __KERNEL_DS			(GDT_ENTRY_KERNEL_DS*8)
#define __USER_DS			(GDT_ENTRY_DEFAULT_USER_DS*8 + 3)
#define __USER_CS			(GDT_ENTRY_DEFAULT_USER_CS*8 + 3)
#define __ESPFIX_SS			(GDT_ENTRY_ESPFIX_SS*8)

/*
 * The index for the tss.ist[] array. The hardware limit is 7 entries.
 */
#define	IST_INDEX_DF		0
#define	IST_INDEX_NMI		1
#define	IST_INDEX_DB		2
#define	IST_INDEX_MCE		3
#define	IST_INDEX_VC		4


struct idt_bits {
    u16		ist	: 3,
            zero	: 5,
            type	: 5,
            dpl	: 2,
            p	: 1;
} __attribute__((packed));

struct idt_data {
    unsigned int	vector;
    unsigned int	segment;
    struct idt_bits	bits;
    const void	*addr;
};

struct gate_struct {
    u16		offset_low;
    u16		segment;
    struct idt_bits	bits;
    u16		offset_middle;
    u32		offset_high;
	u32		reserved;
} __attribute__((packed));

typedef struct gate_struct gate_desc;

#define IDT_TABLE_SIZE		(IDT_ENTRIES * sizeof(gate_desc))

/* Must be page-aligned because the real IDT is used in the cpu entry area */
static gate_desc idt_table[IDT_ENTRIES] __page_aligned_bss;

enum {
    GATE_INTERRUPT = 0xE,
    GATE_TRAP = 0xF,
    GATE_CALL = 0xC,
    GATE_TASK = 0x5,
};

#define G(_vector, _addr, _ist, _type, _dpl, _segment)	\
	{						\
		.vector		= _vector,		\
		.bits.ist	= _ist,			\
		.bits.type	= _type,		\
		.bits.dpl	= _dpl,			\
		.bits.p		= 1,			\
		.addr		= _addr,		\
		.segment	= _segment,		\
	}

#define DPL0		0x0
#define DPL3		0x3

#define DEFAULT_STACK	0

/* Interrupt gate */
#define INTG(_vector, _addr)				\
	G(_vector, _addr, DEFAULT_STACK, GATE_INTERRUPT, DPL0, __KERNEL_CS)


/* System interrupt gate */
#define SYSG(_vector, _addr)				\
	G(_vector, _addr, DEFAULT_STACK, GATE_INTERRUPT, DPL3, __KERNEL_CS)

/*
* Interrupt gate with interrupt stack. The _ist index is the index in
* the tss.ist[] array, but for the descriptor it needs to start at 1.
*/
#define ISTG(_vector, _addr, _ist)			\
	G(_vector, _addr, _ist + 1, GATE_INTERRUPT, DPL0, __KERNEL_CS)

/*
 * The default IDT entries which are set up in trap_init() before
 * cpu_init() is invoked. Interrupt stacks cannot be used at that point and
 * the traps which use them are reinitialized with IST after cpu_init() has
 * set up TSS.
 */
static const struct idt_data def_idts[] = {
    INTG(X86_TRAP_DE,		asm_exc_divide_error),
    ISTG(X86_TRAP_DB,		asm_exc_debug, IST_INDEX_DB), // TODO: early ?
    //TODO: ISTG(X86_TRAP_NMI,		asm_exc_nmi, IST_INDEX_NMI),
    SYSG(X86_TRAP_BP,		asm_exc_int3), // TODO: early ?
    SYSG(X86_TRAP_OF,		asm_exc_overflow),
    INTG(X86_TRAP_BR,		asm_exc_bounds),
    INTG(X86_TRAP_UD,		asm_exc_invalid_op),
    INTG(X86_TRAP_NM,		asm_exc_device_not_available),
    ISTG(X86_TRAP_DF,		asm_exc_double_fault, IST_INDEX_DF),
    INTG(X86_TRAP_OLD_MF,	asm_exc_coproc_segment_overrun),
    INTG(X86_TRAP_TS,		asm_exc_invalid_tss),
    INTG(X86_TRAP_NP,		asm_exc_segment_not_present),
    INTG(X86_TRAP_SS,		asm_exc_stack_segment),
    INTG(X86_TRAP_GP,		asm_exc_general_protection),
    /*
     * TODO: Not possible on 64-bit
     */
    INTG(X86_TRAP_PF,		asm_exc_page_fault), // TODO: early ?
    INTG(X86_TRAP_SPURIOUS, asm_exc_spurious_interrupt_bug),
    INTG(X86_TRAP_MF,		asm_exc_coprocessor_error),
    INTG(X86_TRAP_AC,		asm_exc_alignment_check),
    INTG(X86_TRAP_XF,		asm_exc_simd_coprocessor_error),
};

DECLARE_BITMAP(system_vectors, NR_VECTORS);

extern char irq_entries_start[];

struct desc_ptr {
    unsigned short size;
    unsigned long address;
} __attribute__((packed)) ;

static struct desc_ptr idt_descr __ro_after_init = {
        .size		= IDT_TABLE_SIZE - 1,
        .address	= (unsigned long) idt_table,
};

static inline void init_idt_data(struct idt_data *data, unsigned int n,
                                 const void *addr)
{
    BUG_ON(n > 0xFF);

    memset(data, 0, sizeof(*data));
    data->vector	= n;
    data->addr	= addr;
    data->segment	= __KERNEL_CS;
    data->bits.type	= GATE_INTERRUPT;
    data->bits.p	= 1;
}

static inline void idt_init_desc(gate_desc *gate, const struct idt_data *d)
{
    unsigned long addr = (unsigned long) d->addr;

    gate->offset_low	= (u16) addr;
    gate->segment		= (u16) d->segment;
    gate->bits		= d->bits;
    gate->offset_middle	= (u16) (addr >> 16);

    gate->offset_high	= (u32) (addr >> 32);
	gate->reserved		= 0;

}

static inline void write_idt_entry(gate_desc *idt, int entry, const gate_desc *gate)
{
    memcpy(&idt[entry], gate, sizeof(*gate));
}

static void
idt_setup_from_table(gate_desc *idt, const struct idt_data *t, int size, bool sys)
{
    gate_desc desc;

    for (; size > 0; t++, size--) {
        idt_init_desc(&desc, t);
        write_idt_entry(idt, t->vector, &desc);
        if (sys)
            set_bit(t->vector, system_vectors);
    }
}

static void set_intr_gate(unsigned int n, const void *addr)
{
    struct idt_data data;

    init_idt_data(&data, n, addr);

    idt_setup_from_table(idt_table, &data, 1, false);
}

static __always_inline void load_idt(const struct desc_ptr *dtr)
{
    asm volatile("lidt %0"::"m" (*dtr));
}

/**
 * idt_setup_traps - Initialize the idt table with default traps
 */
void idt_setup_traps(void)
{
    idt_setup_from_table(idt_table, def_idts, ARRAY_SIZE(def_idts), true);

    int i = FIRST_EXTERNAL_VECTOR;
    void *entry;

    for_each_clear_bit_from(i, system_vectors, FIRST_SYSTEM_VECTOR) {
        entry = irq_entries_start + IDT_ALIGN * (i - FIRST_EXTERNAL_VECTOR);
        set_intr_gate(i, entry);
    }

    load_idt(&idt_descr);
}
