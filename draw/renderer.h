//
// Created by wright on 5/3/26.
//

#ifndef ALTCORE_RENDERER_H
#define ALTCORE_RENDERER_H

#include "types.h"
#include "draw/framebuffer.h"

struct RENDERER_T;
typedef struct RENDERER_T Renderer;

typedef struct RENDERER_CREATE_INFO_T {

} RendererCreateInfo;

Renderer* renderer_create(const RendererCreateInfo* create_info);

void renderer_clear_framebuffer(Framebuffer* framebuffer, uVec4 rgba);

void renderer_destroy(Renderer* renderer);

#endif //ALTCORE_RENDERER_H
