//
// Created by wright on 2/18/26.
//

#ifndef ALTCORE_TYPES_H
#define ALTCORE_TYPES_H

#include <stdint.h>

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
typedef _BitInt(128) i128;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef unsigned _BitInt(128) u128;

typedef struct F32_T {
    u8 data[4];
} f32;

typedef struct F64_T {
    u8 data[8];
} f64;

typedef enum VEC_TYPE {
#ifndef X_VEC_ELEM_TYPES
#define X_VEC_ELEM_TYPES \
    X(U8) \
    X(U16) \
    X(U32) \
    X(U64) \
    X(I8) \
    X(I16) \
    X(I32) \
    X(I64) \
    X(F32) \
    X(F64) \
    X(COUNT)
#endif
#ifndef X
#define X(type) \
    VEC_ELEM_TYPE_##type,
#endif
    X_VEC_ELEM_TYPES
#undef X
} VecElemType;

typedef struct V64_T {
    u8 data[8];
} v64;

typedef struct V128_T {
    u8 data[16];
} v128;

typedef struct V256_T {
    u8 data[32];
} v256;

typedef struct V512_T {
    u8 data[64];
} v512;

typedef struct I_VEC2_T {
    union {
        v64 data;

        struct {
            i32 x, y;
        };

        struct {
            i32 u, v;
        };
    };
} iVec2;

typedef struct F_VEC2_T {
    union {
        v64 data;

        struct {
            f32 x, y;
        };

        struct {
            f32 u, v;
        };
    };
} fVec2;

typedef struct I_VEC4_T {
    union {
        v128 data;

        struct {
            i32 x, y, z, w;
        };

        struct {
            i32 r, g, b, a;
        };
    };
} iVec4;

typedef struct F_VEC4_T {
    union {
        v128 data;

        struct {
            f32 x, y, z, w;
        };

        struct {
            f32 r, g, b, a;
        };
    };
} fVec4;

typedef struct I_RECT_T {
    i32 x, y, w, h;
} iRect;

typedef struct F_RECT_T {
    f32 x, y, w, h;
} fRect;

struct ARENA_T;

extern void *kNullPtr;

#ifndef ARRAY_FIELDS
#define ARRAY_FIELDS(type) \
struct ARENA_T* arena; \
i64 len; \
i64 cap; \
type *data;
#endif

#ifndef ARRAY_MAKE
#define ARRAY_MAKE(array_ptr) \
do { \
    array_alloc( \
        (void**)(&(array_ptr)->data), \
        &((array_ptr)->len), \
        &((array_ptr)->cap), \
        sizeof(*((array_ptr)->data)), \
        (array_ptr)->arena \
    ); \
} while(0)
#endif

#ifndef ARRAY_EMPTY
#define ARRAY_EMPTY(array_ptr) \
(!((array_ptr)->data) || (array_ptr)->len <= 0 || (array_ptr)->cap <= 0)
#endif

#ifndef ARRAY_FOR
#define ARRAY_FOR(ptr_var, array_ptr) \
for ( \
    typeof(*((array_ptr)->data)) *ptr_var = (array_ptr)->data; \
    ptr_var < (array_ptr)->data + (array_ptr)->len; \
    ptr_var++ \
)
#endif

#ifndef ARRAY_ELEM
#define ARRAY_ELEM(array_ptr, idx_ptr) \
( \
    (*(idx_ptr) >= 0) && (*(idx_ptr) < (array_ptr)->len) ? \
        &((array_ptr)->data[*(idx_ptr)]) : \
        &kNullPtr[*(idx_ptr)] \
)
#endif

#ifndef ARRAY_PUSH
#define ARRAY_PUSH(array_ptr, elem_ptr) \
do { \
    array_push( \
        (void**)(&((array_ptr)->data)), \
        &((array_ptr)->len), \
        &((array_ptr)->cap), \
        sizeof(*((array_ptr)->data)), \
        sizeof(*(elem_ptr)), \
        (const void*)(elem_ptr), \
        (array_ptr)->arena \
    ); \
} while(0)
#endif

#ifndef ARRAY_POP
#define ARRAY_POP(array_ptr) \
( \
(array_ptr)->len > 0 ? \
(array_ptr)->data[--((array_ptr)->len)] : \
kNullPtr[0] \
)
#endif

#ifndef ARRAY_SORT
#define ARRAY_SORT(array_ptr, sort_fn) \
do { \
    array_sort( \
        (array_ptr)->data, \
        (array_ptr)->len, \
        sizeof(*((array_ptr)->data)), \
        &sort_fn \
    ); \
} while(0)
#endif

void array_alloc(
    void **data_ptr,
    i64 *len,
    i64 *cap,
    i64 elem_size,
    struct ARENA_T *arena
);

void array_push(
    void **data_ptr,
    i64 *len,
    i64 *cap,
    i64 data_elem_size,
    i64 new_elem_size,
    const void *new_elem,
    struct ARENA_T *arena
);

void array_sort(
    void *data,
    i64 len,
    i64 elem_size,
    int (*sort_fn)(const void *, const void *)
);

inline f32 f32_init(float val);

inline f32 f32_add(f32 left, f32 right);

inline f32 f32_sub(f32 left, f32 right);

inline f32 f32_mul(f32 left, f32 right);

inline f32 f32_div(f32 left, f32 right);

inline bool f32_is_inf(f32 val);

inline f64 f64_init(double val);

inline f64 f64_add(f64 left, f64 right);

inline f64 f64_sub(f64 left, f64 right);

inline f64 f64_mul(f64 left, f64 right);

inline f64 f64_div(f64 left, f64 right);

inline bool f64_is_inf(f64 val);

inline void v128_add(VecElemType type, const v128 *left, const v128 *right, v128 *out);

inline void v128_sub(VecElemType type, const v128 *left, const v128 *right, v128 *out);

inline void v128_mul(VecElemType type, const v128 *left, const v128 *right, v128 *out);

inline void v128_div(VecElemType type, const v128 *left, const v128 *right, v128 *out);

typedef struct I8S_T {
    ARRAY_FIELDS(i8)
} i8s;

typedef struct I16S_T {
    ARRAY_FIELDS(i16)
} i16s;

typedef struct I32S_T {
    ARRAY_FIELDS(i32)
} i32s;

typedef struct I64S_T {
    ARRAY_FIELDS(i64)
} i64s;

typedef struct U8S_T {
    ARRAY_FIELDS(u8)
} u8s;

typedef struct U16S_T {
    ARRAY_FIELDS(u16)
} u16s;

typedef struct U32S_T {
    ARRAY_FIELDS(u32)
} u32s;

typedef struct U64S_T {
    ARRAY_FIELDS(u64)
} u64s;

typedef struct F32S_T {
    ARRAY_FIELDS(f32)
} f32s;

typedef struct F64S_T {
    ARRAY_FIELDS(f64)
} f64s;

#ifndef STATIC_ARRAY_LEN
#define STATIC_ARRAY_LEN(k_array) \
    (sizeof(k_array) / sizeof(k_array[0]))
#endif

#endif //ALTCORE_TYPES_H
