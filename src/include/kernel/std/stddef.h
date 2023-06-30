
#pragma once

#undef NULL
#define NULL ((void *)0)

enum {
    false	= 0,
    true	= 1
};

#undef offsetof
#define offsetof(TYPE, MEMBER)	__builtin_offsetof(TYPE, MEMBER)
