//
// Created by wright on 5/3/26.
//

#include "renderer.h"

#include <assert.h>
#include <SDL3/SDL_cpuinfo.h>

#include "../debug.h"
#include "../memory.h"
#include "../worker.h"
#include "../barrier.h"
#include "cmds/clear.h"
#include "cmds/present.h"
#include "cmds/draw_rect.h"
#include "cmds/draw_text.h"

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
    switch (renderer->type) {
        case RENDERER_TYPE_SOFTWARE_SINGLE_THREAD: {
            break;
        }
        case RENDERER_TYPE_SOFTWARE_MULTI_THREAD: {
            for (i32 thread_idx = 0; thread_idx < renderer->data.software_multi_thread.rendering_threads_count;
                 thread_idx++) {
                worker_destroy(renderer->data.software_multi_thread.rendering_threads[thread_idx]);
            }

            barrier_destroy(renderer->data.software_multi_thread.sync_barrier);

            alt_free(renderer->data.software_multi_thread.rendering_threads);
            renderer->data.software_multi_thread.rendering_threads = nullptr;
            renderer->data.software_multi_thread.rendering_threads_count = 0;
            break;
        }
        default:
            crash_msg("Unhandled renderer type %d\n", renderer->type);
            break;
    }

    alt_free(renderer);
}

void renderer_execute_cmd_buf(Renderer *renderer, RenderCmdBuffer *cmd_buf) {
    switch (renderer->type) {
        case RENDERER_TYPE_SOFTWARE_SINGLE_THREAD: {
            ARRAY_FOR(cmd, cmd_buf) {
                switch (cmd->type) {
                    case RENDER_CMD_TYPE_CLEAR: {
                        soft_cmd_clear(
                            cmd->data.clear.framebuffer,
                            cmd->data.clear.color
                        );
                        break;
                    }
                    case RENDER_CMD_TYPE_PRESENT: {
                        soft_cmd_present(
                            cmd->data.present.framebuffer,
                            cmd->data.present.window
                        );
                        break;
                    }
                    case RENDER_CMD_TYPE_DRAW_RECT: {
                        RenderCmdDrawRect *data = &cmd->data.draw_rect;

                        soft_cmd_draw_rect(
                            data->framebuffer,
                            f32x4_to_i32(data->dst),
                            data->bg_color,
                            data->corner_radii,
                            data->border_color,
                            data->border_widths
                        );
                        break;
                    }
                    case RENDER_CMD_TYPE_DRAW_TEXT: {
                        RenderCmdDrawText* data = &cmd->data.draw_text;
                        i32x4 px_buf_dst = f32x4_to_i32(data->dst);

                        string_view str_view = {
                            .start = data->text,
                            .len = data->text_len
                        };

                        soft_cmd_draw_text(
                            data->framebuffer,
                            px_buf_dst,
                            str_view,
                            data->font,
                            data->font_height_px,
                            data->letter_spacing_px,
                            data->color
                        );

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
