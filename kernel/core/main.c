#include <xil_printf.h>
#include <gic.h>
#include <ktimer.h>
#include <timer.h>
#include <task.h>
#include <wait.h>

extern uint32_t show_stat;

void cpuidle(void)
{
	xil_printf("I am cpuidle.....\n");
	/* do nothing for now */
	while (1) {
		if (show_stat) {
			xil_printf("cpuidle is running....\n");
		}
	}
}

int main(void) 
{
	init_gic();
	init_idletask();
	init_rq();
	init_wq();
	init_shell();
	init_cfs_workers();
	init_timertree();
	init_cfs_scheduler();
	init_timer();
	update_csd();
	timer_enable();
	cpuidle();

	return 0;
}
