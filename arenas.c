//
// Created by wright on 2/18/26.
//

#include "arenas.h"

#include <assert.h>
#include "malloc.h"

Arena arena_make(i64 initial_cap) {
    Arena arena = {};

    assert(initial_cap > 0);

    arena.buffer = alt_calloc(1, sizeof(ArenaBuffer));
    assert(arena.buffer);

    // Align to 32-bit
    initial_cap = ((initial_cap + 3) >> 2) << 2;
    arena.buffer->cap = initial_cap;
    arena.buffer->data = alt_calloc(arena.buffer->cap, sizeof(u8));
    assert(arena.buffer->data);

    int res = mtx_init(&arena.lock, mtx_plain);
    assert(res == thrd_success);

    return arena;
}

void arena_free(Arena *arena) {
    assert(arena);

    int res = mtx_lock(&arena->lock);
    assert(res == thrd_success);

    ArenaBuffer* current_buffer = arena->buffer;

    while (current_buffer) {
        if (current_buffer->data) {
            alt_free(current_buffer->data);
            current_buffer->data = nullptr;
            current_buffer->cap = 0;
        }

        ArenaBuffer* next_buffer = current_buffer->next;
        alt_free(current_buffer);
        current_buffer = next_buffer;
    }

    arena->buffer = nullptr;

    res = mtx_unlock(&arena->lock);
    assert(res == thrd_success);

    mtx_destroy(&arena->lock);
}

void *arena_alloc(Arena *arena, i64 size) {
    if (!arena || !arena->buffer || size <= 0) {
        return nullptr;
    }

    int res = mtx_lock(&arena->lock);
    assert(res == thrd_success);

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
            ArenaBuffer* next_buffer = alt_calloc(1, sizeof(ArenaBuffer));
            assert(next_buffer);
            next_buffer->cap = aligned_size > current_buffer->cap ? aligned_size : current_buffer->cap;
            next_buffer->data = alt_calloc(next_buffer->cap, sizeof(u8));
            current_buffer->next = next_buffer;
        }
        current_buffer = current_buffer->next;
        current_buffer_offset = 0;
    }

    u8* new_alloc = current_buffer->data + current_buffer_offset;
    arena->offset += aligned_size;

    res = mtx_unlock(&arena->lock);
    assert(res == thrd_success);

    return new_alloc;
}
