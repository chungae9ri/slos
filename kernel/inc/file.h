/*
  kernel/core/file.h slfs
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

#ifndef _FILE_
#define _FILE_
#include <stdint.h>

struct file {
	struct file_system *pfs;
	uint32_t fd;
	uint32_t pos;
	uint32_t fsz;
	uint32_t oCnt;
	char name[128];
};

struct file *create_file(int _fd, char *str);
struct file *open_file(char *str);
uint32_t close_file(struct file *fp);
uint32_t delete_file(struct file *fp);
uint32_t read(struct file *fp, uint32_t _n, char *_buf);
uint32_t write(struct file *fp, uint32_t _n, char * _buf);
void reset(struct file *fp);
void rewrite(struct file *fp);
uint32_t is_eof(struct file *fp);
#endif
