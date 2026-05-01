//
// Created by wright on 5/1/26.
//

#include "init.h"

#include <assert.h>

#include "libs/dlmalloc/malloc.h"
#include <SDL3/SDL_stdinc.h>

#include "memory.h"
#include "defer.h"

void alt_init(i64 address_space_max_size) {
    dlmalloc_set_footprint_limit(address_space_max_size);

    bool success = SDL_SetMemoryFunctions(alt_malloc, alt_calloc, alt_realloc, alt_free);
    assert(success);

    defer_init();
}

void alt_uninit() {
    defer_uninit();

    SDL_malloc_func malloc_fn;
    SDL_calloc_func calloc_fn;
    SDL_realloc_func realloc_fn;
    SDL_free_func free_fn;

    SDL_GetOriginalMemoryFunctions(&malloc_fn, &calloc_fn, &realloc_fn, &free_fn);

    bool success = SDL_SetMemoryFunctions(malloc_fn, calloc_fn, realloc_fn, free_fn);
    assert(success);

    dlmalloc_trim(0);
}
