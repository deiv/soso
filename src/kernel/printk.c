
#include "kernel/std/stdarg.h"
#include "kernel/std/string.h"

#include "kernel/printk.h"
#include "kernel/console.h"

extern const struct console vga_console;

int printk(const char* fmt, ...)
{
    va_list args;
    
    va_start(args, fmt);
    int ret = vprintk(fmt, args);
    va_end(args);

    return ret;
}

int vprintk(const char* fmt, va_list args)
{
    char buffer[4096];
    int ret = 0;

    if(!fmt)
        return ret;

    ret = vsnprintf(buffer, 4096, fmt, args);

    if (ret < 0)
        return ret;

    vga_console.put_string(buffer);

    return ret;
}
