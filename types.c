//
// Created by wright on 2/18/26.
//

#include "types.h"

#include <string.h>

#include "assert.h"
#include "arenas.h"

void *kNullPtr = nullptr;

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
    i64 data_elem_size,
    i64 new_elem_size,
    const void *new_elem,
    struct ARENA_T *arena
) {
    assert(data_elem_size == new_elem_size);
    assert(*len >= 0);
    assert(*len <= *cap);

    if (*len == *cap) {
        array_extend(data_ptr, len, cap, data_elem_size, arena);
    }

    u8 *data_bytes = *data_ptr;
    u8 *new_elem_bytes = data_bytes + (new_elem_size * (*len));
    memcpy(new_elem_bytes, new_elem, new_elem_size);

    (*len)++;
}

void array_extend(
    void **data_ptr,
    const i64 *len,
    i64 *cap,
    i64 elem_size,
    struct ARENA_T *arena
) {
    assert(*data_ptr);

    i64 new_cap = 2 * (*cap);

    void *new_data = arena_alloc(arena, new_cap * elem_size);
    assert(new_data);

    memcpy(new_data, *data_ptr, (*len) * elem_size);

    *data_ptr = new_data;
    *cap = new_cap;
}

fx12 fx12_mul(fx12 left, fx12 right) {
    return (fx12){ (i32)(((i64)left.data * (i64)right.data) >> 12)};
}

fx12 fx12_div(fx12 left, fx12 right) {
    fx12 result = {INT32_MAX};
    if (right.data != 0) {
        result.data = (i32)(((i64)left.data << 12)/((i64)right.data));
    }
    return result;
}

fx24 fx24_mul(fx24 left, fx24 right) {
    return (fx24){ (i32)(((i64)left.data * (i64)right.data) >> 24)};
}

fx24 fx24_div(fx24 left, fx24 right) {
    fx24 result = {INT32_MAX};
    if (right.data != 0) {
        result.data = (i32)(((i64)left.data << 24)/((i64)right.data));
    }
    return result;
}
