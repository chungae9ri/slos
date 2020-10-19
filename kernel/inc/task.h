/*
  kernel/inc/task.h 
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

#ifndef __TASK_H__
#define __TASK_H__

#include <stdint-gcc.h>
#include <stddef.h>
#include <mem_layout.h>
#include <sched.h>

#define MAX_WORKQ	32
#define WORKQ_DMA	0

struct workq {
	void (*func)(void *);
	void *arg;
};

struct worker {
	int enq_idx;
	int deq_idx;
	struct workq wkq[MAX_WORKQ];
};

struct worker qworker;

void init_idletask(void);
void init_cfs_workers(void);
void init_shell(void);
void init_workers(void);

void create_usr_cfs_task(char *name, 
		task_entry cfs_task, 
		uint32_t pri, 
		uint32_t appIdx);
void create_cfs_task(char *name, task_entry cfs_task, uint32_t pri);
void create_rt_task(char *name, task_entry handler, uint32_t dur);
void create_workq_worker(void);

#endif
