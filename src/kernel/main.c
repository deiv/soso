
#include "print.h"

void kernel_main(unsigned long addr_mboot_info) {
    print_clear();
    print_set_color(PRINT_COLOR_YELLOW, PRINT_COLOR_BLACK);
    print_str("Booting 64-bit kernel!\n");

    print_multiboot_info(addr_mboot_info);
}
