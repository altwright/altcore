//
// Created by wright on 5/3/26.
//

#include "renderer.h"

#include <SDL3/SDL_cpuinfo.h>

#include "../debug.h"
#include "../memory.h"
#include "../worker.h"
#include "cmds/clear.h"

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

void renderer_destroy(Renderer *renderer) {
    for (i32 thread_idx = 0; thread_idx < renderer->data.software.rendering_threads_count; thread_idx++) {
        worker_destroy(renderer->data.software.rendering_threads[thread_idx]);
    }

    alt_free(renderer->data.software.rendering_threads);
    renderer->data.software.rendering_threads = nullptr;
    renderer->data.software.rendering_threads_count = 0;

    alt_free(renderer);
}

void renderer_execute_cmd_bufs(Renderer *renderer, RenderCmdBuffer cmd_bufs[], i32 cmd_bufs_len) {
    switch (renderer->type) {
        case RENDERER_TYPE_SOFTWARE: {
            i32 workers_per_cmd_buf = renderer->data.software.rendering_threads_count / cmd_bufs_len;
            if (workers_per_cmd_buf < 1) {
                workers_per_cmd_buf = 1;
            }

            bool serial_execution = false;
            if ((cmd_bufs_len == 1)
                || (
                    (workers_per_cmd_buf * cmd_bufs_len) > renderer->data.software.rendering_threads_count
                )
            ) {
                serial_execution = true;
            }

            for (i32 cmd_buf_idx = 0; cmd_buf_idx < cmd_bufs_len; cmd_buf_idx++) {
                RenderCmdBuffer *cmd_buf = &cmd_bufs[cmd_buf_idx];

                i32 worker_group_offset = serial_execution ? 0 : (cmd_buf_idx * workers_per_cmd_buf);

                ARRAY_FOR(cmd, cmd_buf) {
                    switch (cmd->type) {
                        case RENDER_CMD_TYPE_CLEAR: {
                            soft_renderer_clear(
                                cmd->data.clear.framebuffer,
                                cmd->data.clear.rgba,
                                renderer->data.software.rendering_threads + worker_group_offset,
                                workers_per_cmd_buf
                            );
                            break;
                        }
                        case RENDER_CMD_TYPE_FRAMEBUFFER_TRANSITION: {

                            break;
                        }
                        default:
                            crash_msg("Unhandled cmd buf type %d\n", cmd->type);
                            break;
                    }
                }
            }

            break;
        }
        default:
            break;
    }
}
