
#include <stdarg.h>

#include "string.h"

#include "kernel/printk.h"
#include "kernel/console.h"

extern const struct console vga_console;

int printk(const char* str, ...)
{
    if(!str)
        return 0;

    va_list	args;
    va_start (args, str);
    size_t i;

    for (i=0; i<strlen(str);i++) {
        switch (str[i]) {
            case '%':
                switch (str[i+1]) {
                    /*** characters ***/
                    case 'c': {
                        char c = va_arg (args, int);
                        vga_console.put_char(c);
                        i++;
                        break;
                    }

                        /*** address of ***/
                    case 's': {
                        char* c = va_arg (args, char*);
                        vga_console.put_string(c);
                        i++;
                        break;
                    }

                        /*** integers ***/
                    case 'd':
                    case 'i': {
                        int c = va_arg (args, int);
                        char str[64]={0};
                        itoa(c, str, 10);
                        vga_console.put_string(str);
                        i++;
                        break;
                    }

                        /*** display in hex ***/
                    case 'X':
                    case 'x': {
                        int c = va_arg (args, int);
                        char str[64]={0};
                        itoa(c, str, 10);
                        vga_console.put_string(str);
                        i++;
                        break;
                    }

                    default:
                        va_end (args);
                        return 1;
                }

                break;

            default:
                vga_console.put_char(str[i]);
                break;
        }

    }

    va_end (args);
    return i;
}