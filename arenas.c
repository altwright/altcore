//
// Created by wright on 2/18/26.
//

#include "arenas.h"

#include <assert.h>
#include <stdlib.h>

Arena arena_make(i64 initial_cap) {
    Arena arena = {};

    assert(initial_cap > 0);

    arena.buffer = calloc(1, sizeof(ArenaBuffer));
    assert(arena.buffer);

    // Align to 32-bit
    initial_cap = ((initial_cap + 3) >> 2) << 2;
    arena.buffer->cap = initial_cap;
    arena.buffer->data = calloc(arena.buffer->cap, sizeof(u8));
    assert(arena.buffer->data);

    return arena;
}

void arena_free(Arena *arena) {
    assert(arena);
    ArenaBuffer* current_buffer = arena->buffer;

    while (current_buffer) {
        if (current_buffer->data) {
            free(current_buffer->data);
            current_buffer->data = nullptr;
            current_buffer->cap = 0;
        }

        ArenaBuffer* next_buffer = current_buffer->next;
        free(current_buffer);
        current_buffer = next_buffer;
    }

    arena->buffer = nullptr;
}

void *arena_alloc(Arena *arena, i64 size) {
    if (!arena || !arena->buffer) {
        return nullptr;
    }

    if (size <= 0) {
        return nullptr;
    }

    i64 aligned_size = ((size + 3) >> 2) << 2;
    i64 current_buffer_offset = arena->offset;

    ArenaBuffer* current_buffer = arena->buffer;
    while (current_buffer_offset > current_buffer->cap) {
        current_buffer_offset -= current_buffer->cap;
        assert(current_buffer->next);
        current_buffer = current_buffer->next;
    }

    while (aligned_size >= (current_buffer->cap - current_buffer_offset)) {
        if (!current_buffer->next) {
            ArenaBuffer* next_buffer = calloc(1, sizeof(ArenaBuffer));
            assert(next_buffer);
            next_buffer->cap = aligned_size > current_buffer->cap ? aligned_size : current_buffer->cap;
            next_buffer->data = calloc(next_buffer->cap, sizeof(u8));
            current_buffer->next = next_buffer;
        }
        current_buffer = current_buffer->next;
        current_buffer_offset = 0;
    }

    u8* new_alloc = current_buffer->data + current_buffer_offset;
    arena->offset += aligned_size;

    return new_alloc;
}
