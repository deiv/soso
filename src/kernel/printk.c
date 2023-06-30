
#include "kernel/std/stdarg.h"
#include "kernel/std/string.h"

#include "kernel/printk.h"
#include "kernel/console.h"

static struct console* console_list = NULL;

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

    for (struct console* current_console = console_list;
         current_console != NULL;
         current_console = current_console->next) {
        current_console->put_string(current_console, buffer);
    }

    return ret;
}

void register_console(struct console *newcon) {

    if (newcon->setup && newcon->setup(newcon) != 0) {
        /* TODO: warn error. We are not buffering, so is that happens, it will gets lost */
        return;
    }

    /*
     * All registered consoles will get messages from us
     */
    newcon->flags |= CONSOLE_ENABLED;

    if (console_list == NULL) {
        console_list = newcon;
        newcon->next = NULL;
        /*
         * First console will be the root one. Easy as we know that the vga is always first.
         */
        newcon->flags |= CONSOLE_CONSDEV;

    } else {
        struct console* next_console = console_list;
        while(next_console->next != NULL) {
            next_console = next_console->next;
        }

        next_console->next = newcon;
    }
}
