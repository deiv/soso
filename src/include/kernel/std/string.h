
#pragma once

#include <kernel/std/types.h>
#include <kernel/std/stdarg.h>

char* itoa(int value, char * buffer, int radix);
size_t strlen(const char *str);

int snprintf(char *buf, size_t size, const char *fmt, ...);
int vsnprintf(char *buf, size_t size, const char *fmt, va_list args);

void *memcpy(void *dest, const void *src, size_t count);
void *memset(void *s, int c, size_t count);
void *memmove(void *dest, const void *src, size_t count);