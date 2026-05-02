//
// Created by wright on 2/18/26.
//

#include "types.h"

#include <stdlib.h>
#include <string.h>

#include "assert.h"
#include "arenas.h"

void *kNullPtr = nullptr;

const f32 kF32Infinity = {
    .data = {0xff, 0xff, 0xff, 0xff},
};

const f64 kF64Infinity = {
    .data = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff},
};

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
    assert(data_ptr && *data_ptr);
    assert(data_elem_size == new_elem_size);
    assert(*len >= 0);
    assert(*len <= *cap);

    if (*len == *cap) {
        i64 new_cap = 2 * (*cap);
        u8 *new_data = arena_alloc(arena, new_cap * data_elem_size);
        assert(new_data);
        memcpy(new_data, *data_ptr, (*len) * data_elem_size);
        *data_ptr = new_data;
        *cap = new_cap;
    }

    u8 *data_bytes = *data_ptr;
    u8 *new_elem_bytes = data_bytes + (new_elem_size * (*len));
    memcpy(new_elem_bytes, new_elem, new_elem_size);

    (*len)++;
}

void array_sort(
    void *data,
    i64 len,
    i64 elem_size,
    int (*sort_fn)(const void *, const void *)
) {
    assert(data && len > 0 && elem_size > 0 && sort_fn);
    qsort(data, len, elem_size, sort_fn);
}

void v128_add(VecElemType type, const v128 *left, const v128 *right, v128 *out) {
}

void v128_sub(VecElemType type, const v128 *left, const v128 *right, v128 *out) {
}

void v128_mul(VecElemType type, const v128 *left, const v128 *right, v128 *out) {
}

void v128_div(VecElemType type, const v128 *left, const v128 *right, v128 *out) {
}

f32 f32_init_ex(float val, F32Precision precision) {
    f32 out = {};

    switch (precision) {
        case F32_PRECISION_FLOAT: {
            float *fo = (float *) &out;
            *fo = val;
            break;
        }
        default:
            assert(0 && "Unhandled f32 precision");
            break;
    }

    return out;
}

f32 f32_add_ex(f32 left, f32 right, F32Precision precision) {
    f32 out = {};

    switch (precision) {
        case F32_PRECISION_FLOAT: {
            float *fl = (float *) (&left);
            float *fr = (float *) (&right);
            float *fo = (float *) (&out);

            *fo = *fl + *fr;
            break;
        }
        default: {
            assert(0 && "Unhandled f32 precision");
            break;
        }
    }

    return out;
}

f32 f32_sub_ex(f32 left, f32 right, F32Precision precision) {
    f32 out = {};

    switch (precision) {
        case F32_PRECISION_FLOAT: {
            float *fl = (float *) (&left);
            float *fr = (float *) (&right);
            float *fo = (float *) (&out);

            *fo = *fl - *fr;
            break;
        }
        default: {
            assert(0 && "Unhandled f32 precision");
            break;
        }
    }

    return out;
}

f32 f32_mul_ex(f32 left, f32 right, F32Precision precision) {
    f32 out = {};

    switch (precision) {
        case F32_PRECISION_FLOAT: {
            float *fl = (float *) (&left);
            float *fr = (float *) (&right);
            float *fo = (float *) (&out);

            *fo = (*fl) * (*fr);
            break;
        }
        default: {
            assert(0 && "Unhandled f32 precision");
            break;
        }
    }

    return out;
}

f32 f32_div_ex(f32 left, f32 right, F32Precision precision) {
    f32 out = {};

    if (memcmp(&right, &out, sizeof(f32)) == 0) {
        out = kF32Infinity;
    } else {
        switch (precision) {
            case F32_PRECISION_FLOAT: {
                float *fl = (float *) (&left);
                float *fr = (float *) (&right);
                float *fo = (float *) (&out);
                *fo = (*fl) / (*fr);

                break;
            }
            default: {
                assert(0 && "Unhandled f32 precision");
                break;
            }
        }
    }

    return out;
}

bool f32_is_inf(f32 val) {
    return memcmp(&val, &kF32Infinity, sizeof(f32)) == 0;
}

f64 f64_init_ex(double val, F64Precision precision) {
    f64 out = {};

    switch (precision) {
        case F64_PRECISION_DOUBLE: {
            double *fo = (double *) (&out);
            *fo = val;
            break;
        }
        default: {
            assert(0 && "Unhandled f64 precision");
            break;
        }
    }

    return out;
}

f64 f64_add_ex(f64 left, f64 right, F64Precision precision) {
    f64 out = {};

    switch (precision) {
        case F64_PRECISION_DOUBLE: {
            double *fl = (double *) (&left);
            double *fr = (double *) (&right);
            double *fo = (double *) (&out);

            *fo = *fl + *fr;
            break;
        }
        default: {
            assert(0 && "Unhandled f64 precision");
            break;
        }
    }

    return out;
}

f64 f64_sub_ex(f64 left, f64 right, F64Precision precision) {
    f64 out = {};

    switch (precision) {
        case F64_PRECISION_DOUBLE: {
            double *fl = (double *) (&left);
            double *fr = (double *) (&right);
            double *fo = (double *) (&out);

            *fo = *fl - *fr;
            break;
        }
        default: {
            assert(0 && "Unhandled f64 precision");
            break;
        }
    }

    return out;
}

f64 f64_mul_ex(f64 left, f64 right, F64Precision precision) {
    f64 out = {};

    switch (precision) {
        case F64_PRECISION_DOUBLE: {
            double *fl = (double *) (&left);
            double *fr = (double *) (&right);
            double *fo = (double *) (&out);

            *fo = (*fl) * (*fr);
            break;
        }
        default: {
            assert(0 && "Unhandled f64 precision");
            break;
        }
    }

    return out;
}

f64 f64_div_ex(f64 left, f64 right, F64Precision precision) {
    f64 out = {};

    if (memcmp(&right, &out, sizeof(f64)) == 0) {
        out = kF64Infinity;
    } else {
        switch (precision) {
            case F64_PRECISION_DOUBLE: {
                double *fl = (double *) (&left);
                double *fr = (double *) (&right);
                double *fo = (double *) (&out);
                *fo = (*fl) / (*fr);
                break;
            }
            default: {
                assert(0 && "Unhandled f64 precision");
                break;
            }
        }
    }

    return out;
}

bool f64_is_inf(f64 val) {
    return memcmp(&val, &kF64Infinity, sizeof(f64)) == 0;
}
