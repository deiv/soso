
#pragma once

#include <kernel/std/const.h>
#include <kernel/std/bits.h>
#include <kernel/std/stddef.h>
#include <kernel/lib/find_bit.h>
#include <asm/bitsperlong.h>
#include <asm/bitops.h>

#define BITS_PER_BYTE		8
#define BITS_PER_TYPE(type)	(sizeof(type) * BITS_PER_BYTE)

#define BITS_TO_LONGS(nr)	__KERNEL_DIV_ROUND_UP(nr, BITS_PER_TYPE(long))

/*
 * small_const_nbits(n) is true precisely when it is known at compile-time
 * that BITMAP_SIZE(n) is 1, i.e. 1 <= n <= BITS_PER_LONG. This allows
 * various bit/bitmap APIs to provide a fast inline implementation. Bitmaps
 * of size 0 are very rare, and a compile-time-known-size 0 is most likely
 * a sign of error. They will be handled correctly by the bit/bitmap APIs,
 * but using the out-of-line functions, so that the inline implementations
 * can unconditionally dereference the pointer(s).
 */
#define small_const_nbits(nbits) \
	(__builtin_constant_p(nbits) && (nbits) <= BITS_PER_LONG && (nbits) > 0)

/**
 * find_next_zero_bit - find the next cleared bit in a memory region
 * @addr: The address to base the search on
 * @size: The bitmap size in bits
 * @offset: The bitnumber to start searching at
 *
 * Returns the bit number of the next zero bit
 * If no bits are zero, returns @size.
 */
static inline
unsigned long find_next_zero_bit(const unsigned long *addr, unsigned long size,
                                 unsigned long offset)
{
    if (small_const_nbits(size)) {
        unsigned long val;

        if (unlikely(offset >= size))
            return size;

        val = *addr | ~GENMASK(size - 1, offset);
        return val == ~0UL ? size : ffz(val);
    }

    return _find_next_bit(addr, NULL, size, offset, ~0UL, 0);
}

/* same as for_each_clear_bit() but use bit as value to start with */
#define for_each_clear_bit_from(bit, addr, size) \
	for ((bit) = find_next_zero_bit((addr), (size), (bit));	\
	     (bit) < (size);					\
	     (bit) = find_next_zero_bit((addr), (size), (bit) + 1))
