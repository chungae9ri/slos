// SPDX-License-Identifier: MIT OR Apache-2.0
//
// Copyright (c) 2024 kwangdo.yi<kwangdo.yi@gmail.com>

#ifndef _RAMDISK_IO_H_
#define _RAMDISK_IO_H_

#include <stdint.h>

#define RAMDISK_START           (0x03000000) /* 48MiB */
#define RAMDISK_SIZE            (0x400000)   /* 4MiB */
#define RAMDISK_BLK_SIZE        (0x200)      /* 512B */
#define RAMDISK_BLK_SIZE_SHIFT  (9)
#define RAMDISK_PAGE_SIZE       (0x1000) /* 4KiB */
#define RAMDISK_PAGE_SIZE_SHIFT (12)
#define RAMDISK_BLK_NUM         (RAMDISK_SIZE / RAMDISK_BLK_SIZE)
#define RAMDISK_PAGE_NUM        (RAMDISK_SIZE / RAMDISK_PAGE_SIZE)

typedef int (*flash_erase_chip)(void);
typedef int (*flash_erase_page)(uint32_t page);
typedef int (*flash_erase_addr_len)(uint32_t addr, uint32_t len);
typedef int (*flash_write)(uint32_t addr, uint32_t len, const uint8_t *buf);
typedef int (*flash_read)(uint32_t addr, uint32_t len, uint8_t *buf);

struct ramdisk_io_ops {
	flash_erase_chip erase_chip;
	flash_erase_page erase_page;
	flash_erase_addr_len erase_addr_len;
	flash_write write;
	flash_read read;
};

extern struct ramdisk_io_ops io_ops;
#endif
