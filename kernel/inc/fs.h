/* SPDX-License-Identifier: MIT OR Apache-2.0
 *
 * Copyright (c) 2024 kwangdo.yi<kwangdo.yi@gmail.com>
 */

/**
 * @addtogroup kernel
 * @{
 * @addtogroup kernel_fs File system
 * @{
 *
 * @brief File system interface
 *
 */

#ifndef _FS_H_
#define _FS_H_

#include <stdint.h>
#include <stddef.h>

#include <slfs.h>
#include <lfs.h>

/** File system types */
typedef enum {
	SLFS_FILE_SYSTEM,     /**< SLFS file system type */
	LITTLEFS_FILE_SYSTEM, /**< Littlefs file system type */
} FILE_SYSTEM_TYPE;

/** File type */
typedef struct {
	FILE_SYSTEM_TYPE fs_t; /**< File system type */
	const uint8_t *name;   /**< File name */
	size_t size;	       /**< File size */
	void *virt_fp;	       /**< file pointer to slfs or lfs */
} file_t;

/**
 * @brief Mount file system
 *
 * @param [in] fs File system type to be mounted
 * @return int32_t 0 for success
 */
int32_t fs_mount(FILE_SYSTEM_TYPE fs);

/**
 * @brief File open
 *
 * @param [in] fs_t File system type
 * @param [in] name File name
 * @param [in] fp File pointer
 * @param [in] flag File open flag
 * @return int32_t 0 for success
 */
int32_t fs_open(FILE_SYSTEM_TYPE fs_t, const uint8_t *name, file_t *fp, uint32_t flag);

/**
 * @brief Read file contents
 *
 * @param [in] fp File pointer
 * @param [in] buff Buffer memory to store read data
 * @param [in] len Read size
 * @return int32_t 0 for success
 */
int32_t fs_read(file_t *fp, uint8_t *buff, size_t len);

/**
 * @brief Write data to file
 *
 * @param [in] fp File pointer
 * @param [in] buff Buffer memory to be written
 * @param [in] len Write size
 * @return int32_t 0 for success
 */
int32_t fs_write(file_t *fp, const uint8_t *buff, size_t len);

/**
 * @brief Close file
 *
 * @param [in] fp File pointer
 * @return int32_t 0 for success
 */
int32_t fs_close(file_t *fp);

/**
 * @brief Unmount file system
 *
 * @param [in] fs File system unmounted
 * @return int32_t 0 for success
 */
int32_t fs_umount(FILE_SYSTEM_TYPE fs);

/**
 * @brief Move file content pointer location
 *
 * @param [in] fp File pointer
 * @param [in] offset Offset size in byte
 * @param [in] whence Origination of offset
 * @return int32_t 0 for success
 */
int32_t fs_seek(file_t *fp, uint32_t offset, int whence);

/**
 * @brief Create file system with the contents of ramdisk
 *
 * @param [in] fs File system type to be used
 * @return int32_t 0 for success
 */
int32_t create_ramdisk_fs(FILE_SYSTEM_TYPE fs);
#endif

/**
 * @}
 * @}
 *
 */
