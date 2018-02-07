/*
  kernel/core/main.c main start kernel
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

#include <xil_printf.h>
#include <gic.h>
#include <ktimer.h>
#include <timer.h>
#include <task.h>
#include <wait.h>
#include <mm.h>

extern uint32_t show_stat;

void cpuidle(void)
{
	uint32_t i = 0;
	xil_printf("I am cpuidle.....\n");
	/* do nothing for now */
	while (1) {
		if (show_stat) {
			xil_printf("cpuidle is running....\n");
		}
		if (i == 0xFFFFFFF) i = 0;
		else i++;
	}
}

int start_kernel(void) 
{
	struct framepool fp;
	struct pagetable pgt;
	struct vmpool kheap;

	init_kernmem(&fp, &pgt, &kheap);
	init_gic();
	init_idletask();
	init_rq();
	init_wq();
	init_shell();
	init_timertree();
	init_cfs_scheduler();
	init_timer();
	update_csd();
	timer_enable();
	cpuidle();

	return 0;
}
