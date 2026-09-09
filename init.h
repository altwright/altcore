//
// Created by wright on 5/1/26.
//

#ifndef ALTCORE_INIT_H
#define ALTCORE_INIT_H

#include "types.h"

typedef struct ALT_INIT_INFO_T {
    i64 max_address_space_size;
} AltInitInfo;

void alt_init(const AltInitInfo* init_info);

void alt_deinit();

#endif //ALTCORE_INIT_H
