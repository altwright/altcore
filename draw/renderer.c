//
// Created by wright on 5/3/26.
//

#include "renderer.h"

#include <SDL3/SDL_cpuinfo.h>

#include "memory.h"
#include "worker.h"

typedef enum RENDERER_TYPE_E {
#ifndef X_RENDERER_TYPES
#define X_RENDERER_TYPES \
    X(SOFTWARE) \
    X(COUNT)
#endif
#ifndef X
#define X(type) \
    RENDERER_TYPE_##type,
#endif
    X_RENDERER_TYPES
#undef X
} RendererType;

struct RENDERER_T {
    RendererType type;

    union {
        struct {
            Worker **rendering_threads;
            i32 rendering_threads_count;
        } software;
    } data;
};

Renderer *renderer_create(const RendererCreateInfo *create_info) {
    Renderer *renderer = alt_malloc(sizeof(Renderer));

    i32 max_logical_threads = SDL_GetNumLogicalCPUCores();

    i32 num_rendering_threads = max_logical_threads - 1;
    if (num_rendering_threads < 1) {
        num_rendering_threads = 1;
    }

    Worker **rendering_threads = alt_calloc(num_rendering_threads,
                                            sizeof(*renderer->data.software.rendering_threads));

    for (i32 thread_idx = 0; thread_idx < num_rendering_threads; thread_idx++) {
        WorkerCreateInfo worker_info = {
            .task_q_cap = 64,
        };
        rendering_threads[thread_idx] = worker_create(&worker_info);
    }

    *renderer = (Renderer){
        .type = RENDERER_TYPE_SOFTWARE,
        .data = {
            .software = {
                .rendering_threads = rendering_threads,
                .rendering_threads_count = num_rendering_threads,
            },
        },
    };

    return renderer;
}

void renderer_clear_framebuffer(Framebuffer *framebuffer, uVec4 rgba) {
}

void renderer_destroy(Renderer *renderer) {
    for (i32 thread_idx = 0; thread_idx < renderer->data.software.rendering_threads_count; thread_idx++) {
        worker_destroy(renderer->data.software.rendering_threads[thread_idx]);
    }

    alt_free(renderer->data.software.rendering_threads);
    renderer->data.software.rendering_threads = nullptr;
    renderer->data.software.rendering_threads_count = 0;

    alt_free(renderer);
}
