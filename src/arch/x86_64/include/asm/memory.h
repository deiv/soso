/**
 * @file memory.h
 *
 * @brief Definitions relating to memory addresses and paging.
 */

#pragma once

/**
 * @brief The VMA offset for the kernel.
 *
 * @details Kernel code is loaded into a physical memory space which
 * is memory mapped into virtual addresses with this offset added.
 */
#define KERNEL_OFFSET 0xffffffff80000000

/**
 * @brief The size of a page of memory.
 */
#define PAGE_SIZE 0x1000

/**
 * @brief Flag to indicate that a page is preset.
 */
#define PAGE_PRESENT (1 << 0)

/**
 * @brief Flag to indicate that a page is writable.
 */
#define PAGE_WRITABLE (1 << 1)

/**
 * @brief Converts a virtual memory address into a physical memory
 * address.
 */
#define virtual_to_physical(addr) (addr - KERNEL_OFFSET)

/**
 * @brief Converts a physical memory address into a virtual memory
 * address.
 */
#define physical_to_virtual(addr) (addr + KERNEL_OFFSET)
