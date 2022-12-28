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

#include <printk.h>
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
#include <percpu.h>
#include <sgi.h>
#include <mailbox.h>

extern uint32_t show_stat;
extern void secondary_reset(void);
extern void flush_ent_dcache(void);
extern uint32_t read_scr(void);

#define A9_CPU_RST_CTRL		(0xF8000244)
#define A9_RST0_MASK		(0x1)
#define A9_RST1_MASK		(0x2)
#define A9_CLKSTOP0_MASK	(0x10)
#define A9_CLKSTOP1_MASK	(0x20)

typedef struct uart_ps {
	volatile uint32_t CR;
	volatile uint32_t MR;
	volatile uint32_t IER;
	volatile uint32_t IDR;
	volatile const uint32_t IMR;
	volatile uint32_t ISR;
	volatile uint32_t BAUDGEN;
	volatile uint32_t RXTOUT;
	volatile uint32_t RXWM;
	volatile uint32_t MODEMCR;
	volatile uint32_t MODEMSR;
	volatile const uint32_t SR;
	volatile uint32_t FIFO;
	volatile uint32_t BAUD_DIV;
	volatile uint32_t FLOW_DELAY;
	volatile uint32_t TX_FIFO_TRIG_LV;
} UART_PS_t;

static void init_uart(void)
{
	UART_PS_t *puart = (UART_PS_t *)(STDIN_BASEADDRESS);
	puart->CR = 0x00000114;
	puart->MR = 0x00000020;
	puart->IER = 0x00000000;
	puart->IDR = 0x00000000;
	puart->BAUDGEN = 0x0000007C;
	puart->RXTOUT = 0x0000000A;
	puart->RXWM = 0x00000038;
	puart->MODEMCR = 0x00000003;
	puart->BAUD_DIV = 0x00000006;
	puart->FLOW_DELAY = 0x00000000;
	puart->TX_FIFO_TRIG_LV = 0x00000020;
}

static void init_platform(void)
{
	init_uart();
	printk("vPlatformInit\n");
}

static void cpuidle_secondary(void)
{
	uint32_t i = 0;
	printk("I am cpuidle_secondary.....\n");

	while (1) {
		if (show_stat) {
			printk("cpuidle_secondary is running....\n");
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
	printk("I am cpuidle.....\n");

	while (1) {
		if (show_stat) {
			printk("cpuidle is running....\n");
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
	printk("I am cpu 0x%x!\n", cpuid);
	scr = read_scr();
	printk("cpu 0x%x scr: 0x%x\n", cpuid, scr);

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

	init_platform();

	cpuid = smp_processor_id();
	scr = read_scr();
	printk("cpu 0x%x scr: 0x%x\n", cpuid, scr);

	init_kernmem(&framepool, &pgt, &kheap);
	printk("### init_kernmem done.\n");
	init_gic();
	init_idletask();
#if 0
	init_file_system();
	mount_file_system();
	format_file_system();
	printk("### mount slfs file system.\n");
#endif

	init_wq();
	init_rq();
	init_shell();
	init_timertree();
	init_oneshot_timers();
	init_cfs_scheduler();
	init_timer();
	init_mailbox();
	update_csd();
	timer_enable();

	/*
	create_ramdisk_fs();
	printk("### load user app to slfs.\n");
	init_dma();
	 */

	create_workq_worker();

#if _ENABLE_SMP_
	printk("### start secondary cpu.\n");
	start_cpu1();
#endif
	cpuidle();

	return 0;
}
