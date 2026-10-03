//
// Created by wright on 2/18/26.
//

#ifndef ALTCORE_STRINGS_H
#define ALTCORE_STRINGS_H

#include "types.h"
#include "arenas.h"

typedef struct STRING_T {
    ARRAY_FIELDS(char)
} string;

typedef struct STRINGS_T {
    ARRAY_FIELDS(string)
} strings;

typedef struct STRING_VIEW_T {
    const char *start;
    i64 len;
} string_view;

typedef struct STRING_VIEWS_T {
    ARRAY_FIELDS(string_view)
} string_views;

string string_make(Arena *arena, const char *fmt, ...);

bool string_empty(const string *str);

void string_push(string *str, const char *fmt, ...);

string string_dup(Arena *arena, const string *str);

void string_del(string *str, i64 start_idx, i64 num_chars);

void string_put(string *str, i64 start_idx, const char *fmt, ...);

#ifndef STRING_FOR
#define STRING_FOR(ptr_var, string_ptr) \
    for ( \
        const char *ptr_var = (string_ptr)->data; \
        ptr_var && (ptr_var < (string_ptr)->data + (string_ptr)->len); \
        ptr_var = utf8_next(ptr_var, ((string_ptr)->data + (string_ptr)->len) - ptr_var) \
    )
#endif

#ifndef STRING_VIEW_FOR
#define STRING_VIEW_FOR(ptr_var, string_view_ptr) \
    for ( \
        const char *ptr_var = (string_view_ptr)->start; \
        ptr_var && (ptr_var < (string_view_ptr)->start + (string_view_ptr)->len); \
        ptr_var = utf8_next(ptr_var, ((string_view_ptr)->start + (string_view_ptr)->len) - ptr_var) \
    )
#endif

const char *utf8_next(const char *current, i64 max_bytes);

i64 utf8_size(const char *current, i64 max_bytes);

u32 utf8_to_unicode(const char *current, i64 max_bytes);

#endif //ALTCORE_STRINGS_H
