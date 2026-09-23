//
// Created by wright on 5/1/26.
//

#include "init.h"

#include <assert.h>

#include "libs/dlmalloc/malloc.h"
#include <SDL3/SDL_stdinc.h>

#include "memory.h"
#include "defer.h"
#include "clock.h"
#include "db.h"

void alt_init(const AltInitInfo* info) {
    dlmalloc_set_footprint_limit(info->max_address_space_size);

    bool success = SDL_SetMemoryFunctions(alt_malloc, alt_calloc, alt_realloc, alt_free);
    assert(success);

    clock_init();
    db_init();
    defer_init();
}

void alt_deinit() {
    defer_uninit();
    db_deinit();
    clock_deinit();

    SDL_malloc_func malloc_fn;
    SDL_calloc_func calloc_fn;
    SDL_realloc_func realloc_fn;
    SDL_free_func free_fn;

    SDL_GetOriginalMemoryFunctions(&malloc_fn, &calloc_fn, &realloc_fn, &free_fn);

    bool success = SDL_SetMemoryFunctions(malloc_fn, calloc_fn, realloc_fn, free_fn);
    assert(success);

    dlmalloc_trim(0);
}
