//
// Created by wright on 2/18/26.
//

#ifndef ALTCORE_ARENAS_H
#define ALTCORE_ARENAS_H

#include <threads.h>

#include "types.h"

typedef struct ARENA_BUFFER_T {
    u8 *data;
    i64 cap;
    struct ARENA_BUFFER_T *next;
} ArenaBuffer;

typedef struct ARENA_T {
    ArenaBuffer *buffer;
    i64 offset;
    mtx_t lock;
} Arena;

Arena arena_make(i64 initial_cap);

void arena_free(Arena *arena);

void *arena_alloc(Arena *arena, i64 size);

#endif //ALTCORE_ARENAS_H
