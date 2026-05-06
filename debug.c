//
// Created by wright on 3/28/26.
//

#include "debug.h"

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

void debug_msg_ex(const char* filename, i32 line_num, const char* fmt, ...) {
#ifndef NDEBUG
    va_list args;
    va_start(args);

    fprintf(stderr, "[%s, %d] ", filename, line_num);
    vfprintf(stderr, fmt, args);
    va_end(args);
#endif
}

void crash_msg_ex(const char* filename, i32 line_num, const char* fmt, ...) {
    va_list args;
    va_start(args);

    fprintf(stderr, "[%s, %d] ", filename, line_num);
    vfprintf(stderr, fmt, args);
    va_end(args);

    assert(0);
    exit(EXIT_FAILURE);
}
