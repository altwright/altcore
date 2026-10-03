//
// Created by wright on 10/3/26.
//

#ifndef BRITANNICUS_DRAW_TEXT_IMPL_H
#define BRITANNICUS_DRAW_TEXT_IMPL_H

#include "../../types.h"
#include "../../strings.h"
#include "../fonts.h"

f32x2 draw_text_impl_get_start_cursor(string_view text,
                                      FontSets *font_sets,
                                      FontStyle font_style,
                                      i32 font_height_px,
                                      f32 row_height);

#endif //BRITANNICUS_DRAW_TEXT_IMPL_H
