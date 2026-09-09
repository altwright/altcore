//
// Created by wright on 2/18/26.
//

#include "types.h"

#include <stdlib.h>
#include <string.h>

#include "assert.h"
#include "arenas.h"

void *kNullPtr = nullptr;

void array_make(
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
        *cap = 1;
    }

    *data_ptr = arena_alloc(arena, (*cap) * elem_size);
    assert(*data_ptr);
    if ((*len) > 0) {
        memset(*data_ptr, 0, (*len) * elem_size);
    }
}

static void array_expand(
    void **data_ptr,
    i64 data_elem_size,
    i64 *len,
    i64 *cap,
    struct ARENA_T *arena
) {
    i64 new_cap = 2 * (*cap);
    u8 *new_data = arena_alloc(arena, new_cap * data_elem_size);
    assert(new_data);
    memcpy(new_data, *data_ptr, (*len) * data_elem_size);
    *data_ptr = new_data;
    *cap = new_cap;
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
    assert(data_ptr && *data_ptr);
    assert(data_elem_size == new_elem_size);
    assert(*len >= 0);
    assert(*len <= *cap);

    while (*len >= *cap) {
        array_expand(data_ptr, data_elem_size, len, cap, arena);
    }

    u8 *data = *data_ptr;
    u8 *new_elem_bytes = data + (data_elem_size * (*len));
    memcpy(new_elem_bytes, new_elem, new_elem_size);

    (*len)++;
}

void array_put(
    void **data_ptr,
    i64 data_elem_size,
    i64 *len,
    i64 *cap,
    void *new_elem,
    i64 new_elem_size,
    i64 idx,
    struct ARENA_T *arena
) {
    assert(data_elem_size == new_elem_size);
    assert(idx <= *len); // Can be inserted at the end of the array

    while (*len >= *cap) {
        array_expand(data_ptr, data_elem_size, len, cap, arena);
    }

    u8 *data = *data_ptr;

    for (i64 current_idx = *len; current_idx > idx; current_idx--) {
        u8 *current_elem = data + (current_idx * data_elem_size);
        u8 *prev_elem = current_elem - data_elem_size;
        memcpy(current_elem, prev_elem, data_elem_size);
    }

    memcpy(data + (idx * data_elem_size), new_elem, data_elem_size);

    (*len)++;
}

void array_del(
    void *data,
    i64 data_elem_size,
    i64 *len,
    i64 idx
) {
    for (i64 current_idx = idx; current_idx < (*len - 1); current_idx++) {
        u8 *current_elem_byte = (u8 *) data + (current_idx * data_elem_size);
        u8 *next_elem_byte = current_elem_byte + data_elem_size;
        memcpy(current_elem_byte, next_elem_byte, data_elem_size);
    }

    (*len)--;
}

void array_sort(
    void *data,
    i64 len,
    i64 elem_size,
    int (*sort_fn)(const void *, const void *)
) {
    if (len > 1) {
        qsort(data, len, elem_size, sort_fn);
    }
}

i32x4 f32x4_to_i32(f32x4 vec) {
    return (i32x4){
        .x = (i32) (vec.x),
        .y = (i32) (vec.y),
        .z = (i32) (vec.z),
        .w = (i32) (vec.w),
    };
}

f32x4 i32x4_to_f32(i32x4 vec) {
    return (f32x4){
        .x = (f32) (vec.x),
        .y = (f32) (vec.y),
        .z = (f32) (vec.z),
        .w = (f32) (vec.w),
    };
}

void u64x2_bit_set(u64x2 *bits, i32 bit_idx) {
    if (bit_idx < 0 || bit_idx >= 128) {
        return;
    }

    if (bit_idx < 64) {
        bits->lower |= 1ULL << bit_idx;
    } else {
        bits->upper |= 1ULL << (bit_idx - 64);
    }
}

void u64x2_bit_unset(u64x2 *bits, i32 bit_idx) {
    if (bit_idx < 0 || bit_idx >= 128) {
        return;
    }

    if (bit_idx < 64) {
        bits->lower &= ~(1ULL << bit_idx);
    } else {
        bits->upper &= ~(1ULL << (bit_idx - 64));
    }
}

bool u64x2_bit_is_set(u64x2 bits, i32 bit_idx) {
    if (bit_idx < 0 || bit_idx >= 128) {
        return false;
    }

    if (bit_idx < 64) {
        return bits.lower & (1ULL << bit_idx);
    } else {
        return bits.upper & (1ULL << (bit_idx - 64));
    }
}

u64x2 u64x2_bits_and(u64x2 left, u64x2 right) {
    return (u64x2){
        .lower = (left.lower & right.lower),
        .upper = (left.upper & right.upper),
    };
}

u64x2 u64x2_bits_or(u64x2 left, u64x2 right) {
    return (u64x2){
        .lower = (left.lower | right.lower),
        .upper = (left.upper | right.upper),
    };
}

u64x2 u64x2_bits_not(u64x2 bits) {
    return (u64x2){
        .lower = ~bits.lower,
        .upper = ~bits.upper,
    };
}

bool u64x2_equal(u64x2 left, u64x2 right) {
    return (left.lower == right.lower) && (left.upper == right.upper);
}

void queue_make(
    struct ARENA_T *arena,
    i64 *cap,
    i64 *count,
    i64 *head,
    void **data,
    u64 elem_size
) {
    if (*count < 0) {
        *count = 0;
    }

    if (*cap < 0) {
        *cap = 0;
    }

    if (*cap < *count) {
        *cap = *count;
    }

    *head = 0;

    *data = arena_alloc(arena, (*cap) * (i64) elem_size);
    memset(*data, 0, (*count) * elem_size);
}

static void queue_expand(
    struct ARENA_T *arena,
    i64 *cap,
    void** data,
    u64 elem_size
) {
    i64 new_cap = 2 * (*cap);
    void* new_data = arena_alloc(arena, new_cap * (i64)elem_size);
    memcpy(new_data, *data, (*cap) * elem_size);
    *data = new_data;
    *cap = new_cap;
}

void queue_push_back(
    struct ARENA_T *arena,
    i64 *cap,
    i64 *count,
    i64 *head,
    void **data,
    u64 elem_size,
    void *new_elem_ptr
) {
    while (*count >= *cap) {
        queue_expand(arena, cap, data, elem_size);
    }

    i64 new_elem_idx = (*head + *count) % (*cap);
    u8* new_elem_bytes = (u8*)(*data) + (new_elem_idx * elem_size);
    memcpy(new_elem_bytes, new_elem_ptr, elem_size);

    (*count)++;
}
