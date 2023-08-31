
#pragma once

#include <kernel/compiler/compiler.h>

#define __swab64(x) (__u64)__builtin_bswap64((__u64)(x))

static __always_inline unsigned long swab(const unsigned long y)
{
#if __BITS_PER_LONG == 64
    return __swab64(y);
#else
#error bits not supported
#endif
}
