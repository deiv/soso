
#pragma once

#include <kernel/compiler/compiler.h>
#include <soso/types.h>
#include <asm/alternative.h>
#include <asm/asm.h>

/**
 * fls - find last set bit in word
 * @x: the word to search
 *
 * This is defined in a similar way as the libc and compiler builtin
 * ffs, but returns the position of the most significant set bit.
 *
 * fls(value) returns 0 if value is 0 or the position of the last
 * set bit if value is nonzero. The last (most significant) bit is
 * at position 32.
 */
static __always_inline int fls(unsigned int x)
{
    int r;

    /*
	 * AMD64 says BSRL won't clobber the dest reg if x==0; Intel64 says the
	 * dest reg is undefined if x==0, but their CPU architect says its
	 * value is written to set it to the same as before, except that the
	 * top 32 bits will be cleared.
	 *
	 * We cannot do this on 32 bits because at the very least some
	 * 486 CPUs did not behave this way.
	 */
	asm("bsrl %1,%0"
	    : "=r" (r)
	    : "rm" (x), "0" (-1));

    return r + 1;
}

/**
 * fls64 - find last set bit in a 64-bit word
 * @x: the word to search
 *
 * This is defined in a similar way as the libc and compiler builtin
 * ffsll, but returns the position of the most significant set bit.
 *
 * fls64(value) returns 0 if value is 0 or the position of the last
 * set bit if value is nonzero. The last (most significant) bit is
 * at position 64.
 */

static __always_inline int fls64(__u64 x)
{
	int bitpos = -1;
	/*
	 * AMD64 says BSRQ won't clobber the dest reg if x==0; Intel64 says the
	 * dest reg is undefined if x==0, but their CPU architect says its
	 * value is written to set it to the same as before.
	 */
	asm("bsrq %1,%q0"
	    : "+r" (bitpos)
	    : "rm" (x));
	return bitpos + 1;
}

/*
 * These have to be done with inline assembly: that way the bit-setting
 * is guaranteed to be atomic. All bit operations return 0 if the bit
 * was cleared before the operation and != 0 if it was not.
 *
 * bit 0 is the LSB of addr; bit 32 is the LSB of (addr+1).
 */

#define RLONG_ADDR(x)			 "m" (*(volatile long *) (x))
#define WBYTE_ADDR(x)			"+m" (*(volatile char *) (x))

/*
 * We do the locked ops that don't return the old value as
 * a mask operation on a byte.
 */
#define CONST_MASK_ADDR(nr, addr)	WBYTE_ADDR((void *)(addr) + ((nr)>>3))
#define CONST_MASK(nr)			(1 << ((nr) & 7))


static __always_inline void
set_bit(long nr, volatile unsigned long *addr)
{
    if (__builtin_constant_p(nr)) {
        asm volatile(LOCK_PREFIX "orb %b1,%0"
        : CONST_MASK_ADDR(nr, addr)
        : "iq" (CONST_MASK(nr))
        : "memory");
    } else {
        asm volatile(LOCK_PREFIX __ASM_SIZE(bts) " %1,%0"
        : : RLONG_ADDR(addr), "Ir" (nr) : "memory");
    }
}


/**
 * ffz - find first zero bit in word
 * @word: The word to search
 *
 * Undefined if no zero exists, so code should check against ~0UL first.
 */
static __always_inline unsigned long ffz(unsigned long word)
{
    asm("rep; bsf %1,%0"
            : "=r" (word)
            : "r" (~word));
    return word;
}

/**
 * __ffs - find first set bit in word
 * @word: The word to search
 *
 * Undefined if no bit exists, so code should check against 0 first.
 */
static __always_inline unsigned long __ffs(unsigned long word)
{
    asm("rep; bsf %1,%0"
            : "=r" (word)
            : "rm" (word));
    return word;
}
