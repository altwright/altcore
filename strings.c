//
// Created by wright on 2/18/26.
//

#include "strings.h"

#include <stdarg.h>
#include <stdio.h>
#include <assert.h>
#include <ctype.h>
#include <string.h>

string str_make(Arena *arena, const char *fmt, ...) {
    va_list args_read, args_write;
    va_start(args_read);
    va_start(args_write);

    i32 len = vsnprintf(nullptr, 0, fmt, args_read);
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
        char *new_data = arena_alloc(str->arena, new_cap);
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

string str_dup(Arena *arena, const string *str) {
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
        *c_ptr = (char) tolower(*c_ptr);
    }
}

void str_to_upper(string *str) {
    ARRAY_FOR(c_ptr, str) {
        *c_ptr = (char) toupper(*c_ptr);
    }
}

string_views str_split(Arena *arena, const string_view *str, const char *delimiter) {
    assert(str && delimiter);

    string_views split_views = {arena};
    ARRAY_MAKE(&split_views);

    i64 delim_len = (i64) strlen(delimiter);

    const char *delim_start = nullptr;
    string_view delim_view = {
        str->data,
        str->len,
    };
    const char *prev_start = str->data;

    while (delim_start = str_find_sub(&delim_view, delimiter)
           , delim_start) {
        i64 sub_str_len = (delim_start - prev_start);
        string_view sub_str = {prev_start, sub_str_len};

        ARRAY_PUSH(&split_views, &sub_str);

        str_advance(&delim_view, sub_str_len + delim_len);
        prev_start = delim_start + delim_len;
    }

    if (prev_start < str->data + str->len) {
        string_view final_sub_str = {
            prev_start,
            (str->data + str->len) - prev_start,
        };
        ARRAY_PUSH(&split_views, &final_sub_str);
    }

    return split_views;
}

i64s str_split_idxs(Arena *arena, const string_view *str, const char *delimiter) {
    assert(str);

    i64s idxs = {arena};
    ARRAY_MAKE(&idxs);

    size_t delim_len = strlen(delimiter);

    const char *delim_start = str->data;

    while (delim_start = strstr(delim_start, delimiter), delim_start) {
        i64 delim_idx = delim_start - str->data;
        ARRAY_PUSH(&idxs, &delim_idx);

        delim_start += delim_len;
    }

    return idxs;
}

string str_join(Arena *arena, const string_views *strs, const char *delimiter) {
    string new_str = {arena};

    i64 total_len = 0;

    u64 delim_len = strlen(delimiter);

    ARRAY_FOR(str, strs) {
        total_len += str->len;
    }

    total_len += (i64) delim_len * (strs->len - 1);

    new_str.cap = total_len + 1;

    ARRAY_MAKE(&new_str);
    memset(new_str.data, 0, new_str.cap);

    for (i64 str_idx = 0; str_idx < strs->len; str_idx++) {
        str_append(&new_str, "%s", strs->data[str_idx].data);

        if (str_idx < strs->len - 1) {
            str_append(&new_str, "%s", delimiter);
        }
    }

    return new_str;
}

string str_make_view(Arena *arena, const string_view *view) {
    string str = str_make(arena, "%.*s", view->len, view->data);
    return str;
}

void str_strip(string_view *str) {
    i64 c_idx = 0;
    for (c_idx = 0; c_idx < str->len; c_idx++) {
        if (!isspace(str->data[c_idx])) {
            break;
        }
    }

    str_advance(str, c_idx);

    for (c_idx = str->len - 1; c_idx >= 0; c_idx--) {
        if (!isspace(str->data[c_idx])) {
            break;
        }
    }

    str->len = c_idx + 1;
}

void str_advance(string_view *str, i64 offset) {
    assert(offset >= 0 && offset <= str->len);

    str->data += offset;
    str->len -= offset;
}

const char *str_find_sub(const string_view *haystack, const char *needle) {
    const char *sub_str = nullptr;

    i64 needle_len = (i64) strlen(needle);

    bool found = false;
    i64 c_idx = 0;
    for (c_idx = 0; c_idx < haystack->len - needle_len; c_idx++) {
        const char *current_start = haystack->data + c_idx;

        if (strncmp(current_start, needle, needle_len) == 0) {
            found = true;
            break;
        }
    }

    if (found) {
        sub_str = haystack->data + c_idx;
    }

    return sub_str;
}

void str_replace_at(string *str, i64 section_start_idx, i64 section_len, const char *new_str) {
    assert(
        str
        && new_str
        && section_start_idx >= 0
        && section_start_idx < str->len
        && section_len <= str->len - section_start_idx
    );

    u64 new_str_len = strlen(new_str);

    i64 new_section_len_delta = (i64) new_str_len - section_len;

    i64 new_cap = str->cap;
    while (str->len + new_section_len_delta >= new_cap) {
        new_cap *= 2;
    }

    if (new_cap > str->cap) {
        u8 *new_data = arena_alloc(str->arena, new_cap);
        assert(new_data);
        memcpy(new_data, str->data, str->len * sizeof(char));
        str->data = (char *) new_data;
        str->cap = new_cap;
    }

    if (new_section_len_delta < 0) {
        for (
            i64 c_idx = section_start_idx + section_len;
            c_idx < str->len + new_section_len_delta;
            c_idx++
        ) {
            str->data[c_idx + new_section_len_delta] = str->data[c_idx];
        }
    } else if (new_section_len_delta > 0) {
        for (
            i64 c_idx = str->len - 1;
            c_idx >= section_start_idx + section_len;
            c_idx--
        ) {
            str->data[c_idx + new_section_len_delta] = str->data[c_idx];
        }
    }

    str->data[str->len + new_section_len_delta] = '\0';
    str->len += new_section_len_delta;

    memcpy(str->data + section_start_idx, new_str, new_str_len);
}

string_views str_strs_to_views(Arena *arena, const strings *strs) {
    assert(arena && strs);

    string_views views = {arena, 0, strs->len};
    ARRAY_MAKE(&views);

    ARRAY_FOR(str, strs) {
        string_view view = {
            str->data,
            str->len,
        };

        ARRAY_PUSH(&views, &view);
    }

    return views;
}

strings str_views_to_strs(Arena *arena, const string_views *views) {
    assert(arena && views);

    strings strs = {arena, 0, views->len};
    ARRAY_MAKE(&strs);

    ARRAY_FOR(view, views) {
        string str = str_make_view(arena, view);
        ARRAY_PUSH(&strs, &str);
    }

    return strs;
}
