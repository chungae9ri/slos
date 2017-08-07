#include <stdlib.h>
#include <stdint-gcc.h>
#include <timer.h>
#include <gic.h>
#include <xparameters_ps.h>
#include <regops.h>
#include <xil_printf.h>

/*static uint32_t tc;*/
uint32_t ticks_per_sec;
extern struct timer_struct *sched_timer;

uint32_t get_timer_freq()
{
	return (XPAR_CPU_CORTEXA9_0_CPU_CLK_FREQ_HZ / 2);
}

inline uint32_t timer_get_phy_tick_cnt(void)
{
	return (uint32_t)readl(PRIV_TMR_CNTR);
}

void timer_enable()
{
	int ctrl;

	ctrl = readl(PRIV_TMR_CTRL);
	ctrl = ctrl | (PRIV_TMR_EN_MASK 
			| PRIV_TMR_AUTO_RE_MASK
			| PRIV_TMR_IRQ_EN_MASK);
		
	writel(ctrl, PRIV_TMR_CTRL);
}

void timer_disable()
{
	int ctrl;

	ctrl = readl(PRIV_TMR_CTRL);
	ctrl = ctrl & ~PRIV_TMR_EN_MASK;
		
	writel(ctrl, PRIV_TMR_CTRL);
}

int timer_irq (void *arg)
{
	xil_printf("timer_irq");
	return 0;
}

uint32_t get_ticks_per_sec()
{
	return ticks_per_sec;
}

void timer_init()
{
	ticks_per_sec = get_timer_freq();
	writel(ticks_per_sec, PRIV_TMR_LD);

	gic_register_int_handler(PRIV_TMR_INT_VEC, timer_irq, NULL);
	gic_mask_interrupt(PRIV_TMR_INT_VEC);
	timer_enable();
}
