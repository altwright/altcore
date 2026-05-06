//
// Created by wright on 3/28/26.
//

#ifndef ALTCORE_DEBUG_H
#define ALTCORE_DEBUG_H

#include "types.h"

#ifndef debug_msg
#define debug_msg(fmt, ...) \
    debug_msg_ex(__FILE__, __LINE__, fmt __VA_OPT__(,) __VA_ARGS__)
#endif

void debug_msg_ex(const char* filename, i32 line_num, const char* fmt, ...);

#ifndef crash_msg
#define crash_msg(fmt, ...) \
    crash_msg_ex(__FILE__, __LINE__, fmt __VA_OPT__(,) __VA_ARGS__)
#endif

void crash_msg_ex(const char* filename, i32 line_num, const char* fmt, ...);

#endif //ALTCORE_DEBUG_H
