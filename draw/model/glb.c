//
// Created by wright on 10/17/26.
//

#include "glb.h"

#include "../../memory.h"

#define CGLTF_IMPLEMENTATION
#include "../../../libs/cgltf.h"

void *glb_alloc(void *user, size_t size) {
    return alt_malloc(size);
}

void glb_free(void *user, void *ptr) {
    return alt_free(ptr);
}
