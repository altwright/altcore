//
// Created by wright on 5/10/26.
//

#ifndef ALTCORE_WINDOW_IMPL_H
#define ALTCORE_WINDOW_IMPL_H

#include <SDL3/SDL_video.h>
#include "window.h"

WindowHandle* window_impl_get_handle_from_id(SDL_WindowID id);

#endif //ALTCORE_WINDOW_IMPL_H
