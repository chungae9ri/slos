#include <stdint.h>
#include <string.h>
#include <reg.h>
#include <arch/qgic.h>
#include <irqs.h>
#include <timer.h>
#include <task.h>
#include <msm8226-clock.h>
#include <acpuclock.h>
#include <uart_dm.h>
#include <debug.h>

extern enable_interrupt(void);
extern disable_interrupt(void);

extern struct task_struct task_arr[MAX_TASK];
extern uint64_t	jiffies;
extern struct task_struct *current;
extern uint32_t show_stat;

void cpuidle(void)
{
	while(1) {
		drop_usrtask();
		if (show_stat) print_msg("cpuidle running....\r\n");
	}
}

int get_uart_input() 
{
	int byte;
	byte = uart_getc(0, 1);
	return byte;
}

void target_early_init(void)
{
 	uart_dm_init(1, 0, BLSP1_UART2_BASE);
}

void platform_init()
{
	platform_clock_init();
	platform_qgic_init();
	platform_init_timer();
}

void core_init()
{
	init_jiffies();
	init_idletask();
}

void kernel_main_ctl(void)
{
	disable_interrupt();
	platform_init();
	target_early_init();
	core_init();
	init_shell();
	enable_interrupt();
	load_ramdisk();
	cpuidle();
}
