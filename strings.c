//
// Created by wright on 2/18/26.
//

#include "strings.h"

#include <stdarg.h>
#include <stdio.h>
#include <assert.h>
#include <ctype.h>
#include <string.h>

string str_make(Arena* arena, const char *fmt, ...) {
    va_list args_read, args_write;
    va_start(args_read);
    va_start(args_write);

    i32 len = vsnprintf(nullptr, 0, fmt, args_read);
    i64 cap = (((2 * (len + 1)  + 3) >> 2) << 2);
    if (cap < 32) {
        cap = 32;
    }

    string str = {
        .len = len,
        .cap = cap,
        .arena = arena
    };

    ARRAY_MAKE(&str);

    i32 written_len = vsnprintf(str.data, len + 1, fmt, args_write);
    assert(written_len == len);

    va_end(args_read);
    va_end(args_write);

    return str;
}

void str_append(string *str, const char *fmt, ...) {
    va_list args_read, args_write;
    va_start(args_read);
    va_start(args_write);

    i32 len = vsnprintf(nullptr, 0, fmt, args_read);

    i64 new_cap = str->cap;
    while ((len + 1) > (new_cap - str->len)) {
        new_cap *= 2;
    }

    if (new_cap > str->cap) {
        char* new_data = arena_alloc(str->arena, new_cap);
        assert(new_data);
        memcpy(new_data, str->data, str->len + 1);
        str->data = new_data;
        str->cap = new_cap;
    }

    i32 written_len = vsnprintf(str->data + str->len, len + 1, fmt, args_write);
    assert(written_len == len);

    str->len += len;

    va_end(args_read);
    va_end(args_write);
}

bool str_empty(const string *str) {
    return ARRAY_EMPTY(str);
}

string str_dup(Arena* arena, const string *str) {
    string new_str = {
        .len = str->len,
        .cap = str->cap,
        .arena = arena,
    };

    new_str.data = arena_alloc(arena, new_str.cap);
    assert(new_str.data);
    memcpy(new_str.data, str->data, str->len + 1);

    return new_str;
}

void str_to_lower(string *str) {
    ARRAY_FOR(c_ptr, str) {
        *c_ptr = (char)tolower(*c_ptr);
    }
}

void str_to_upper(string *str) {
    ARRAY_FOR(c_ptr, str) {
        *c_ptr = (char)toupper(*c_ptr);
    }
}
