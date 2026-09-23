//
// Created by wright on 9/11/26.
//

#include "db.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "memory.h"
#include "strings.h"
#include "debug.h"

struct DB_READ_STREAM_T {
    Arena *arena;
    FILE *fp;
};

struct DB_WRITE_STREAM_T {
    Arena *arena;
    FILE *fp;
};

static bool g_initialized = false;

static const char *kReadOnlyDbDirPath = "./data/ro";
static const char *kReadWriteDbDirPath = "./data/rw";

void db_init() {
    if (!g_initialized) {
        g_initialized = true;
    }
}

void db_deinit() {
    if (g_initialized) {
        g_initialized = false;
    }
}

static const char *get_mount_path(DbMount mount) {
    const char *mount_path = nullptr;
    switch (mount) {
        case DB_MOUNT_RO: {
            mount_path = kReadOnlyDbDirPath;
            break;
        }
        case DB_MOUNT_RW: {
            mount_path = kReadWriteDbDirPath;
            break;
        }
        default:
            crash_msg("Unhandled mount type %d\n", mount);
            break;
    }

    return mount_path;
}

static string build_folder_path(Arena *arena, const char **nested_folders, i64 nested_folders_len) {
    i64 total_string_len = 0;
    for (i64 nested_folder_idx = 0; nested_folder_idx < nested_folders_len; nested_folder_idx++) {
        total_string_len += strlen(nested_folders[nested_folder_idx]);
    }

    if (nested_folders_len > 0) {
        total_string_len += nested_folders_len - 1;
    }

    string folder_path = {
        .arena = arena,
        .cap = nested_folders_len + 1
    };
    ARRAY_MAKE(&folder_path);

    for (i64 nested_folder_idx = 0; nested_folder_idx < nested_folders_len; nested_folder_idx++) {
        string_cat(&folder_path, "%s", nested_folders[nested_folder_idx]);
        if (nested_folder_idx < nested_folders_len - 1) {
            string_cat(&folder_path, "/");
        }
    }

    return folder_path;
}

static string build_file_path(Arena *tmp, DbMount mount, const char **nested_folders, i64 nested_folders_len,
                              const char *filename) {
    const char *mount_path = get_mount_path(mount);
    string folder_path = build_folder_path(tmp, nested_folders, nested_folders_len);
    return string_make(tmp, "%s/%s/%s", mount_path, folder_path.data, filename);
}

u8s db_read(Arena *arena, const DbReadInfo *info) {
    Arena *tmp = arena_make(KIBIBYTE);

    string file_path = build_file_path(tmp, info->mount, info->folder.nested_path, info->folder.nested_path_len,
                                       info->filename);

    FILE *fp = fopen(file_path.data, "rb");
    if (!fp) {
        crash_msg("Failed to open and read file %s\n", file_path.data);
    }

    fseek(fp, 0, SEEK_END);
    long file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    u8s file_bytes = {
        .arena = arena,
        .len = file_size
    };
    ARRAY_MAKE(&file_bytes);

    size_t bytes_read = fread(file_bytes.data, 1, file_size, fp);
    if (bytes_read != file_size) {
        crash_msg("Failed to read file %s: only read %u of %d bytes\n", file_path.data, bytes_read, file_size);
    }

    fclose(fp);

    arena_free(tmp);

    return file_bytes;
}

void db_write(const DbWriteInfo *info) {
    Arena *tmp = arena_make(KIBIBYTE);

    string file_path = build_file_path(tmp, DB_MOUNT_RW, info->folder.nested_path, info->folder.nested_path_len,
                                       info->filename);

    FILE *fp = fopen(file_path.data, "wb");
    if (!fp) {
        crash_msg("Failed to open and write file %s\n", file_path.data);
    }

    size_t bytes_written = fwrite(info->bytes.data, 1, info->bytes.len, fp);
    if (bytes_written != info->bytes.len) {
        crash_msg("Only %u bytes written for file %s of size %d\n", bytes_written, file_path.data, info->bytes.len);
    }

    fclose(fp);

    arena_free(tmp);
}

DbReadStream *db_read_open(const DbReadInfo *info) {
    DbReadStream *stream = alt_malloc(sizeof(*stream));

    *stream = (DbReadStream){
        .arena = arena_make(KIBIBYTE),
    };

    string file_path = build_file_path(stream->arena, info->mount, info->folder.nested_path,
                                       info->folder.nested_path_len,
                                       info->filename);

    stream->fp = fopen(file_path.data, "rb");
    if (!stream->fp) {
        crash_msg("Failed to open and read file %s for stream\n", file_path.data);
    }

    return stream;
}

u64 db_read_next(DbReadStream *stream, u8 *out_bytes, u64 out_bytes_len) {
    return fread(out_bytes, 1, out_bytes_len, stream->fp);
}

void db_read_close(DbReadStream *stream) {
    fclose(stream->fp);
    arena_free(stream->arena);
    alt_free(stream);
}

DbWriteStream *db_write_open(const DbWriteInfo *info) {
    DbWriteStream *stream = alt_malloc(sizeof(*stream));

    *stream = (DbWriteStream){
        .arena = arena_make(KIBIBYTE),
    };

    string file_path = build_file_path(stream->arena, DB_MOUNT_RW, info->folder.nested_path,
                                       info->folder.nested_path_len,
                                       info->filename);

    stream->fp = fopen(file_path.data, "wb");
    if (!stream->fp) {
        crash_msg("Failed to open and write file stream %s\n", file_path.data);
    }

    return stream;
}

u64 db_write_next(DbWriteStream *stream, const u8 *in_bytes, u64 in_bytes_len) {
    return fwrite(in_bytes, 1, in_bytes_len, stream->fp);
}

void db_write_close(DbWriteStream *stream) {
    fclose(stream->fp);
    arena_free(stream->arena);
    alt_free(stream);
}

bool db_exists(const DbReadInfo *info) {
    Arena *tmp = arena_make(KIBIBYTE);

    string file_path = build_file_path(tmp, info->mount, info->folder.nested_path, info->folder.nested_path_len,
                                       info->filename);

    bool exists = access(file_path.data, F_OK) == 0;

    arena_free(tmp);

    return exists;
}
