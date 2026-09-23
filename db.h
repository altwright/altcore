//
// Created by wright on 9/11/26.
//

#ifndef ALTCORE_DB_H
#define ALTCORE_DB_H

#include "types.h"
#include "arenas.h"

typedef enum DB_MOUNT_E : u32 {
#define X_DB_MOUNTS \
    X(RO) \
    X(RW) \
    X(COUNT)
#define X(mount) \
    DB_MOUNT_##mount,
    X_DB_MOUNTS
} DbMount;

typedef struct DB_FOLDER_T {
    const char **nested_path;
    i64 nested_path_len;
} DbFolder;

typedef struct DB_READ_INFO_T {
    DbMount mount;
    DbFolder folder;
    const char *filename;
} DbReadInfo;

typedef struct DB_WRITE_INFO_T {
    DbFolder folder;
    const char *filename;

    struct {
        const u8 *data;
        i64 len;
    } bytes;
} DbWriteInfo;

typedef struct DB_READ_STREAM_T DbReadStream;

typedef struct DB_WRITE_STREAM_T DbWriteStream;

void db_init();

void db_deinit();

bool db_exists(const DbReadInfo* info);

u8s db_read(Arena *arena, const DbReadInfo *info);

void db_write(const DbWriteInfo *info);

DbReadStream *db_read_open(const DbReadInfo *info);

u64 db_read_next(DbReadStream *stream, u8 *out_bytes, u64 out_bytes_len);

void db_read_close(DbReadStream *stream);

/*
 * The size of the byte buffer to be written is read from
 * the info parameter - none of the contents of the byte
 * buffer are copied during the stream handle creation.
 */
DbWriteStream *db_write_open(const DbWriteInfo *info);

u64 db_write_next(DbWriteStream *stream, const u8 *in_bytes, u64 in_bytes_len);

void db_write_close(DbWriteStream *stream);

#endif //ALTCORE_DB_H
