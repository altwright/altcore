//
// Created by wright on 5/10/26.
//

#ifndef ALTCORE_PIXELS_IMPL_H
#define ALTCORE_PIXELS_IMPL_H

#include <SDL3/SDL_pixels.h>
#include "pixels.h"

SDL_PixelFormat pixels_impl_to_sdl_format(PixelFormat format);

PixelFormat pixels_impl_from_sdl_format(SDL_PixelFormat format);

#endif //ALTCORE_PIXELS_IMPL_H
