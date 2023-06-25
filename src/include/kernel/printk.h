
#pragma once

#include "kernel/std/stdarg.h"

int printk(const char* str, ...);
int vprintk(const char* str, va_list args);
