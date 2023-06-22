
#include "kernel/console.h"
#include "memory.h"

const static size_t NUM_COLS = 80;
const static size_t NUM_ROWS = 25;

struct Char {
    uint8_t character;
    uint8_t color;
};

struct Char* buffer = (struct Char*) (physical_to_virtual(0xb8000));
size_t current_col = 0;
size_t current_row = 0;
uint8_t current_color = PRINT_COLOR_WHITE | PRINT_COLOR_BLACK << 4;

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

void vga_console_clear() {
    for (size_t i = 0; i < NUM_ROWS; i++) {
        vga_console_clear_row(i);
    }
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

void vga_console_put_char(char character) {
    if (character == '\n') {
        vga_console_newline();
        return;
    }

    if (current_col > NUM_COLS) {
        vga_console_newline();
    }

    buffer[current_col + NUM_COLS * current_row] = (struct Char) {
            .character =  (uint8_t) character,
            .color = current_color,
    };

    current_col++;
}

void vga_console_put_string(char* str) {
    for (size_t i = 0; 1; i++) {
        char character = (uint8_t) str[i];

        if (character == '\0') {
            return;
        }

        vga_console_put_char(character);
    }
}

void vga_console_set_color(uint8_t foreground, uint8_t background) {
    current_color = foreground + (background << 4);
}

const struct console vga_console = {
        .clear = vga_console_clear,
        .put_char = vga_console_put_char,
        .put_string = vga_console_put_string,
        .set_color = vga_console_set_color
};
