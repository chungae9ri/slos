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
  along with this program; if not, see <http://www.gnu.org/licenses/>
*/

#include <xil_printf.h>
#include <gic.h>
#include <ktimer.h>
#include <timer.h>
#include <task.h>
#include <sched.h>
#include <runq.h>
#include <waitq.h>
#include <mm.h>
#include <file_system.h>
#include <loader.h>
#include <dma.h>
#include <odev.h>

extern uint32_t show_stat;
extern void secondary_reset();

void cpuidle(void)
{
	uint32_t i = 0;
	uint32_t A9_rst_ctrl;
	xil_printf("I am cpuidle.....\n");

	while (1) {
		if (show_stat) {
			xil_printf("cpuidle is running....\n");
		}
#if 1
		if (i == 0) {
			A9_rst_ctrl = *(volatile uint32_t *)(0xF8000244);
			A9_rst_ctrl |= (0x22); // set clock disable, reset bit for cpu1
			*(volatile uint32_t *)(0xF8000244) = A9_rst_ctrl;

			*(volatile uint32_t *) (0x0) = *(volatile uint32_t *)(secondary_reset);
			*(volatile uint32_t *) (0x4) = *(volatile uint32_t *)(secondary_reset + 4);
			*(volatile uint32_t *) (0x8) = (uint32_t)(0x200000);

			while (i < 1000)
				i++;

			// release cpu1 reset
			A9_rst_ctrl &= ~(0x2);
			*(volatile uint32_t *)(0xF8000244) = A9_rst_ctrl;
			// release cpu1 clock
			A9_rst_ctrl &= ~(0x20);
			*(volatile uint32_t *)(0xF8000244) = A9_rst_ctrl;
		}
#endif

		if (i == 0xFFFFFFF) i = 1;
		else i++;
		asm ("DSB" :::);
		asm ("WFI" :::);
	}
}

int secondary_start_kernel(void)
{
	xil_printf("I am secondary cpu!\n");

	while (1);

	return 0;
}

int start_kernel(void) 
{
	struct framepool framepool;
	struct pagetable pgt;
	struct vmpool kheap;

	init_kernmem(&framepool, &pgt, &kheap);
	xil_printf("### init_kernmem done.\n");
	init_gic();
	init_idletask();

	init_file_system();
	mount_file_system();
	format_file_system();
	xil_printf("### mount slfs file system.\n");

	init_rq();
	init_wq();
	init_shell();
	init_timertree();
	init_cfs_scheduler();
	init_timer();
	update_csd();
	timer_enable();

	create_ramdisk_fs();
	xil_printf("### load user app to slfs.\n");
	init_dma();
	init_odev();
	create_workq_worker();
	cpuidle();

	return 0;
}
