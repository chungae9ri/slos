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

struct vmpool *pvm_kernel;
struct vmpool *pvm_user;

void cpuidle(void) 
{
	while(1) {
		/* imsi for test */
		/*drop_usrtask();*/
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

void core_init(unsigned int *ppd) 
{
	init_jiffies();
	init_idletask(ppd);
}

void mem_init() 
{

}

int main(void) 
{
	struct framepool kfp, pfp;
	static struct pagetable pgt;
	struct vmpool kheap, pheap;
	int i=1;

	disable_interrupt();
	mem_init();

	while(i==1);

	/* initialize frame pools */
	init_framepool(&kfp, KERNEL_START_FRAME, 
			KERNEL_FRAME_NUM, 0);
	unsigned long process_mem_pool_info_frame = get_frame(&kfp);

	init_framepool(&pfp,PROCESS_HEAP_START_FRAME,
			PROCESS_HEAP_FRAME_NUM,
			process_mem_pool_info_frame);
	/* donot change the order */
	mark_inaccessible(&kfp, KERNEL_INACC_FRAME, KERNEL_INACC_FRAME_NUM);

	init_pageregion(&pgt, &kfp, &pfp, 0 MB);
	init_pagetable(&pgt, PG_TABLE_KERN);
	load_pagetable(&pgt);
	enable_paging();

	/*pkernel_pt = &kernel_pt;*/

	init_vmpool(&kheap, &pgt, 8 MB, 8 MB);
	/*init_vmpool(&pheap, &pgt, 1 GB, 112 MB);*/

	pvm_kernel = &kheap;
	/*pvm_user = &pheap;*/

	platform_init();
	target_early_init();
	core_init(pgt.page_directory);
	init_shell(pgt.page_directory);
	enable_interrupt();
	/* imsi out while virtual mem implementation */
	/*load_ramdisk();*/
	timer_enable();
	cpuidle();

	return 0;
}
