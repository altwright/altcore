//
// Created by wright on 2/19/26.
//

#ifndef ALTCORE_MEMORY_H
#define ALTCORE_MEMORY_H

#include <stddef.h>

#include "types.h"

#ifndef KIBIBYTE
#define KIBIBYTE (1024ULL)
#endif

#ifndef MIBIBYTE
#define MIBIBYTE (1024ULL * KIBIBYTE)
#endif

#ifndef GIBIBYTE
#define GIBIBYTE (1024ULL * MIBIBYTE)
#endif

void *alt_malloc(size_t size);

void alt_free(void *ptr);

void* alt_calloc(size_t num_elems, size_t elem_size);

void* alt_realloc(void *ptr, size_t new_size);

#endif //ALTCORE_MEMORY_H
