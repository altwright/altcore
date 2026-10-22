//
// Created by wright on 10/21/26.
//

#include "texture.h"

#include <string.h>

#include "pixels.h"
#include "framebuffer.impl.h"
#include "../debug.h"
#include "../memory.h"
#include "../arenas.h"
#include "../maths.h"
#include "../../libs/stb_image.h"

rgba32f texture_sample(Framebuffer *texture, f32x2 coord, TextureSampleInfo info) {
    rgba32f sample = {};

    coord.u = SATURATE(coord.u);
    coord.v = SATURATE(coord.v);

    FramebufferInfo texture_info = framebuffer_get_info(texture);
    u8* bytes = nullptr;
    i64 pitch = 0;
    framebuffer_impl_get_bytes(texture, &bytes, &pitch);

    FramebufferPixelInfo* pixel_buf_info = &texture_info.data.pixel;
    i32 p_x = (i32) (((f32) pixel_buf_info->size.x * coord.u) + 0.5f);
    i32 p_y = (i32) (((f32) pixel_buf_info->size.y * coord.v) + 0.5f);

    i64 px_size = pixel_size(pixel_buf_info->format);

    pixel_set(
        &(Pixel){
            .format = PIXEL_FORMAT_RGBA32F,
            .start = (u8 *) sample.data,
        },
        &(Pixel){
            .format = pixel_buf_info->format,
            .start = bytes + p_y * pitch + p_x * px_size
        }
    );

    return sample;
}
