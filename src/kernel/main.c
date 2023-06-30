
#include "kernel/console.h"
#include "kernel/printk.h"
#include "kernel/std/string.h"

extern struct console vga_console;
extern struct console* serial8250_console;

void kernel_main(unsigned long addr_mboot_info) {

    /*
     * TODO: initialize the console in a more independent fashion ...
     */
    register_console(&vga_console);
    register_console(serial8250_console);

    printk("Booting 64-bit kernel!\n");

    print_multiboot_info(addr_mboot_info);
}
