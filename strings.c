//
// Created by wright on 2/18/26.
//

#include "strings.h"

#include <assert.h>
#include <uchar.h>
#include <string.h>
#include "maths.h"
#include "debug.h"

#define STB_SPRINTF_IMPLEMENTATION
#include "libs/stb_sprintf.h"

string string_make(Arena *arena, const char *fmt, ...) {
    va_list args_read = {}, args_write = {};
    va_start(args_read);
    va_start(args_write);

    i32 len = stbsp_vsnprintf(nullptr, 0, fmt, args_read);
    i64 cap = (((2 * (len + 1) + 3) >> 2) << 2);
    if (cap < 32) {
        cap = 32;
    }

    string str = {
        .len = len,
        .cap = cap,
        .arena = arena
    };

    ARRAY_MAKE(&str);

    i32 written_len = stbsp_vsnprintf(str.data, len + 1, fmt, args_write);
    assert(written_len == len);

    va_end(args_read);
    va_end(args_write);

    return str;
}

void string_push(string *str, const char *fmt, ...) {
    va_list args_read = {}, args_write = {};
    va_start(args_read);
    va_start(args_write);

    i32 len = stbsp_vsnprintf(nullptr, 0, fmt, args_read);

    i64 new_cap = str->cap;
    while ((len + 1) > (new_cap - str->len)) {
        new_cap *= 2;
    }

    if (new_cap > str->cap) {
        char *new_data = arena_alloc(str->arena, new_cap);
        memcpy(new_data, str->data, str->len + 1);
        str->data = new_data;
        str->cap = new_cap;
    }

    i32 written_len = stbsp_vsnprintf(str->data + str->len, len + 1, fmt, args_write);
    if (written_len != len) {
        crash_msg("Written length %d does not match expected length %d", written_len, len);
    }

    str->len += len;

    va_end(args_read);
    va_end(args_write);
}

bool string_empty(const string *str) {
    return ARRAY_EMPTY(str);
}

string string_dup(Arena *arena, const string *str) {
    string new_str = {
        .len = str->len,
        .cap = str->cap,
        .arena = arena,
    };

    new_str.data = arena_alloc(arena, new_str.cap);
    memcpy(new_str.data, str->data, str->len + 1);

    return new_str;
}

void string_del(string *str, i64 start_idx, i64 num_chars) {
    if (start_idx < 0 || num_chars < 0) {
        crash_msg("Invalid start_idx %d or num_chars %d\n", start_idx, num_chars);
    }

    if (start_idx >= str->len) {
        start_idx = str->len - 1;
    }

    if (start_idx + num_chars > str->len) {
        num_chars = str->len - start_idx;
    }

    for (i64 c_idx = start_idx + num_chars; c_idx <= str->len; c_idx++) {
        str->data[c_idx - num_chars] = str->data[c_idx];
    }

    str->len -= num_chars;
    str->data[str->len] = '\0';
}

void string_put(string *str, i64 start_idx, const char *fmt, ...) {
    va_list args_read = {}, args_write = {};
    va_start(args_read);
    va_start(args_write);

    i32 len = stbsp_vsnprintf(nullptr, 0, fmt, args_read);

    i64 new_cap = str->cap;
    while ((len + 1) > (new_cap - str->len)) {
        new_cap *= 2;
    }

    if (new_cap > str->cap) {
        char *new_data = arena_alloc(str->arena, new_cap);
        memcpy(new_data, str->data, str->len + 1);
        str->data = new_data;
        str->cap = new_cap;
    }

    for (i64 c_idx = str->len; c_idx >= start_idx; c_idx--) {
        str->data[c_idx + len] = str->data[c_idx];
    }

    i32 written_len = stbsp_vsnprintf(str->data + start_idx, len, fmt, args_write);
    if (written_len != len) {
        crash_msg("Written length %d does not match expected length %d", written_len, len);
    }

    str->len += len;
    str->data[str->len] = '\0';

    va_end(args_read);
    va_end(args_write);
}

const char *utf8_next(const char *current, i64 max_bytes) {
    i64 remaining_bytes = max_bytes;
    auto utf8 = (const char8_t *) current;

    do {
        utf8++;
        remaining_bytes--;
        if (remaining_bytes <= 0) {
            return nullptr;
        }
    } while ((*utf8 & 0xC0) == 0x80);

    return (const char *) utf8;
}

i64 utf8_size(const char *current, i64 max_bytes) {
    i64 size = 0;
    auto utf8 = (const char8_t *) current;

    max_bytes = CLAMP(max_bytes, 0, 4);

    for (i64 byte_idx = 0; byte_idx < max_bytes; byte_idx++) {
        utf8++;
        size++;
        if ((*utf8 & 0xC0) != 0x80) {
            break;
        }
    }

    return size;
}

u32 utf8_to_unicode(const char *current, i64 max_bytes) {
    u32 unicode = 0;

    auto utf8 = (const char8_t *) current;

    if (utf8[0] < 0x80) {
        unicode = utf8[0];
    } else if ((utf8[0] & 0xE0) == 0xC0) {
        unicode = ((utf8[0] & 0x1F) << 6) | (utf8[1] & 0x3F);
    } else if ((utf8[0] & 0xF0) == 0xE0) {
        unicode = ((utf8[0] & 0x0F) << 12) | ((utf8[1] & 0x3F) << 6) | (utf8[2] & 0x3F);
    } else if ((utf8[0] & 0xF8) == 0xF0) {
        unicode = ((utf8[0] & 0x07) << 18) | ((utf8[1] & 0x3F) << 12) | ((utf8[2] & 0x3F) << 6) | (utf8[3] & 0x3F);
    } else {
        unicode = 0xFFFD;
    }

    return unicode;
}
