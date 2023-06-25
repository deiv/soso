
#include "kernel/std/stdarg.h"
#include "kernel/printk.h"

void warn_printk(const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    vprintk(fmt, args);
    va_end(args);
}
