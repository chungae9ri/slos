/* SPDX-License-Identifier: MIT OR Apache-2.0
 *
 * Copyright (c) 2024 kwangdo.yi<kwangdo.yi@gmail.com>
 */

/**
 * @addtogroup kernel
 * @{
 * @addtogroup kernel_drivers Drivers
 * @{
 * @addtogroup kernel_drivers_rd Ramdisk driver
 * @{
 *
 * @brief Ramdisk driver implemented
 *
 * ramdisk simulation for flash device.
 * 1page = 4KiB, erasable,
 * datablk = 512B
 * ramdisk size = 4MiB
 */

#ifndef _RAMDISK_IO_H_
#define _RAMDISK_IO_H_

#include <stdint.h>

/** Ramdisk start address */
#define RAMDISK_START (0x03000000) /* 48MiB */
/** Ramdisk size */
#define RAMDISK_SIZE (0x400000) /* 4MiB */
/** Ramdisk block size */
#define RAMDISK_BLK_SIZE (0x200) /* 512B */
/** Ramdisk block size bit shift */
#define RAMDISK_BLK_SIZE_SHIFT (9)
/** Ramdisk page size */
#define RAMDISK_PAGE_SIZE (0x1000) /* 4KiB */
/** Ramdisk page size bit shift */
#define RAMDISK_PAGE_SIZE_SHIFT (12)
/** Ramdisk block number */
#define RAMDISK_BLK_NUM (RAMDISK_SIZE / RAMDISK_BLK_SIZE)
/** Ramdisk page number */
#define RAMDISK_PAGE_NUM (RAMDISK_SIZE / RAMDISK_PAGE_SIZE)

/** flash erase chip operation function type */
typedef int (*flash_erase_chip)(void);
/** flash erase page operation function type */
typedef int (*flash_erase_page)(uint32_t page);
/** flash erase from addr to lenghth operation function type */
typedef int (*flash_erase_addr_len)(uint32_t addr, uint32_t len);
/** flash write operation function type */
typedef int (*flash_write)(uint32_t addr, uint32_t len, const uint8_t *buf);
/** flash read operation function type */
typedef int (*flash_read)(uint32_t addr, uint32_t len, uint8_t *buf);

/** Ramdisk IO operation interface structure */
struct ramdisk_io_ops {
	flash_erase_chip erase_chip;
	flash_erase_page erase_page;
	flash_erase_addr_len erase_addr_len;
	flash_write write;
	flash_read read;
};

extern struct ramdisk_io_ops io_ops;
#endif

/**
 * @}
 * @}
 * @}
 *
 */
