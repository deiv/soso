
#pragma once

#include <soso/types.h>
#include <kernel/std/bitops.h>

#ifndef __ASSEMBLY__

typedef __s8  s8;
typedef __u8  u8;
typedef __s16 s16;
typedef __u16 u16;
typedef __s32 s32;
typedef __u32 u32;
typedef __s64 s64;
typedef __u64 u64;

typedef _Bool bool;

typedef __kernel_size_t	 size_t;
typedef __kernel_ssize_t ssize_t;
typedef __kernel_ptrdiff_t	ptrdiff_t;

#define DECLARE_BITMAP(name,bits) \
	unsigned long name[BITS_TO_LONGS(bits)]

#endif

