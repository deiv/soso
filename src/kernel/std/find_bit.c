
#include <kernel/compiler/compiler.h>

#include <kernel/std/math.h>
#include <kernel/std/minmax.h>

#include <kernel/lib/bitmap.h>
#include <kernel/lib/swab.h>

unsigned long _find_next_bit(const unsigned long *addr1,
                             const unsigned long *addr2, unsigned long nbits,
                             unsigned long start, unsigned long invert, unsigned long le)
{
    unsigned long tmp, mask;

    if (unlikely(start >= nbits))
        return nbits;

    tmp = addr1[start / BITS_PER_LONG];
    if (addr2)
        tmp &= addr2[start / BITS_PER_LONG];
    tmp ^= invert;

    /* Handle 1st word. */
    mask = BITMAP_FIRST_WORD_MASK(start);
    if (le)
        mask = swab(mask);

    tmp &= mask;

    start = round_down(start, BITS_PER_LONG);

    while (!tmp) {
        start += BITS_PER_LONG;
        if (start >= nbits)
            return nbits;

        tmp = addr1[start / BITS_PER_LONG];
        if (addr2)
            tmp &= addr2[start / BITS_PER_LONG];
        tmp ^= invert;
    }

    if (le)
        tmp = swab(tmp);

    return min(start + __ffs(tmp), nbits);
}
