//
// Created by wright on 5/3/26.
//

#include "renderer.h"

#include <SDL3/SDL_cpuinfo.h>

#include "../debug.h"
#include "../memory.h"
#include "../worker.h"
#include "cmds/clear.h"
#include "../barrier.h"
#include "cmds/present.h"

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
            Barrier *sync_barrier;
        } software;
    } data;
};

constexpr i32 kDefaultCmdBufCap = 128;

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

    BarrierCreateInfo barrier_info = {
        .expected_threads = num_rendering_threads,
    };

    *renderer = (Renderer){
        .type = RENDERER_TYPE_SOFTWARE,
        .data = {
            .software = {
                .rendering_threads = rendering_threads,
                .rendering_threads_count = num_rendering_threads,
                .sync_barrier = barrier_create(&barrier_info),
            },
        },
    };

    return renderer;
}

void renderer_destroy(Renderer *renderer) {
    for (i32 thread_idx = 0; thread_idx < renderer->data.software.rendering_threads_count; thread_idx++) {
        worker_destroy(renderer->data.software.rendering_threads[thread_idx]);
    }

    barrier_destroy(renderer->data.software.sync_barrier);

    alt_free(renderer->data.software.rendering_threads);
    renderer->data.software.rendering_threads = nullptr;
    renderer->data.software.rendering_threads_count = 0;

    alt_free(renderer);
}

void renderer_execute_cmd_buf(Renderer *renderer, RenderCmdBuffer *cmd_buf) {
    switch (renderer->type) {
        case RENDERER_TYPE_SOFTWARE: {
            ARRAY_FOR(cmd, cmd_buf) {
                switch (cmd->type) {
                    case RENDER_CMD_TYPE_CLEAR: {
                        soft_renderer_clear(
                            cmd->data.clear.fb_data,
                            cmd->data.clear.rgba,
                            renderer->data.software.rendering_threads,
                            renderer->data.software.rendering_threads_count,
                            renderer->data.software.sync_barrier
                        );
                        break;
                    }
                    case RENDER_CMD_TYPE_PRESENT: {
                        soft_renderer_present(
                            cmd->data.present.window,
                            cmd->data.present.swapchain_buf,
                            renderer->data.software.rendering_threads[0]
                        );
                        break;
                    }
                    default:
                        crash_msg("Unhandled cmd buf type %d\n", cmd->type);
                        break;
                }
            }
            break;
        }
        default:
            crash_msg("Unhandled renderer type");
            break;
    }
}

void renderer_wait_until_idle(Renderer *renderer) {
    i32 remaining_tasks = 0;
    do {
        remaining_tasks = 0;
        for (i32 thread_idx = 0; thread_idx < renderer->data.software.rendering_threads_count; thread_idx++) {
            remaining_tasks += worker_get_task_count(renderer->data.software.rendering_threads[thread_idx]);
        }
    } while (remaining_tasks);
}
