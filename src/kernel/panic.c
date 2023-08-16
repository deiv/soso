
#include <kernel/printk.h>
#include <kernel/std/string.h>

/**
 *	panic - halt the system
 *	@fmt: The text string to print
 *
 *	Display a message, then perform cleanups.
 *
 *	This function never returns.
 */
void panic(const char *fmt, ...)
{
    va_list args;
    static char buf[1024];
    long len;

    va_start(args, fmt);
    len = vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    if (len && buf[len - 1] == '\n')
        buf[len - 1] = '\0';

    printk("Kernel panic - not syncing: %s\n", buf);

    for (;;) {

    }
}