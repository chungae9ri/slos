extern "C" {
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
	#include <loader.h>
	#include <frame_pool.h>

	extern void enable_interrupt();
	extern void disable_interrupt();

	extern struct task_struct task_arr[MAX_TASK];
	extern uint64_t	jiffies;
	extern struct task_struct *current;
	extern uint32_t show_stat;

	void cpuidle(void) {
		while(1) {
			drop_usrtask();
			if (show_stat) print_msg("cpuidle running....\r\n");
		}
	}

	int get_uart_input() {
		int byte;
		byte = uart_getc(0, 1);
		return byte;
	}

	void target_early_init(void) {
		uart_dm_init(1, 0, BLSP1_UART2_BASE);
	}

	void platform_init() {
		platform_clock_init();
		platform_qgic_init();
		platform_init_timer();
	}

	void core_init() {
		init_jiffies();
		init_idletask();
	}

	void mem_init() {
		/* initialize frame pools */
		FramePool kernel_mem_pool(KERNEL_POOL_START_FRAME, KERNEL_POOL_SIZE, 0);
		unsigned long process_mem_pool_info_frame = kernel_mem_pool.get_frame();
		FramePool process_mem_pool(PROCESS_POOL_START_FRAME,
				PROCESS_POOL_SIZE,
				process_mem_pool_info_frame);
		/*process_mem_pool.mark_inaccessible(MEM_HOLE_START_FRAME, MEM_HOLE_SIZE);*/

	}

	void kernel_main_ctl(void) {
		disable_interrupt();
		mem_init();
		platform_init();
		target_early_init();
		core_init();
		init_shell();
		enable_interrupt();
		load_ramdisk();
		cpuidle();
	}
} // extern C
