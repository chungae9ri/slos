// SPDX-License-Identifier: MIT OR Apache-2.0
//
// Copyright (c) 2024 kwangdo.yi<kwangdo.yi@gmail.com>

#ifndef _FS_H_
#define _FS_H_

#include <stdint.h>
#include <stddef.h>

#include <slfs.h>
#include <lfs.h>

typedef enum {
    SLFS_FILE_SYSTEM,
    LITTLEFS_FILE_SYSTEM,
} FILE_SYSTEM_TYPE;

typedef struct {
    FILE_SYSTEM_TYPE fs_t;
    const uint8_t *name;
    size_t size;
    /* file pointer to slfs or lfs */
    void *virt_fp;
} file_t;

int32_t fs_mount(FILE_SYSTEM_TYPE fs);
int32_t fs_open(FILE_SYSTEM_TYPE fs_t, const uint8_t *name, file_t *fp, uint32_t flag);
int32_t fs_read(file_t *fp, uint8_t *buff, size_t len);
int32_t fs_write(file_t *fp, const uint8_t *buff, size_t len);
int32_t fs_close(file_t *fp);
int32_t fs_umount(FILE_SYSTEM_TYPE fs);
int32_t fs_seek(file_t *fp, uint32_t offset, int whence);
int32_t create_ramdisk_fs(FILE_SYSTEM_TYPE fs);
#endif
