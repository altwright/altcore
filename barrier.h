//
// Created by wright on 5/8/26.
//

#ifndef ALTCORE_BARRIER_H
#define ALTCORE_BARRIER_H

#include "types.h"

struct BARRIER_T;
typedef struct BARRIER_T Barrier;

typedef struct BARRIER_CREATE_INFO_T {
    i32 expected_threads;
} BarrierCreateInfo;

Barrier *barrier_create(BarrierCreateInfo *info);

void barrier_wait(Barrier *barrier);

void barrier_destroy(Barrier *barrier);

#endif //ALTCORE_BARRIER_H
