//
// Created by wright on 5/8/26.
//

#include "clear.h"

#include "../../memory.h"
#include "../../debug.h"

typedef struct SOFT_RENDERER_CLEAR_TASK_ARG_T {
    FramebufferData fb_data;
    i32 y_offset;
    i32 rows_per_worker;
    i32 num_remainder_rows;
    rgba8888 rgba;
} SoftRendererClearTaskArg;

static void soft_renderer_clear_task(void *arg) {
    SoftRendererClearTaskArg *clear_arg = arg;

    FramebufferData fb_data = clear_arg->fb_data;

    for (
        i32 row_idx = clear_arg->y_offset;
        row_idx < fb_data.size.y;
        row_idx += clear_arg->rows_per_worker
    ) {
        for (i32 col_idx = 0; col_idx < fb_data.size.x; col_idx++) {
            framebuffer_data_set_pixel(fb_data, col_idx, row_idx, clear_arg->rgba);
        }
    }

    // If worker has been assigned to clear the remainder
    if (clear_arg->num_remainder_rows > 0) {
        for (
            i32 row_idx = fb_data.size.y - clear_arg->num_remainder_rows;
            row_idx < fb_data.size.y;
            row_idx++
        ) {
            for (i32 col_idx = 0; col_idx < clear_arg->fb_data.size.x; col_idx++) {
                framebuffer_data_set_pixel(fb_data, col_idx, row_idx, clear_arg->rgba);
            }
        }
    }

    alt_free(clear_arg);
}

void soft_renderer_clear(FramebufferData fb_data, rgba8888 rgba, Worker *workers[], i32 workers_len) {
    i32 rows_per_worker = fb_data.size.y / workers_len;
    i32 rows_remainder = fb_data.size.y % workers_len;

    for (i32 worker_idx = 0; worker_idx < workers_len; worker_idx++) {
        SoftRendererClearTaskArg *arg = alt_malloc(sizeof(*arg));
        *arg = (SoftRendererClearTaskArg){
            .fb_data = fb_data,
            .y_offset = worker_idx,
            .rows_per_worker = rows_per_worker,
            .rgba = rgba
        };

        if (worker_idx == workers_len - 1) {
            arg->num_remainder_rows = rows_remainder;
        }

        Task clear_task = {
            .fn_ptr = soft_renderer_clear_task,
            .arg = arg,
        };

        worker_push_task(workers[worker_idx], &clear_task);
    }
}
