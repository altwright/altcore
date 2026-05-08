//
// Created by wright on 5/8/26.
//

#include "fb_transition.h"
#include "../../memory.h"
#include "../../barrier.h"

typedef struct SOFT_RESOLVE_TASK_ARG_T {
    Framebuffer* framebuffer;
    Barrier *barrier;
    bool is_primary_worker;
} SoftResolveTaskArg;

static void soft_fb_transition_task(void* arg) {
    SoftResolveTaskArg* resolve_arg = arg;

    barrier_wait(resolve_arg->barrier);

    if (resolve_arg->is_primary_worker) {
        barrier_destroy(resolve_arg->barrier);
    }

    alt_free(resolve_arg);
}

void soft_renderer_fb_transition(Framebuffer* fb, Worker** workers, i32 workers_len) {
    BarrierCreateInfo barrier_create_info = {
        .expected_threads = workers_len,
    };

    Barrier *barrier = barrier_create(&barrier_create_info);

    for (i32 worker_idx = 0; worker_idx < workers_len; worker_idx++) {
        SoftResolveTaskArg* task_arg = alt_malloc(sizeof(*task_arg));
        *task_arg = (SoftResolveTaskArg) {
            .framebuffer = fb,
            .barrier = barrier,
        };

        if (worker_idx == 0) {
            task_arg->is_primary_worker = true;
        }

        Task task = {
            .fn_ptr = soft_fb_transition_task,
            .arg = task_arg,
        };

        worker_push_task(workers[worker_idx], &task);
    }
}
