//
// Created by wright on 5/1/26.
//

#include "init.h"

#include <assert.h>

#include "defer.h"
#include "libs/dlmalloc/malloc.h"

void alt_init(i64 address_space_max_size) {
    size_t real_size = dlmalloc_set_footprint_limit(address_space_max_size);
    assert(real_size == address_space_max_size);

    defer_init();
}

void alt_uninit() {
    defer_uninit();
}
