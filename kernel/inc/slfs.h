/* SPDX-License-Identifier: MIT OR Apache-2.0
 *
 * Copyright (c) 2024 kwangdo.yi<kwangdo.yi@gmail.com>
 */

/**
 * @addtogroup kernel
 * @{
 * @addtogroup kernel_fs File system
 * @{
 * @addtogroup kernel_fs_slfs SLFS file system
 * @{
 *
 * @file
 *
 * @brief SLFS file system functions
 *
 */

#ifndef _SLFS_H_
#define _SLFS_H_

#include <stdint.h>
#include <ramdisk_io.h>

/** SLFS file name length */
#define SLFS_FNAME_LEN (16)

/** SLFS file type */
typedef struct {
	uint32_t fd;		 /**< file id */
	uint32_t pos;		 /**< file pointer position */
	uint32_t file_size;	 /**< Used for file update starts (OPEN evt) flag */
	uint32_t open_cnt;	 /**< Open count */
	uint32_t datablk_addr;	 /**< Used for file update ends (CLOSE evt) flag  */
	uint32_t inode_idx;	 /**< inode index */
	uint32_t allocedblk_num; /**< Allocated block number */
	uint32_t update_cnt;	 /**< Only the biggest one is valid, others are outdated inode*/
	uint8_t name[SLFS_FNAME_LEN]; /**< File name */
} slfs_file_t;

/** File pointer origin */
typedef enum {
	SLFS_SEEK_SET = 0,
	SLFS_SEEK_CUR = 1,
	SLFS_SEEK_END = 2,
} slfs_fseek_t;

/**
 * @brief SLFS file system mount
 *
 * @return int 0 for success
 */
int slfs_mount(void);

/**
 * @brief SLFS file system unmount
 *
 * @return int 0 for success
 */
int slfs_umount(void);

/**
 * @brief SLFS file system format
 *
 * @return int 0 for success
 */
int slfs_format(void);

/**
 * @brief SLFS file open
 *
 * @param [in] pname File name
 * @param [in] pf File pointer
 * @return int 0 for success
 */
int slfs_open(const uint8_t *pname, slfs_file_t *pf);

/**
 * @brief Set SLFS file pointer
 *
 * @param [in] pf File pointer
 * @param [in] offset Offset from origin
 * @param [in] whence File pointer origin
 * @return int 0 for success
 */
int slfs_seek(slfs_file_t *pf, uint32_t offset, slfs_fseek_t whence);

/**
 * @brief File write
 *
 * @param [in] pf File pointer
 * @param [in] pbuf Buffer to be written
 * @param [in] len Write length
 * @return int 0 for success
 */
int slfs_write(slfs_file_t *pf, const uint8_t *pbuf, uint32_t len);

/**
 * @brief File read
 *
 * @param [in] pf File pointer
 * @param [in] pbuf Buffer to store read data
 * @param [in] len Read length
 * @return int 0 for success
 */
int slfs_read(slfs_file_t *pf, uint8_t *pbuf, uint32_t len);

/**
 * @brief File close
 *
 * @param [in] pf File pointer
 * @return int 0 for success
 */
int slfs_close(slfs_file_t *pf);

/**
 * @brief File delete
 *
 * @param [in] pf File pointer
 * @return int 0 for success
 */
int slfs_delete(slfs_file_t *pf);

/**
 * @brief Get next file in the file list
 *
 * @param [in] pf File pointer
 * @param [in] pinode_loc inode location
 * @return int 0 for success
 */
int slfs_get_next_file(slfs_file_t *pf, uint32_t *pinode_loc);
#endif

/**
 * @}
 * @}
 * @}
 *
 */
