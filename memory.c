//
// Created by wright on 2/19/26.
//

#include "memory.h"

#define USE_LOCKS 1
#include "libs/dlmalloc/malloc.c"

void *alt_malloc(size_t size) {
    return dlmalloc(size);
}

void alt_free(void *ptr) {
    return dlfree(ptr);
}

void *alt_calloc(size_t num_elems, size_t elem_size) {
    return dlcalloc(num_elems, elem_size);
}

void *alt_realloc(void *ptr, size_t new_size) {
    return dlrealloc(ptr, new_size);
}
