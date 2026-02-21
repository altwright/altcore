//
// Created by wright on 2/20/26.
//

#include "hashmap.h"
#include "memory.h"

static void* hashmap_realloc(void* context, void* ptr, size_t new_size) {
    return alt_realloc(ptr, new_size);
}

static void hashmap_free(void* context, void* ptr) {
    alt_free(ptr);
}

#ifdef STBDS_REALLOC
#undef STBDS_REALLOC
#define STBDS_REALLOC(context, ptr, size) hashmap_realloc(context, ptr, size)
#endif

#ifdef STBDS_FREE
#undef STBDS_FREE
#define STBDS_FREE(context, ptr) hashmap_free(context, ptr)
#endif

#define STB_DS_IMPLEMENTATION
#include "libs/stb_ds.h"
