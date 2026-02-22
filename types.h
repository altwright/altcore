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

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef float f32;
typedef double f64;

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
(!(array_ptr)->data || (array_ptr)->len <= 0 || (array_ptr)->cap <= 0)
#endif

#ifndef ARRAY_FOR
#define ARRAY_FOR(ptr_var, array_ptr) \
for ( \
    typeof(*((array_ptr)->data)) *ptr_var = (array_ptr)->data; \
    ptr_var < (array_ptr)->data + (array_ptr)->len; \
    ptr_var++ \
)
#endif

#ifndef ARRAY_EXTEND
#define ARRAY_EXTEND(array_ptr) \
do { \
    array_extend( \
        (void**)&((array_ptr)->data), \
        (array_ptr)->len, \
        (array_ptr)->cap, \
        sizeof(*((array_ptr)->data)), \
        (array_ptr)->arena \
    ); \
} while(0)
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

void array_extend(
    void **data_ptr,
    const i64 *len,
    i64 *cap,
    i64 elem_size,
    struct ARENA_T *arena
);

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

#endif //ALTCORE_TYPES_H
