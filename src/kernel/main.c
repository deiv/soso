
#include "kernel/console.h"
#include "kernel/printk.h"

extern const struct console vga_console;

void kernel_main(unsigned long addr_mboot_info) {

    /*
     * TODO: initialize the console ...
     */
    vga_console.init();
    vga_console.set_color(PRINT_COLOR_YELLOW, PRINT_COLOR_BLACK);


    printk("Booting 64-bit kernel!\n");

    print_multiboot_info(addr_mboot_info);
}
