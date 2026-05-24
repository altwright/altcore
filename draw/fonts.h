//
// Created by wright on 5/23/26.
//

#ifndef ALTCORE_FONTS_H
#define ALTCORE_FONTS_H

#include "../types.h"
#include "../fs.h"

struct FONT_HANDLE_T;
typedef struct FONT_HANDLE_T FontHandle;

FontHandle *font_load_file(const Filepath* path);

void font_unload(FontHandle *font);

#endif //ALTCORE_FONTS_H
