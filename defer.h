//
// Created by wright on 2/18/26.
//

#ifndef ALTCORE_DEFER_H
#define ALTCORE_DEFER_H

void defer_init();

void defer_uninit();

void defer_lock_false();

void defer_lock_true();

bool defer_is_false_unlock();

#ifndef DEFER
#define DEFER(...) \
for(defer_lock_false(); defer_is_false_unlock(); defer_lock_true(), __VA_ARGS__)
#endif

#endif //ALTCORE_DEFER_H
