
#pragma once

#include <kernel/compiler/compiler.h>
#include <kernel/printk.h>
#include <kernel/panic.h>

void warn_printk(const char *fmt, ...);

#define WARN(fmt, args...) warn_printk(fmt, args);

#define WARN_ON_TRUE(condition, fmt, args...) ({    \
	int __ret_warn_on = !!(condition);		        \
	if (unlikely(__ret_warn_on))			        \
		warn_printk(fmt, args);                    \
	unlikely(__ret_warn_on);				        \
})

#define BUG() do { \
	printk("BUG: failure at %s:%d/%s()!\n", __FILE__, __LINE__, __func__); \
	barrier_before_unreachable(); \
	panic("BUG!"); \
} while (0)

#define BUG_ON(condition) do { if (unlikely(condition)) BUG(); } while (0)
