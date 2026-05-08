//
// Created by wright on 5/8/26.
//

#include "barrier.h"

#include <threads.h>
#include <stdatomic.h>

#include "memory.h"

struct BARRIER_T {
    mtx_t lock;
    cnd_t fence_cond;
    i32 remaining_threads;
    i32 total_threads;
    u32 generation;
};

Barrier *barrier_create(BarrierCreateInfo *info) {
    Barrier *barrier = alt_malloc(sizeof(*barrier));
    *barrier = (Barrier){};

    mtx_init(&barrier->lock, mtx_plain);
    cnd_init(&barrier->fence_cond);

    barrier->remaining_threads = barrier->total_threads = info->expected_threads;
    barrier->generation = 0;

    return barrier;
}

void barrier_wait(Barrier *barrier) {
    mtx_lock(&barrier->lock);

    u32 current_generation = barrier->generation;

    barrier->remaining_threads--;

    if (barrier->remaining_threads <= 0) {
        barrier->remaining_threads = barrier->total_threads;
        barrier->generation++;
        cnd_broadcast(&barrier->fence_cond);
    } else {
        while (current_generation == barrier->generation) {
            cnd_wait(&barrier->fence_cond, &barrier->lock);
        }
    }

    mtx_unlock(&barrier->lock);
}

void barrier_destroy(Barrier *barrier) {
    mtx_destroy(&barrier->lock);
    cnd_destroy(&barrier->fence_cond);

    alt_free(barrier);
}
