
#pragma once

#include <kernel/boot/boot_parameters.h>

void get_boot_parameters(unsigned long addr_info, struct boot_parameters* boot_parameters);
