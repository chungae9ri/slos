// SPDX-License-Identifier: MIT OR Apache-2.0
//
// Copyright (c) 2024 kwangdo.yi<kwangdo.yi@gmail.com>

#include <generated_kconfig_defs.h>

#if defined(ARCH_CORTEX_A9)
#include <ktimer.h>
#include <task.h>
#include <sched.h>
#include <runq.h>
#include <waitq.h>
#include <mm.h>
#include <loader.h>
#include <dma.h>
#include <odev.h>
#include <percpu.h>
#include <sgi.h>
#include <mailbox.h>
#include <slfs.h>
#include <uart.h>
#endif

#include <timer.h>
#include <gic_v1.h>
#include <printk.h>
#include <uart.h>

#if defined(ARCH_CORTEX_A9)
extern uint32_t show_stat;
extern void secondary_reset(void);
extern void flush_ent_dcache(void);
extern uint32_t read_scr(void);

#define A9_CPU_RST_CTRL		(0xF8000244)
#define A9_RST0_MASK		(0x1)
#define A9_RST1_MASK		(0x2)
#define A9_CLKSTOP0_MASK	(0x10)
#define A9_CLKSTOP1_MASK	(0x20)

static void cpuidle_secondary(void)
{
	uint32_t i = 0;
	printk("### I am cpuidle_secondary.....\n");

	while (1) {
		if (show_stat) {
			printk("### cpuidle_secondary is running....\n");
		}

		/* cpuidle can't go to waitq.
		 * arch-dependent power saving routine here.
		 */
		while( i <= 10000){
			i++;
		}
		i = 0;
	}
}

static void cpuidle(void)
{
	uint32_t i = 0;
	printk("### I am cpuidle.....\n");

	while (1) {
		if (show_stat) {
			printk("### cpuidle is running....\n");
		}
		/* cpuidle can't go to waitq.
		 * arch-dependent power saving routine here.
		 */
		while( i <= 10000){
			i++;
		}
		i = 0;
	}
}

/* Running from CPU 0 */
static void start_cpu1(void)
{
	uint32_t i, A9_rst_ctrl;

	A9_rst_ctrl = *(volatile uint32_t *)(A9_CPU_RST_CTRL);
	A9_rst_ctrl |= (A9_RST1_MASK | A9_CLKSTOP1_MASK); // set clock disable, reset bit for cpu1
	*(volatile uint32_t *)(A9_CPU_RST_CTRL) = A9_rst_ctrl;

	/*load [0x8] to r0*/
	*(volatile uint32_t *) (0x0) = *(uint32_t *)(secondary_reset);
	/* bx to [r0] */
	*(volatile uint32_t *) (0x4) = *(uint32_t *)(secondary_reset + 4);
	*(volatile uint32_t *) (0x8) = KERNEL_CODE_BASE;

	/* flush cache of cpu 0 */
	flush_ent_dcache();

	/* msleep isn't ready yet */
	while (i < 1000)
		i++;

	// release cpu1 reset
	A9_rst_ctrl &= ~(A9_RST1_MASK);
	*(volatile uint32_t *)(A9_CPU_RST_CTRL) = A9_rst_ctrl;
	// release cpu1 clock
	A9_rst_ctrl &= ~(A9_CLKSTOP1_MASK);
	*(volatile uint32_t *)(A9_CPU_RST_CTRL) = A9_rst_ctrl;
}

/* Running from CPU 1 */
int secondary_start_kernel(void)
{
	uint32_t cpuid;
	uint32_t scr = 0xFFFFFFFF;

	cpuid = smp_processor_id();
	printk("### I am cpu %d!\n", cpuid);
	scr = read_scr();
	printk("### cpu %d scr: 0x%x\n", cpuid, scr);

	init_gic_secondary();
	init_idletask();
	init_wq();
	init_rq();

	init_timertree();
	init_oneshot_timers();
	init_cfs_scheduler();
	init_timer();
	update_csd();
	timer_enable_secondary();
	/* enable sgi 15 for starting odev task */
	enable_sgi_irq(0xF, sgi_irq);

	/* odev device driver is running in the cpu1 */
	init_odev();
	create_workq_worker();
	cpuidle_secondary();

	return 0;
}

int start_kernel(void) 
{
	uint32_t scr = 0xFFFFFFFF;
	uint32_t cpuid;
	struct framepool framepool;
	struct pagetable pgt;
	struct vmpool kheap;

	init_uart();
	printk("stdio uart initialized");

	cpuid = smp_processor_id();
	scr = read_scr();
	printk("### cpu %d scr: 0x%x\n", cpuid, scr);

	init_kernmem(&framepool, &pgt, &kheap);
	printk("### init_kernmem done.\n");
	init_gic();
	init_idletask();

	init_wq();
	init_rq();
	init_shell();
	init_timertree();
	init_oneshot_timers();
	init_cfs_scheduler();
	init_timer();
	init_mailbox();
	update_csd();

	/* dma task is running in cpu0 */
	init_dma();

	create_workq_worker();

#if _ENABLE_SMP_
	printk("### start secondary cpu.\n");
	start_cpu1();
#endif
	timer_enable();
	cpuidle();

	return 0;
}
#elif defined (ARCH_CORTEX_A53)

int main(void)
{
	uint32_t current_el= 0;
	init_uart();
	printk("stdio uart initialized");

	/* read currentEL*/
	asm volatile ("mrs %[cel], CurrentEL" : [cel] "=r" (current_el)::);
	printk("currentEL: 0x%x\n", current_el >> 2); 
	
	init_gic();
	init_timer();

	while (1) ;

	return 0;
}
#endif

