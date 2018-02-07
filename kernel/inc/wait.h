/*
  kernel/inc/wait.h 
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
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include <stdint-gcc.h>
#include <task.h>

struct wait_queue {
	int magic;
	volatile uint32_t queuelock;
	struct list_head task_list;
	int count;
};

void init_wq(void);
void add_to_wq(struct task_struct *tp);
void remove_from_wq(struct task_struct *tp);
