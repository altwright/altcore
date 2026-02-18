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

#ifndef ARRAY
#define ARRAY(type) \
type *data; \
i64 len; \
i64 cap; \
struct ARENA_T* arena;
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
    typeof(*((array_ptr)->data)) ptr_var = (array_ptr)->data; \
    ptr_var < (array_ptr)->data + (array_ptr)->len; \
    ptr_var++ \
)
#endif

#ifndef ARRAY_EXTEND
#define ARRAY_EXTEND(array_ptr) \
do { \
    array_extend( \
        (array_ptr)->data, \
        (array_ptr)->len, \
        (array_ptr)->cap, \
        sizeof(*((array_ptr)->data)), \
        (array_ptr)->arena \
    ); \
} while(0)
#endif

void array_alloc(
    void **data_ptr,
    i64 *len,
    i64 *cap,
    i64 elem_size,
    struct ARENA_T* arena
);

void array_push(
    void **data_ptr,
    i64 *len,
    i64 *cap,
    i64 elem_size,
    const void *new_elem,
    struct ARENA_T* arena
);

void array_extend(
    void *data_ptr,
    i64 *len,
    i64 *cap,
    i64 elem_size,
    struct ARENA_T* arena
);

#endif //ALTCORE_TYPES_H
