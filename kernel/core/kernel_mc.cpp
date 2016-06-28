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
	#include <page_table.h>
	#include <vm_pool.h>

	extern void enable_interrupt();
	extern void disable_interrupt();

	extern struct task_struct task_arr[MAX_TASK];
	extern uint64_t	jiffies;
	extern struct task_struct *current;
	extern uint32_t show_stat;

	PageTable *pkernel_pt;

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

	void core_init(unsigned int *ppd) {
		init_jiffies();
		init_idletask(ppd);
	}

	void mem_init() {
		
	}

	void kernel_main_ctl(void) {
		unsigned int *pdadr;

		disable_interrupt();
		mem_init();

		/* initialize frame pools */
		FramePool kernel_mem_pool(KERNEL_HEAP_START_FRAME, KERNEL_HEAP_FRAME_NUM, 0);
		unsigned long process_mem_pool_info_frame = kernel_mem_pool.get_frame();
		FramePool process_mem_pool(PROCESS_HEAP_START_FRAME,
				PROCESS_HEAP_FRAME_NUM,
				process_mem_pool_info_frame);
		/*process_mem_pool.mark_inaccessible(MEM_HOLE_START_FRAME, MEM_HOLE_SIZE);*/

		PageTable::init_paging(&kernel_mem_pool,
				&process_mem_pool,
				0 MB);

		PageTable kernel_pt(PG_TABLE_KERN);
		pdadr = kernel_pt.getpd();
		kernel_pt.load();
		PageTable::enable_paging();

		pkernel_pt = &kernel_pt;

		VMPool kernel_heap(512 MB, 12 MB, &kernel_mem_pool, &kernel_pt);
		/*VMPool proc_heap(1 GB, 256 MB, &process_mem_pool, &);*/

		platform_init();
		target_early_init();
		core_init(pdadr);
		init_shell(pdadr);
		enable_interrupt();
		/* imsi out while virtual mem implementation */
		/*load_ramdisk();*/
		cpuidle();
	}
} // extern C
