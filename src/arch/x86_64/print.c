
#include <stdarg.h>

#include "print.h"
#include "memory.h"

const static size_t NUM_COLS = 80;
const static size_t NUM_ROWS = 25;

struct Char {
    uint8_t character;
    uint8_t color;
};

struct Char* buffer = (struct Char*) (physical_to_virtual(0xb8000));
size_t col = 0;
size_t row = 0;
uint8_t color = PRINT_COLOR_WHITE | PRINT_COLOR_BLACK << 4;

void clear_row(size_t row) {
    struct Char empty = (struct Char) {
            .character =  ' ',
            .color =  color,
    };

    for (size_t col = 0; col < NUM_COLS; col++) {
        buffer[col + NUM_COLS * row] = empty;
    }
}

void print_clear() {
    for (size_t i = 0; i < NUM_ROWS; i++) {
        clear_row(i);
    }
}

void print_newline() {
    col = 0;

    if (row < NUM_ROWS - 1) {
        row++;
        return;
    }

    for (size_t row = 1; row < NUM_ROWS; row++) {
        for (size_t col = 0; col < NUM_COLS; col++) {
            struct Char character = buffer[col + NUM_COLS * row];
            buffer[col + NUM_COLS * (row - 1)] = character;
        }
    }

    clear_row(NUM_COLS - 1);
}

void print_char(char character) {
    if (character == '\n') {
        print_newline();
        return;
    }

    if (col > NUM_COLS) {
        print_newline();
    }

    buffer[col + NUM_COLS * row] = (struct Char) {
            .character =  (uint8_t) character,
            .color = color,
    };

    col++;
}

void print_str(char* str) {
    for (size_t i = 0; 1; i++) {
        char character = (uint8_t) str[i];

        if (character == '\0') {
            return;
        }

        print_char(character);
    }
}

void print_set_color(uint8_t foreground, uint8_t background) {
    color = foreground + (background << 4);
}

int printf(const char* str, ...)
{
    if(!str)
        return 0;

    va_list	args;
    va_start (args, str);
    size_t i;

    for (i=0; i<strlen(str);i++) {
        switch (str[i]) {
            case '%':
                switch (str[i+1]) {
                    /*** characters ***/
                    case 'c': {
                        char c = va_arg (args, char);
                        print_char(c);
                        i++;
                        break;
                    }

                     /*** address of ***/
                    case 's': {
                        char* c = va_arg (args, char*);
                        print_str(c);
                        i++;
                        break;
                    }

                    /*** integers ***/
                    case 'd':
                    case 'i': {
                        int c = va_arg (args, int);
                        char str[64]={0};
                        itoa(c, str, 10);
                        print_str(str);
                        i++;
                        break;
                    }

                    /*** display in hex ***/
                    case 'X':
                    case 'x': {
                        int c = va_arg (args, int);
                        char str[64]={0};
                        itoa(c, str, 10);
                        print_str(str);
                        i++;
                        break;
                    }

                    default:
                        va_end (args);
                        return 1;
                }

                break;

            default:
                print_char(str[i]);
                break;
        }

    }

    va_end (args);
    return i;
}
