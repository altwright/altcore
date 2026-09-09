//
// Created by wright on 9/9/26.
//

#ifndef ALTCORE_CLOCK_H
#define ALTCORE_CLOCK_H

#include "types.h"

void clock_init();

void clock_deinit();

u64 clock_get_elapsed_ns();

#endif //ALTCORE_CLOCK_H
