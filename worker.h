//
// Created by wright on 5/3/26.
//

#ifndef ALTCORE_WORKER_H
#define ALTCORE_WORKER_H

#include "types.h"

struct WORKER_T;
typedef struct WORKER_T Worker;

typedef enum TASK_TYPE_E {
#ifndef X_TASK_TYPES
#define X_TASK_TYPES \
    X(FN_PTR) \
    X(COUNT)
#endif
#ifndef X
#define X(type) \
    TASK_TYPE_##type,
#endif
    X_TASK_TYPES
#undef X
} TaskType;

typedef void (*TaskFunc)(void *arg);

typedef struct TASK_T {
    TaskType type;

    union {
        struct {
            TaskFunc ptr;
            void* arg;
        } fn;
    } data;

    i32 priority;
    bool do_not_drop;
} Task;

typedef struct WORKER_CREATE_INFO_T {
    i64 task_q_cap;
    bool drop_task_if_q_full;
} WorkerCreateInfo;

Worker *worker_create(WorkerCreateInfo *info);

void worker_destroy(Worker *worker);

bool worker_push_task(Worker *worker, const Task *task);

#endif //ALTCORE_WORKER_H
