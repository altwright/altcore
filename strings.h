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

#endif //ALTCORE_STRINGS_H
