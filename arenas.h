//
// Created by wright on 2/18/26.
//

#ifndef ALTCORE_ARENAS_H
#define ALTCORE_ARENAS_H

#include "types.h"

struct ARENA_T;
typedef struct ARENA_T Arena;

Arena* arena_make(i64 initial_cap);

void arena_free(Arena *arena);

void *arena_alloc(Arena *arena, i64 size);

void arena_reset(Arena *arena);

#endif //ALTCORE_ARENAS_H
