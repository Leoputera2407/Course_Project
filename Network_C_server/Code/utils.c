#include "utils.h"
#include <stdio.h>
#include <unistd.h>
#include <stdarg.h>

char unbuf_printf_buf[BUF_MAXLEN];

void unbuf_printf(int fildes, const char* format, ...) {
     va_list a_list;
    va_start(a_list, format);
    int chars_printed = vsnprintf(unbuf_printf_buf, BUF_MAXLEN, format, a_list) + 1; // include null byte.
    if(!write(fildes, unbuf_printf_buf, chars_printed))
        chars_printed = 0;
    va_end(a_list);
}


