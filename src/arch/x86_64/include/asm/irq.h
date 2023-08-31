
#pragma once

#include <kernel/compiler/compiler.h>

static __always_inline void native_irq_disable(void)
{
    asm volatile("cli": : :"memory");
}

static __always_inline void native_irq_enable(void)
{
    asm volatile("sti": : :"memory");
}

