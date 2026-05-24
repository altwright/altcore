//
// Created by wright on 5/23/26.
//

#include <string.h>

#include "fonts.h"

#include "../libs/stb_truetype.h"
#include "../memory.h"

struct FONT_HANDLE_T {
    char *filepath;
    u8 *ttf;
    stbtt_fontinfo info;
};

FontHandle *font_load_ttf(Filepath* path) {
    FileHandle* file = fs_file_open(path, FILE_HANDLE_MODE_BINARY_READ);
    if (!file) {
        return nullptr;
    }

    Arena* arena = arena_make(4 * 1024ULL);

    u8s ttf = fs_file_to_buf(arena, file);

    fs_file_close(file);

    FontHandle* font = alt_malloc(sizeof(*font));

    string abs_filepath_str = fs_path_get_abs(arena, path);

    char* abs_filepath_buf = alt_calloc(abs_filepath_str.len + 1, sizeof(char));
    memcpy(abs_filepath_buf, abs_filepath_str.data, abs_filepath_str.len);

    font->filepath = abs_filepath_buf;

    u8* ttf_buf = alt_calloc(ttf.len, sizeof(u8));
    memcpy(ttf_buf, ttf.data, ttf.len);

    font->ttf = ttf_buf;

    int success = stbtt_InitFont(&font->info, font->ttf, 0);
    if (!success) {
        return nullptr;
    }

    arena_free(arena);

    return font;
}

void font_unload(FontHandle *font) {
    alt_free(font->filepath);
    alt_free(font->ttf);
    alt_free(font);
}
