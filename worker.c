//
// Created by wright on 5/3/26.
//

#include "worker.h"

#include <assert.h>
#include <threads.h>
#include <stdatomic.h>

#include "memory.h"

constexpr i32 kDefaultTaskQueueCap = 32;

typedef struct TASK_QUEUE_T {
    Task *data;
    i32 head_idx, tail_idx, count;
    i32 cap;
    mtx_t lock;
    cnd_t not_empty;
    cnd_t not_full;
    atomic_bool shutdown;
} TaskQueue;

struct WORKER_T {
    thrd_t thread;
    TaskQueue task_q;
    bool block_if_full;
};

static void task_queue_push(TaskQueue *q, const Task *task) {
    q->data[q->tail_idx] = *task;
    q->tail_idx = (q->tail_idx + 1) % kDefaultTaskQueueCap;
    q->count++;
}

static Task task_queue_pop(TaskQueue *q) {
    Task task = q->data[q->head_idx];
    q->head_idx = (q->head_idx + 1) % kDefaultTaskQueueCap;
    q->count--;
    return task;
}

static int worker_thread_fn(void *arg) {
    Worker *worker = arg;

    TaskQueue *q = &worker->task_q;

    while (true) {
        mtx_lock(&q->lock);

        while (q->count == 0 && !atomic_load(&q->shutdown)) {
            cnd_wait(&q->not_empty, &q->lock);
        }

        if (q->count == 0 && atomic_load(&q->shutdown)) {
            mtx_unlock(&q->lock);
            return 0;
        }

        Task task = task_queue_pop(q);

        cnd_signal(&q->not_empty);

        mtx_unlock(&q->lock);

        task.fn_ptr(task.arg);
    }
}

Worker *worker_create(WorkerCreateInfo *info) {
    Worker *worker = alt_calloc(1, sizeof(Worker));
    assert(worker);

    i64 q_cap = kDefaultTaskQueueCap;

    if (info->task_q_cap > 0) {
        q_cap = info->task_q_cap;
    }

    TaskQueue *q = &worker->task_q;
    q->cap = q_cap;

    q->data = alt_calloc(q_cap, sizeof(Task));
    assert(q->data);

    mtx_init(&q->lock, mtx_plain);
    cnd_init(&q->not_empty);
    cnd_init(&q->not_full);

    thrd_create(&worker->thread, worker_thread_fn, worker);

    return worker;
}

void worker_destroy(Worker *worker) {
    TaskQueue *q = &worker->task_q;

    atomic_store(&q->shutdown, true);
    cnd_broadcast(&q->not_empty);
    cnd_broadcast(&q->not_full);

    thrd_join(worker->thread, nullptr);

    mtx_destroy(&q->lock);
    cnd_destroy(&q->not_empty);
    cnd_destroy(&q->not_full);

    alt_free(q->data);

    alt_free(worker);
}

bool worker_push_task(Worker *worker, const Task *task) {
    bool success = true;

    TaskQueue *q = &worker->task_q;
    mtx_lock(&q->lock);

    while (q->count == q->cap && !atomic_load(&q->shutdown)) {
        cnd_wait(&q->not_full, &q->lock);
    }

    if (atomic_load(&q->shutdown)) {
        mtx_unlock(&q->lock);
        return false;
    }

    task_queue_push(q, task);
    cnd_signal(&q->not_empty);
    mtx_unlock(&q->lock);

    return success;
}

i32 worker_get_task_count(Worker *worker) {
    i32 q_size = 0;

    mtx_lock(&worker->task_q.lock);

    q_size = worker->task_q.count;

    mtx_unlock(&worker->task_q.lock);

    return q_size;
}
