/*
  kernel/inc/defs.h 
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

#ifndef _DEFS_H_
#define _DEFS_H_

#define ARRAY_SIZE(x) (sizeof(x)/sizeof((x)[0]))

#define container_of(ptr, type, member) \
	((type *)((unsigned int)ptr-offsetof(type, member)))
#endif
