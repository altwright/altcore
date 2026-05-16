//
// Created by wright on 5/3/26.
//

#include "renderer.h"

#include <assert.h>
#include <SDL3/SDL_cpuinfo.h>

#include "../debug.h"
#include "../memory.h"
#include "../worker.h"
#include "cmds/clear.h"
#include "../barrier.h"
#include "cmds/present.h"
#include "framebuffer_impl.h"

struct RENDERER_T {
    RendererType type;

    union {
        struct {
            Worker **rendering_threads;
            i32 rendering_threads_count;
            Barrier *sync_barrier;
        } software_multi_thread;
    } data;
};

constexpr i32 kDefaultCmdBufCap = 128;

Renderer *renderer_create(const RendererCreateInfo *create_info) {
    Renderer *renderer = alt_malloc(sizeof(Renderer));

    *renderer = (Renderer){
        .type = create_info->type,
    };

    switch (renderer->type) {
        case RENDERER_TYPE_SOFTWARE_SINGLE_THREAD: {
            break;
        }
        case RENDERER_TYPE_SOFTWARE_MULTI_THREAD: {
            i32 max_logical_threads = SDL_GetNumLogicalCPUCores();

            i32 num_rendering_threads = max_logical_threads - 1;
            if (num_rendering_threads < 1) {
                num_rendering_threads = 1;
            }

            Worker **rendering_threads = alt_calloc(
                num_rendering_threads,
                sizeof(*rendering_threads)
            );

            for (i32 thread_idx = 0; thread_idx < num_rendering_threads; thread_idx++) {
                WorkerCreateInfo worker_info = {
                    .task_q_cap = 64,
                };
                rendering_threads[thread_idx] = worker_create(&worker_info);
            }

            BarrierCreateInfo barrier_info = {
                .expected_threads = num_rendering_threads,
            };

            renderer->data.software_multi_thread.rendering_threads = rendering_threads;
            renderer->data.software_multi_thread.rendering_threads_count = num_rendering_threads;
            renderer->data.software_multi_thread.sync_barrier = barrier_create(&barrier_info);

            break;
        }
        default:
            crash_msg("Unhandled renderer type %d\n", renderer->type);
            break;
    }

    return renderer;
}

void renderer_destroy(Renderer *renderer) {
    for (i32 thread_idx = 0; thread_idx < renderer->data.software_multi_thread.rendering_threads_count; thread_idx++) {
        worker_destroy(renderer->data.software_multi_thread.rendering_threads[thread_idx]);
    }

    barrier_destroy(renderer->data.software_multi_thread.sync_barrier);

    alt_free(renderer->data.software_multi_thread.rendering_threads);
    renderer->data.software_multi_thread.rendering_threads = nullptr;
    renderer->data.software_multi_thread.rendering_threads_count = 0;

    alt_free(renderer);
}

void renderer_execute_cmd_buf(Renderer *renderer, RenderCmdBuffer *cmd_buf) {
    switch (renderer->type) {
        case RENDERER_TYPE_SOFTWARE_SINGLE_THREAD: {
            ARRAY_FOR(cmd, cmd_buf) {
                switch (cmd->type) {
                    case RENDER_CMD_TYPE_CLEAR: {
                        FramebufferInfo fb_info = framebuffer_get_info(cmd->data.clear.framebuffer);
                        assert(fb_info.type == FRAMEBUFFER_TYPE_PIXEL);
                        u8* pixel_bytes = framebuffer_impl_get_bytes(cmd->data.clear.framebuffer);
                        cmd_soft_single_clear(
                            pixel_bytes,
                            fb_info.data.pixel_buf.format,
                            fb_info.data.pixel_buf.size,
                            fb_info.data.pixel_buf.pitch_bytes,
                            cmd->data.clear.rgba
                        );
                        break;
                    }
                    case RENDER_CMD_TYPE_PRESENT: {
                        FramebufferInfo fb_info = framebuffer_get_info(cmd->data.present.framebuffer);
                        assert(fb_info.type == FRAMEBUFFER_TYPE_PIXEL);
                        cmd_soft_single_present(cmd->data.present.framebuffer, cmd->data.present.window);
                        break;
                    }
                    default:
                        crash_msg("Unhandled cmd %d\n", cmd->type);
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
    switch (renderer->type) {
        case RENDERER_TYPE_SOFTWARE_SINGLE_THREAD: {
            break;
        }
        default:
            crash_msg("Unhandled renderer type %d\n", renderer->type);
            break;
    }
}
