
#pragma once

#include <asm/bitsperlong.h>
#include <kernel/std/bitops.h>

#define BITMAP_FIRST_WORD_MASK(start) (~0UL << ((start) & (BITS_PER_LONG - 1)))

#define DECLARE_BITMAP(name,bits) \
	unsigned long name[BITS_TO_LONGS(bits)]
