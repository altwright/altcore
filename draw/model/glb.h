//
// Created by wright on 10/17/26.
//

#ifndef BRITANNICUS_GLB_H
#define BRITANNICUS_GLB_H

#include "../../libs/cgltf.h"

void* glb_alloc(void* user, size_t size);

void glb_free(void* user, void* ptr);

#endif //BRITANNICUS_GLB_H
