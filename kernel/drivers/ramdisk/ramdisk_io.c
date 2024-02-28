// SPDX-License-Identifier: MIT OR Apache-2.0
//
// Copyright (c) 2024 kwangdo.yi<kwangdo.yi@gmail.com>

/* NOTE: ramdisk simulation for flash device.
 *       1page = 4KiB, erasable,
 *       datablk = 512B
 *	 ramdisk size = 4MiB
 */


#include <error.h>
#include <ramdisk_io.h>

static int erase_ramdisk_chip(void)
{
	int i;

	for (i = 0; i < RAMDISK_SIZE; i++)
		((uint8_t *)(RAMDISK_START))[i] = 0xFF;

	return NO_ERR;
}

static int erase_ramdisk_page(uint32_t page)
{
	int i;

	for (i = 0; i < RAMDISK_PAGE_SIZE; i++)
		((uint8_t *)(RAMDISK_START + page * RAMDISK_PAGE_SIZE))[i] = 0xFF; 

	return NO_ERR;
}

static int write_ramdisk(uint32_t addr, uint32_t len, const uint8_t *buf)
{
	uint32_t i;
	uint32_t offset = addr;

	if ((addr + len) > (RAMDISK_SIZE))
		return -1;

	for (i = 0; i < len; i++)
		((uint8_t *)(RAMDISK_START + offset))[i] = buf[i];

	return NO_ERR;
}

static int read_ramdisk(uint32_t addr, uint32_t len, uint8_t *buf)
{
	uint32_t i;
	uint32_t offset = addr;

	if ((offset + len) > (RAMDISK_SIZE))
		return -1;

	for (i = 0; i < len; i++)
		buf[i] = ((uint8_t *)(RAMDISK_START + offset))[i];

	return NO_ERR;
}

static int erase_ramdisk(uint32_t addr, uint32_t len)
{
	uint32_t start_pg;
	uint32_t pg_cnt;
	uint32_t i;
	uint32_t j;

	start_pg = (uint32_t)(addr / RAMDISK_PAGE_SIZE);
	pg_cnt = (uint32_t)(len / RAMDISK_PAGE_SIZE);

	for (i = 0; i < pg_cnt; i++) {
		for (j = 0; j < RAMDISK_PAGE_SIZE; j++) {
			((uint8_t *)(start_pg + i * RAMDISK_PAGE_SIZE))[j] = 0xFF; 
		}
	}

	return NO_ERR;
}

struct ramdisk_io_ops io_ops = {
	.erase_chip = erase_ramdisk_chip,
	.erase_page = erase_ramdisk_page,
	.erase_addr_len = erase_ramdisk,
	.write = write_ramdisk,
	.read = read_ramdisk,
};
