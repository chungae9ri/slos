/*
  kernel/core/ramdisk_io.c
  (C) 2018 Kwangdo Yi <kwangdo.yi@gmail.com>
 
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, see <http://www.gnu.org/licenses/>
*/

/* NOTE: ramdisk simulation for flash device.
 *       1page = 4KiB, erasable,
 *       datablk = 512B
 *	 ramdisk size = 4MiB
 */


#include <ramdisk_io.h>

static int erase_ramdisk_chip(void)
{
	int i;

	for (i = 0; i < RAMDISK_SIZE; i++)
		((uint8_t *)(RAMDISK_START))[i] = 0xFF;

	return 0;
}

static int erase_ramdisk_page(uint32_t page)
{
	int i;

	for (i = 0; i < RAMDISK_PAGE_SIZE; i++)
		((uint8_t *)(RAMDISK_START + page * RAMDISK_PAGE_SIZE))[i] = 0xFF; 

	return 0;
}

static int read_ramdisk_blk(uint32_t blk, uint8_t *buf)
{
	int i;

	for (i = 0; i < RAMDISK_BLK_SIZE; i++)
		buf[i] = ((uint8_t *)(RAMDISK_START + blk * RAMDISK_BLK_SIZE))[i];

	return 0;
}

static int write_ramdisk_blk(uint32_t blk, uint8_t *buf)
{
	int i;

	for (i = 0; i < RAMDISK_BLK_SIZE; i++)
		((uint8_t *)(RAMDISK_START + blk * RAMDISK_BLK_SIZE))[i] = buf[i];

	return 0;
}

static int write_ramdisk(uint32_t addr, uint32_t len, uint8_t *buf)
{
	uint32_t i;
	uint32_t offset = addr;

	if ((addr + len) > (RAMDISK_SIZE))
		return -1;

	for (i = 0; i < len; i++)
		((uint8_t *)(RAMDISK_START + offset))[i] = buf[i];

	return 0;
}

static int read_ramdisk(uint32_t addr, uint32_t len, uint8_t *buf)
{
	uint32_t i;
	uint32_t offset = addr;

	if ((offset + len) > (RAMDISK_SIZE))
		return -1;

	for (i = 0; i < len; i++)
		buf[i] = ((uint8_t *)(RAMDISK_START + offset))[i];

	return 0;
}

struct ramdisk_io_ops io_ops = {
	.erase_chip = erase_ramdisk_chip,
	.erase_page = erase_ramdisk_page,
	.write = write_ramdisk,
	.read = read_ramdisk,
	.write_blk = write_ramdisk_blk,
	.read_blk = read_ramdisk_blk,
};

