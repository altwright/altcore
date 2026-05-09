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
            Barrier **frame_barriers;
            i32 frame_barriers_count;
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

    i32 frame_barriers_count = create_info->max_frames_in_flight;
    Barrier **frame_barriers = alt_calloc(
        frame_barriers_count,
        sizeof(*frame_barriers)
    );

    for (i32 barrier_idx = 0; barrier_idx < frame_barriers_count; barrier_idx++) {
        BarrierCreateInfo barrier_info = {
            .expected_threads = num_rendering_threads,
        };

        frame_barriers[barrier_idx] = barrier_create(&barrier_info);
    }

    *renderer = (Renderer){
        .type = RENDERER_TYPE_SOFTWARE,
        .data = {
            .software = {
                .rendering_threads = rendering_threads,
                .rendering_threads_count = num_rendering_threads,
                .frame_barriers = frame_barriers,
                .frame_barriers_count = frame_barriers_count,
            },
        },
    };

    return renderer;
}

void renderer_destroy(Renderer *renderer) {
    for (i32 thread_idx = 0; thread_idx < renderer->data.software.rendering_threads_count; thread_idx++) {
        worker_destroy(renderer->data.software.rendering_threads[thread_idx]);
    }

    for (i32 barrier_idx = 0; barrier_idx < renderer->data.software.frame_barriers_count; barrier_idx++) {
        barrier_destroy(renderer->data.software.frame_barriers[barrier_idx]);
    }

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
                        FramebufferData fb_data = {};
                        switch (cmd->data.clear.buf_type) {
                            case RENDER_BUFFER_TYPE_FRAMEBUFFER: {
                                fb_data = framebuffer_get_data(cmd->data.clear.buf.framebuffer);
                                break;
                            }
                            case RENDER_BUFFER_TYPE_SWAPCHAIN: {
                                fb_data = swapchain_buf_get_data(cmd->data.clear.buf.swapchain_buf);
                                break;
                            }
                            default:
                                crash_msg("Unhandled render buffer type %d\n", cmd->data.clear.buf_type);
                                break;
                        }

                        soft_renderer_clear(
                            fb_data,
                            cmd->data.clear.rgba,
                            renderer->data.software.rendering_threads,
                            renderer->data.software.rendering_threads_count
                        );
                        break;
                    }
                    case RENDER_CMD_TYPE_SWAPCHAIN_BUF_PRESENT: {
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
