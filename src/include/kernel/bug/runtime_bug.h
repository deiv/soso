
#pragma once

#include <kernel/compiler/compiler.h>

void warn_printk(const char *fmt, ...);

#define WARN(fmt, args...) warn_printk(fmt, args);

#define WARN_ON_TRUE(condition, fmt, args...) ({    \
	int __ret_warn_on = !!(condition);		        \
	if (unlikely(__ret_warn_on))			        \
		warn_printk(fmt, args);                    \
	unlikely(__ret_warn_on);				        \
})
