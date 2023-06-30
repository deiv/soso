
#include "boot/multiboot.h"
#include "boot/multiboot2.h"

#include "kernel/printk.h"
#include "kernel/boot/boot_parameters.h"
#include "kernel/bug/runtime_bug.h"

void get_boot_parameters(unsigned long addr_info, struct boot_parameters* boot_parameters)
{
    struct multiboot_tag *tag;

    for (tag = (struct multiboot_tag *) (((void*)addr_info) + 8);
         tag->type != MULTIBOOT_TAG_TYPE_END;
         tag = (struct multiboot_tag *) ((multiboot_uint8_t *) tag
                                         + ((tag->size + 7) & ~7))) {

        switch (tag->type) {
            case MULTIBOOT_TAG_TYPE_CMDLINE:
                boot_parameters->cmdline = ((struct multiboot_tag_string *) tag)->string;
                break;

            case MULTIBOOT_TAG_TYPE_BOOT_LOADER_NAME:
                boot_parameters->bootloader = ((struct multiboot_tag_string *) tag)->string;
                break;

            case MULTIBOOT_TAG_TYPE_LOAD_BASE_ADDR:
                boot_parameters->load_base_addr = ((struct multiboot_tag_load_base_addr *) tag)->load_base_addr;
                break;

            case MULTIBOOT_TAG_TYPE_BASIC_MEMINFO:
                boot_parameters->mem_lower = ((struct multiboot_tag_basic_meminfo *) tag)->mem_lower;
                boot_parameters->mem_upper = ((struct multiboot_tag_basic_meminfo *) tag)->mem_upper;
                break;

            case MULTIBOOT_TAG_TYPE_MMAP: {
                multiboot_memory_map_t *mmap;

                for (mmap = ((struct multiboot_tag_mmap *) tag)->entries;
                     (multiboot_uint8_t *) mmap
                     < (multiboot_uint8_t *) tag + tag->size;
                     mmap = (multiboot_memory_map_t *)
                             ((unsigned long) mmap
                              + ((struct multiboot_tag_mmap *) tag)->entry_size)) {

                    if (mmap->type == MULTIBOOT_MEMORY_AVAILABLE) {
                        if (WARN_ON_TRUE(
                                boot_parameters->available_memory_map_size >= BOOT_PARAMETERS_MAX_MEMORY_MAP_SIZE,
                                "boot_parameters, reached max available memory map size: %i\n",
                                BOOT_PARAMETERS_MAX_MEMORY_MAP_SIZE)) {
                            continue;
                        }

                        boot_parameters->
                                available_memory_map[boot_parameters->available_memory_map_size]
                                .addr = mmap->addr;
                        boot_parameters->
                                available_memory_map[boot_parameters->available_memory_map_size]
                                .len = mmap->len;
                        boot_parameters->available_memory_map_size++;

                    } else if (mmap->type == MULTIBOOT_MEMORY_RESERVED) {
                        if (WARN_ON_TRUE(
                                boot_parameters->reserved_memory_map_size >= BOOT_PARAMETERS_MAX_MEMORY_MAP_SIZE,
                                "boot_parameters, reached max reserved memory map size: %i\n",
                                BOOT_PARAMETERS_MAX_MEMORY_MAP_SIZE)) {
                            continue;
                        }

                        boot_parameters->
                                reserved_memory_map[boot_parameters->reserved_memory_map_size]
                                .addr = mmap->addr;
                        boot_parameters->
                                reserved_memory_map[boot_parameters->reserved_memory_map_size]
                                .len = mmap->len;
                        boot_parameters->reserved_memory_map_size++;
                    } else {
                        WARN("boot_parameters, please add unsupported mmap->type %i\n", mmap->type);
                    }
                    /*
                    printk(" base_addr = 0x%x%x,"
                            " length = 0x%x%x, type = 0x%x\n",
                            (unsigned) (mmap->addr >> 32),
                            (unsigned) (mmap->addr & 0xffffffff),
                            (unsigned) (mmap->len >> 32),
                            (unsigned) (mmap->len & 0xffffffff),
                            (unsigned) mmap->type);*/
                }
            }
                break;

            case MULTIBOOT_TAG_TYPE_FRAMEBUFFER: {

                    struct screen_info* screen_info = &boot_parameters->screen_info;

                    multiboot_uint32_t color;
                    unsigned i;
                    struct multiboot_tag_framebuffer *tagfb
                            = (struct multiboot_tag_framebuffer *) tag;
                    void *fb = (void *) (unsigned long) tagfb->common.framebuffer_addr;

                    screen_info->type = tagfb->common.framebuffer_type;
                    screen_info->addr = tagfb->common.framebuffer_addr;
                    screen_info->pitch = tagfb->common.framebuffer_pitch;
                    screen_info->width = tagfb->common.framebuffer_width;
                    screen_info->height = tagfb->common.framebuffer_height;
                    screen_info->bpp = tagfb->common.framebuffer_bpp;
/*
                    switch (tagfb->common.framebuffer_type) {

                        case MULTIBOOT_FRAMEBUFFER_TYPE_INDEXED: {
                            unsigned best_distance, distance;
                            struct multiboot_color *palette;

                            palette = tagfb->framebuffer_palette;

                            color = 0;
                            best_distance = 4*256*256;

                            for (i = 0; i < tagfb->framebuffer_palette_num_colors; i++)
                            {
                                distance = (0xff - palette[i].blue)
                                           * (0xff - palette[i].blue)
                                           + palette[i].red * palette[i].red
                                           + palette[i].green * palette[i].green;
                                if (distance < best_distance)
                                {
                                    color = i;
                                    best_distance = distance;
                                }
                            }


                        }
                            break;

                        case MULTIBOOT_FRAMEBUFFER_TYPE_RGB:
                            color = ((1 << tagfb->framebuffer_blue_mask_size) - 1)
                                    << tagfb->framebuffer_blue_field_position;
                            break;

                        case MULTIBOOT_FRAMEBUFFER_TYPE_EGA_TEXT:
                            color = '\\' | 0x0100;
                            break;

                        default:
                            color = 0xffffffff;
                            break;
                    }

                    for (i = 0; i < tagfb->common.framebuffer_width
                                && i < tagfb->common.framebuffer_height; i++)
                    {
                        switch (tagfb->common.framebuffer_bpp)
                        {
                            case 8:
                            {
                                multiboot_uint8_t *pixel = fb
                                                           + tagfb->common.framebuffer_pitch * i + i;
                                *pixel = color;
                            }
                                break;
                            case 15:
                            case 16:
                            {
                                multiboot_uint16_t *pixel
                                        = fb + tagfb->common.framebuffer_pitch * i + 2 * i;
                                *pixel = color;
                            }
                                break;
                            case 24:
                            {
                                multiboot_uint32_t *pixel
                                        = fb + tagfb->common.framebuffer_pitch * i + 3 * i;
                                *pixel = (color & 0xffffff) | (*pixel & 0xff000000);
                            }
                                break;

                            case 32:
                            {
                                multiboot_uint32_t *pixel
                                        = fb + tagfb->common.framebuffer_pitch * i + 4 * i;
                                *pixel = color;
                            }
                                break;
                        }
                    }*/
                    break;
                }

            /*case MULTIBOOT_TAG_TYPE_MODULE:
                printk("Module at 0x%x-0x%x. Command line %s\n",
                        ((struct multiboot_tag_module *) tag)->mod_start,
                        ((struct multiboot_tag_module *) tag)->mod_end,
                        ((struct multiboot_tag_module *) tag)->cmdline);
                break;

              case MULTIBOOT_TAG_TYPE_BOOTDEV:
                printk("Boot device 0x%x,%d,%d\n",
                        ((struct multiboot_tag_bootdev *) tag)->biosdev,
                        ((struct multiboot_tag_bootdev *) tag)->slice,
                        ((struct multiboot_tag_bootdev *) tag)->part);
                break;
            */

            default:
                printk("Tag 0x%x, %i\n", tag->type, tag->type);
                break;
        }
    }
}
