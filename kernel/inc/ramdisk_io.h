/*
  kernel/inc/ramdisk_io.h
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

#ifndef _RAMDISK_IO_
#define _RAMDISK_IO_

#include <file_system.h>

#define RAMDISK_START		0x03000000  /* 48MB */
#define RAMDISK_SIZE		0x400000 /* 4MB */
#define TOTAL_BLK_NUM		(RAMDISK_SIZE / DATA_BLK_SIZE)

void write_ramdisk(int mem_blk_num, char *buf);
void read_ramdisk(int mem_blk_num, char *buf);
#endif
