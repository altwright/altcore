//
// Created by wright on 2/18/26.
//

#ifndef ALTCORE_DEFER_H
#define ALTCORE_DEFER_H

extern bool g_end_of_defer;

#ifndef DEFER
#define DEFER(fn_ptr, ...) \
for(g_end_of_defer = false; !g_end_of_defer; g_end_of_defer = true, fn_ptr(__VA_ARGS__))
#endif

#endif //ALTCORE_DEFER_H
