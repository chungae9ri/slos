/*
  kernel/core/wait.c process wait manager 
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

#include <task.h>
#include <wait.h>
#include <stdbool.h>
#include <defs.h>

struct wait_queue wq;

void init_wq(void)
{
	wq.magic = 1;
	wq.task_list.next = NULL;
	wq.task_list.prev = NULL;
	wq.count = 0;
}

void wake_all_wq(void)
{
	struct list_head *cur, *next;

	cur = wq.task_list.next;
	while (cur) {
		next = cur->next;
		cur->next = NULL;
		cur->prev = NULL;
		cur = next;
	}
	wq.count = 0;
}

void destroy_wq(void)
{
	wake_all_wq();
	wq.magic = 0;
}

void add_to_wq(struct task_struct *p)
{
	struct list_head *cur_p, *temp_p;

	cur_p = wq.task_list.next;
	temp_p = &wq.task_list;

	while (cur_p) {
		temp_p = cur_p;
		cur_p = cur_p->next;
	}
	temp_p->next = &p->waitlist;
	p->waitlist.prev = temp_p;
	p->waitlist.next = NULL;
	wq.count++;
}

void remove_from_wq(struct task_struct *p)
{
	struct list_head *cur_p;
	struct task_struct *pt;

	cur_p = wq.task_list.next;
	while (cur_p) {
		pt = container_of(cur_p, struct task_struct, waitlist);
		if (pt == p) {
			pt->waitlist.prev->next = pt->waitlist.next;
			pt->waitlist.next->prev = pt->waitlist.prev;
			pt->waitlist.prev = NULL;
			pt->waitlist.next = NULL;
			wq.count--;
			break;
		} else {
			cur_p = cur_p->next;
		}
	}
}

int wait_queue_block(unsigned long timeout)
{
	return 0;
}

int wait_queue_wake_one(bool resched)
{
	return 0;
}

int wait_queue_wake_all(bool resched)
{
	return 0;
}
