//
// Created by wright on 2/18/26.
//

#include "types.h"

#include <string.h>

#include "assert.h"
#include "arenas.h"

void array_alloc(
    void **data_ptr,
    i64 *len,
    i64 *cap,
    i64 elem_size,
    struct ARENA_T *arena
) {
    if (*len < 0) {
        *len = 0;
    }

    if (*len > *cap) {
        *cap = *len;
    }

    if (*cap <= 0) {
        *cap = 8;
    }

    *data_ptr = arena_alloc(arena, (*cap) * elem_size);
    assert(*data_ptr);
    memset(*data_ptr, 0, (*cap) * elem_size);
}

void array_push(
    void **data_ptr,
    i64 *len,
    i64 *cap,
    i64 elem_size,
    const void *new_elem,
    struct ARENA_T *arena
) {
    assert(*len >= 0);
    assert(*len <= *cap);

    if (*len == *cap) {
        i64 new_cap = 2 * (*cap);
        void *new_data = arena_alloc(arena, new_cap * elem_size);
        assert(new_data);
        memcpy(new_data, *data_ptr, (*len) * elem_size);

        *data_ptr = new_data;
        *cap = new_cap;
    }

    u8 *data_bytes = *data_ptr;
    u8 *new_elem_bytes = data_bytes + (elem_size * (*len));
    memcpy(new_elem_bytes, new_elem, elem_size);

    (*len)++;
}

void array_extend(
    void **data_ptr,
    i64 *len,
    i64 *cap,
    i64 elem_size,
    struct ARENA_T* arena
) {
    assert(*data_ptr);

    i64 new_cap = 2 * (*cap);

    void *new_data = arena_alloc(arena, new_cap * elem_size);
    assert(new_data);

    memcpy(new_data, *data_ptr, (*len) * elem_size);

    *data_ptr = new_data;
    *cap = new_cap;
}
