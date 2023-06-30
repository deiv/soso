
#pragma once

#include <kernel/std/types.h>

struct screen_info {
    u8 type;
    u64 addr;
    u32 pitch;
    u32 width;      /* for EGA is expressed in character */
    u32 height;     /* for EGA is expressed in character */
    u8 bpp;
};

struct memory_map {
    u64 addr;
    u64 len;
};

#define BOOT_PARAMETERS_MAX_MEMORY_MAP_SIZE 10

struct boot_parameters {
    char* cmdline;
    char* bootloader;
    u32 load_base_addr;
    u32 mem_lower;
    u32 mem_upper;
    u32 available_memory_map_size;
    struct memory_map available_memory_map[BOOT_PARAMETERS_MAX_MEMORY_MAP_SIZE];
    u32 reserved_memory_map_size;
    struct memory_map reserved_memory_map[BOOT_PARAMETERS_MAX_MEMORY_MAP_SIZE];
    struct screen_info screen_info;
};
