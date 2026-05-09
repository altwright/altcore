//
// Created by wright on 5/3/26.
//

#ifndef ALTCORE_WORKER_H
#define ALTCORE_WORKER_H

#include "types.h"

struct WORKER_T;
typedef struct WORKER_T Worker;

typedef void (*TaskFunc)(void *arg);

typedef struct TASK_T {
    TaskFunc fn_ptr;
    void* arg;
} Task;

typedef struct WORKER_CREATE_INFO_T {
    i64 task_q_cap;
} WorkerCreateInfo;

Worker *worker_create(WorkerCreateInfo *info);

void worker_destroy(Worker *worker);

bool worker_push_task(Worker *worker, const Task *task);

i32 worker_get_task_count(Worker *worker);

#endif //ALTCORE_WORKER_H
