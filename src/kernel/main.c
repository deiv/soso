
#include <kernel/console.h>
#include <kernel/printk.h>
#include <kernel/std/stddef.h>
#include <kernel/boot/boot_parameters.h>

#include <asm/pic.h>

#include <boot/multiboot.h>

extern struct console vga_console;
extern struct console* serial8250_console;
extern struct legacy_pic legacy_pic;

struct boot_parameters boot_parameters = {
        .cmdline = NULL,
        .bootloader = NULL,
        .load_base_addr = 0,
        .mem_lower = 0,
        .mem_upper = 0,
        .available_memory_map_size = 0,
        .available_memory_map = {0},
        .reserved_memory_map_size = 0,
        .reserved_memory_map = {0},
        .screen_info = {0}
};

void kernel_main(unsigned long addr_mboot_info) {

    /*
     * TODO: initialize the console in a more independent fashion ...
     */
    register_console(&vga_console);
    register_console(serial8250_console);

    printk("Booting 64-bit kernel!\n");

    get_boot_parameters(addr_mboot_info, &boot_parameters);

    printk("Command line: %s\n", boot_parameters.cmdline);
    printk("Boot loader name: %s\n", boot_parameters.bootloader);
    printk("Load base addr: 0x%x\n", boot_parameters.load_base_addr);
    printk("Memory: lower=%dKB, upper=%dKB\n",
           boot_parameters.mem_lower,
           boot_parameters.mem_upper);

    printk("Memory Map:\n");

    for (u32 idx = 0; idx < boot_parameters.available_memory_map_size; idx++) {
        printk("    Available region, addr: 0x%lx, len: 0x%lx\n",
               boot_parameters.available_memory_map[idx].addr,
               boot_parameters.available_memory_map[idx].len);
    }

    for (u32 idx = 0; idx < boot_parameters.reserved_memory_map_size; idx++) {
        printk("    Reserved region, addr: 0x%lx, len: 0x%lx\n",
               boot_parameters.reserved_memory_map[idx].addr,
               boot_parameters.reserved_memory_map[idx].len);
    }

    printk("Display: type=0x%x, addr=0x%x, pitch=%i, width=%i, height=%i, bpp=%i\n",
            boot_parameters.screen_info.type,
            boot_parameters.screen_info.addr,
            boot_parameters.screen_info.pitch,
            boot_parameters.screen_info.width,
            boot_parameters.screen_info.height,
            boot_parameters.screen_info.bpp);

    /*
     * TODO: at some point this need to be platform dependent on x86
     */
    legacy_pic.probe();
    legacy_pic.init(0);
}
