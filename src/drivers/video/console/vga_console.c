
#include <kernel/std/types.h>
#include "kernel/console.h"
#include "asm/memory.h"
#include "asm/io.h"

/*
 * VGA registers data: http://www.osdever.net/FreeVGA/vga/crtcreg.htm#0A
 */

/* VGA data register ports */
#define VGA_CRT_DC  	0x3D5	/* CRT Controller Data Register - color emulation */
#define VGA_CRT_DM  	0x3B5	/* CRT Controller Data Register - mono emulation */

/* VGA index register ports */
#define VGA_CRT_IC  	0x3D4	/* CRT Controller Index - color emulation */
#define VGA_CRT_IM  	0x3B4	/* CRT Controller Index - mono emulation */

/* VGA CRT controller register indices */
#define VGA_CRTC_CURSOR_START	0x0A
#define VGA_CRTC_CURSOR_END	0x0B
#define VGA_CRTC_CURSOR_HI	0x0E
#define VGA_CRTC_CURSOR_LO	0x0F

const static size_t NUM_COLS = 80;
const static size_t NUM_ROWS = 25;

struct Char {
    u8 character;
    u8 color;
};

struct Char* buffer = (struct Char*) (physical_to_virtual(0xb8000));
size_t current_col = 0;
size_t current_row = 0;
u8 current_color = PRINT_COLOR_WHITE | PRINT_COLOR_BLACK << 4;

void vga_console_clear_row(size_t row);
void vga_console_init();
void vga_console_clear();
void vga_console_newline();
void vga_console_send_char(char character);
void vga_console_put_char(struct console *console, char character);
void vga_console_put_string(struct console *console, const char* str);
void vga_console_set_color(u8 foreground, u8 background);
void vga_console_set_cursor_size(u16 from, u16 to);
void vga_console_move_cursor(unsigned short pos);

void vga_console_clear_row(size_t row) {
    struct Char empty = (struct Char) {
            .character =  ' ',
            .color =  current_color,
    };
    size_t initial_col = NUM_COLS * row;

    for (size_t col = 0; col < NUM_COLS; col++) {
        buffer[initial_col + col] = empty;
    }
}

int vga_console_setup(struct console *console) {
    vga_console_clear();
    vga_console_set_cursor_size(0, 12);

    return 0;
}

void vga_console_clear() {
    for (size_t i = 0; i < NUM_ROWS; i++) {
        vga_console_clear_row(i);
    }

    vga_console_move_cursor(0);
}

void vga_console_newline() {
    current_col = 0;

    if (current_row < NUM_ROWS - 1) {
        current_row++;
        return;
    }

    for (size_t row = 1; row < NUM_ROWS; row++) {
        for (size_t col = 0; col < NUM_COLS; col++) {
            struct Char character = buffer[col + NUM_COLS * row];
            buffer[col + NUM_COLS * (row - 1)] = character;
        }
    }

    vga_console_clear_row(NUM_ROWS - 1);
}

void vga_console_send_char(char character) {
    if (character == '\n') {
        vga_console_newline();
        return;
    }

    if (current_col > NUM_COLS) {
        vga_console_newline();
    }

    buffer[current_col + NUM_COLS * current_row] = (struct Char) {
            .character =  (u8) character,
            .color = current_color,
    };

    current_col++;
}

void vga_console_put_char(struct console *console, char character) {
    vga_console_send_char(character);
    vga_console_move_cursor(current_row * NUM_COLS + current_col);
}

void vga_console_put_string(struct console *console, const char* str) {
    for (size_t i = 0; 1; i++) {
        char character = str[i];

        if (character == '\0') {
            break;
        }

        vga_console_send_char(character);
    }

    vga_console_move_cursor(current_row * NUM_COLS + current_col);
}

void vga_console_set_color(u8 foreground, u8 background) {
    current_color = foreground + (background << 4);
}

void vga_console_set_cursor_size(u16 from, u16 to)
{
    u16 curs, cure;

    /* TODO: read original value */
    curs = (curs & 0xc0) | from;
    cure = (cure & 0xe0) | to;

    outb(VGA_CRTC_CURSOR_START, VGA_CRT_IC);
    outb(curs, VGA_CRT_DC);
    outb(VGA_CRTC_CURSOR_END, VGA_CRT_IC);
    outb(cure, VGA_CRT_DC);
}

void vga_console_move_cursor(unsigned short pos)
{
    outb(VGA_CRTC_CURSOR_HI, VGA_CRT_IC);
    outb(((pos >> 8) & 0x00FF), VGA_CRT_DC);
    outb(VGA_CRTC_CURSOR_LO, VGA_CRT_IC);
    outb(pos & 0x00FF, VGA_CRT_DC);
}

const struct console vga_console = {
    .name = "tty",  /* not a real tty, but it could work atm */
    .setup = vga_console_setup,
    .put_char = vga_console_put_char,
    .put_string = vga_console_put_string
};
