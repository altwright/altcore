//
// Created by wright on 2/18/26.
//

#ifndef ALTCORE_STRINGS_H
#define ALTCORE_STRINGS_H

#include "types.h"
#include "arenas.h"

typedef struct STRING_T {
    ARRAY_TYPE(char)
} string;

typedef struct STRING_VIEW_T {
    const char* data;
    i64 len;
} string_view;

string str_make(Arena *arena, const char *fmt, ...);

void str_append(string *str, const char *fmt, ...);

bool str_empty(const string *str);

string str_dup(Arena *arena, const string *str);

void str_to_lower(string *str);

void str_to_upper(string *str);

#endif //ALTCORE_STRINGS_H
