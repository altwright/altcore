//
// Created by wright on 2/19/26.
//

#ifndef ALTCORE_MALLOC_H
#define ALTCORE_MALLOC_H

#include <stddef.h>

#include "types.h"

void alt_init(i64 initial_cap);

void alt_uninit();

void *alt_malloc(size_t size);

void alt_free(void *ptr);

void* alt_calloc(size_t num_elems, size_t elem_size);

void* alt_realloc(void *ptr, size_t new_size);

#endif //ALTCORE_MALLOC_H
