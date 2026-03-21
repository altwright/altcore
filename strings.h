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
    const char *data;
    i64 len;
} string_view;

typedef struct STRING_VIEWS_T {
    ARRAY_FIELDS(string_view)
} string_views;

#ifndef SV_FMT
#define SV_FMT "%.*s"
#endif

#ifndef SV_DATA
#define SV_DATA(sv_ptr) (sv_ptr)->len, (sv_ptr)->data
#endif

string str_make(Arena *arena, const char *fmt, ...);

string str_make_view(Arena *arena, const string_view *view);

void str_append(string *str, const char *fmt, ...);

bool str_empty(const string *str);

string str_dup(Arena *arena, const string *str);

void str_to_lower(string *str);

void str_to_upper(string *str);

string_views str_split(Arena *arena, const string_view *view, const char *delimiter);

string str_join(Arena *arena, const string_views *strs, const char *delimiter);

void str_replace_at(string *str, i64 section_start_idx, i64 section_len, const char *new_str);

void str_advance(string_view *str, i64 offset);

void str_strip(string_view *str);

const char* str_find_sub(const string_view* haystack, const char* needle);

string_views str_strs_to_views(Arena *arena, const strings *strs);

strings str_views_to_strs(Arena *arena, const string_views *strs);

#endif //ALTCORE_STRINGS_H
