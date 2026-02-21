//
// Created by wright on 2/20/26.
//

#include "hashmap.h"
#include "malloc.h"

#ifdef STBDS_REALLOC
#undef STBDS_REALLOC
#define STBDS_REALLOC(context, ptr, size) alt_realloc(ptr, size)
#endif

#ifdef STBDS_FREE
#undef STBDS_FREE
#define STBDS_FREE(context, ptr) alt_free(ptr)
#endif

#define STB_DS_IMPLEMENTATION
#include "libs/stb_ds.h"
