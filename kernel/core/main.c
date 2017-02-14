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

#ifdef USE_MMU
#include <frame_pool.h>
#include <page_table.h>
#include <vm_pool.h>
#endif

#define USE_FS

#ifdef USE_FS
#include <file_system.h>
#include <file.h>
#endif

extern void enable_interrupt();
extern void disable_interrupt();
extern void update_csd(void);

extern struct task_struct task_arr[MAX_TASK];
extern uint64_t	jiffies;
extern struct task_struct *current;
extern uint32_t show_stat;

#ifdef USE_MMU
struct vmpool *pvm_kernel;
struct vmpool *pvm_user;
#endif


#ifdef USE_FS
struct file_system *pfs;
#endif

void cpuidle(void) 
{
	while (1) {
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

#ifdef USE_MMU
void core_init(unsigned int *ppd) 
{
	init_jiffies();
	init_idletask(ppd);
}
#else
void core_init() 
{
	init_jiffies();
	init_idletask();
}
#endif

void mem_init() 
{

}

#ifdef USE_FS
#define FILE_TEST_LEN	5000
#endif
int main(void) 
{
#ifdef USE_MMU
	struct framepool kfp, pfp;
	static struct pagetable pgt;
	struct vmpool kheap, pheap;
#endif
#ifdef USE_FS
	char buf[FILE_TEST_LEN];
	char temp[FILE_TEST_LEN];
	int len, i;
	struct file *fp;
#endif
	disable_interrupt();

#ifdef USE_MMU
	mem_init();
#endif

#ifdef USE_MMU
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
#endif

	platform_init();
	target_early_init();

#ifdef USE_MMU
	core_init(pgt.page_directory);
	init_shell(pgt.page_directory);
#else
	core_init();
	init_shell();
#endif
	enable_interrupt();
	/* imsi out while virtual mem implementation */
#ifdef USE_FS
	pfs = init_file_system();
	mount_file_system(pfs);
	format_file_system(pfs);
	fp = create_file(1, "test");
	for (i = 0; i < FILE_TEST_LEN; i++) 
		buf[i] = i % 256;
	write(fp, FILE_TEST_LEN, buf);
	reset(fp);
	read(fp, FILE_TEST_LEN, temp);
	print_msg("file test : ");
	for (i = 0; i < FILE_TEST_LEN; i++) {
		if (buf[i] != temp[i]) {
			print_msg("fail!!\n");
			break;
		}
	}
	if (i == FILE_TEST_LEN) print_msg("pass!!\n");
#endif

	update_csd();
	timer_enable();
#ifndef USE_MMU
	load_ramdisk();
#endif
	cpuidle();

	return 0;
}
