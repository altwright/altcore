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

void string_cat(string *str, const char *fmt, ...);

bool string_empty(const string *str);

string string_dup(Arena *arena, const string *str);

#ifndef STRING_FOR
#define STRING_FOR(ptr_var, string_ptr) \
    for ( \
        const char *ptr_var = (string_ptr)->data; \
        ptr_var && (ptr_var < (string_ptr)->data + (string_ptr)->len); \
        ptr_var = utf8_next(ptr_var, (string_ptr)->len - (ptr_var - (string_ptr)->data)) \
    )
#endif

#ifndef STRING_VIEW_FOR
#define STRING_VIEW_FOR(ptr_var, string_view_ptr) \
    for ( \
        const char *ptr_var = (string_view_ptr)->start; \
        ptr_var && (ptr_var < (string_view_ptr)->start + (string_view_ptr)->len); \
        ptr_var = utf8_next(ptr_var, (string_view_ptr)->len - (ptr_var - (string_view_ptr)->start)) \
    )
#endif

const char *utf8_next(const char *current, i64 max_bytes);

#endif //ALTCORE_STRINGS_H
