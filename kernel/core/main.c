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
