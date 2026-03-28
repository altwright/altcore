//
// Created by wright on 3/28/26.
//

#include "debug.h"
#include <stdarg.h>
#include <stdio.h>

void debugf(const char* fmt, ...) {
#ifndef NDEBUG
    va_list args;
    va_start(args);

    vfprintf(stderr, fmt, args);
#endif
}
