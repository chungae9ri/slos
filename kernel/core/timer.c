#include <stdlib.h>
#include <stdint-gcc.h>
#include <timer.h>
#include <arch/qgic.h>
#include <irqs.h>
#include <task.h>
#include <ktimer.h>
#include <defs.h>

#define QTMR_TIMER_CTRL_ENABLE		(0x1 << 0)
#define QTMR_TIMER_CTRL_INT_MASK 	(0x1 << 1)
#define QTMR_PHY_CNT_MAX_VALUE		0xFFFFFFFFFFFFFFFF

static uint32_t tc;
extern void dsb(void);
extern struct timer_struct *sched_timer;
extern struct timer_root *ptroot;
uint32_t ticks_per_sec=0;
extern void kfree(unsigned int free_addr);
extern struct task_struct *current;
extern struct task_struct *prev;
extern struct cfs_rq *runq;

static void delay(uint64_t ticks)
{
	volatile uint64_t cnt;
	uint64_t init_cnt;
	uint64_t timeout = 0;

	cnt = timer_get_phy_tick_cnt();
	init_cnt = cnt;

	timeout = (cnt + ticks) & (uint64_t)(QTMR_PHY_CNT_MAX_VALUE);

	while (timeout < cnt && init_cnt <= cnt)
		cnt = timer_get_phy_tick_cnt();

	while (timeout > cnt)
		cnt = timer_get_phy_tick_cnt();
}

void mdelay(unsigned msecs)
{
	uint64_t ticks;
	ticks = ((uint64_t)msecs * ticks_per_sec)/1000;
	delay(ticks);
}

void udelay(unsigned usecs)
{
	uint64_t ticks;
	ticks = ((uint64_t)usecs * ticks_per_sec)/1000000;
	delay(ticks);
}

uint32_t get_timer_freq()
{
	uint32_t freq;
	freq = readl(QTMR_V1_CNTFRQ);
	return freq;
}

inline uint64_t timer_get_phy_tick_cnt(void)
{
	uint32_t phy_cnt_lo;
	uint32_t phy_cnt_hi_1;
	uint32_t phy_cnt_hi_2;

	do {
		phy_cnt_hi_1 = readl(QTMR_V1_CNTPCT_HI);
		phy_cnt_lo = readl(QTMR_V1_CNTPCT_LO);
		phy_cnt_hi_2 = readl(QTMR_V1_CNTPCT_HI);
	} while (phy_cnt_hi_1 != phy_cnt_hi_2);

	return ((uint64_t)phy_cnt_hi_1<<32) | phy_cnt_lo;
}

void timer_enable()
{
	uint32_t ctrl;

	ctrl = readl(QTMR_V1_CNTP_CTL);

	/* Program CTRL Register */
	ctrl |= QTMR_TIMER_CTRL_ENABLE;
	ctrl &= ~QTMR_TIMER_CTRL_INT_MASK;
	writel(ctrl, QTMR_V1_CNTP_CTL);
	dsb();
}

void timer_disable()
{
	uint32_t ctrl;

	ctrl = readl(QTMR_V1_CNTP_CTL);

	/* program cntrl register */
	ctrl &= ~QTMR_TIMER_CTRL_ENABLE;
	ctrl |= QTMR_TIMER_CTRL_INT_MASK;

	writel(ctrl, QTMR_V1_CNTP_CTL);
	dsb();
}

int timer_irq (void *arg)
{
	struct rb_node *next_node;
	struct timer_struct *pnt, *pct; 
	uint32_t elapsed;

	elapsed = get_elapsedtime();
	pct = container_of(ptroot->rb_leftmost, struct timer_struct, run_node);
	update_timer_tree(elapsed);
	pnt = container_of(ptroot->rb_leftmost, struct timer_struct, run_node);
	tc = pnt->tc;
	/* reprogram periodic timer intr */
	writel(tc, QTMR_V1_CNTP_TVAL);
	dsb();

	if (current->type == CFS_TASK) {
		current->se.ticks_consumed += elapsed;
		current->se.vruntime = (current->se.ticks_consumed) * 
					(current->se.priority) / runq->priority_sum;
	}

	switch(pct->type) {
		case SCHED_TIMER:
			sched_timer->handler(elapsed);
			break;

		case REALTIME_TIMER:
			if (current != pct->pt) {
				switch_context(current, pct->pt);
				prev = current;
				current = pct->pt;
			}
			break;

		case ONESHOT_TIMER:
#if 0
			update_sched_timer();
			next_node = rb_next(&pct->run_node);
			next_timer = container_of(next_node, struct timer_struct, run_node);
			tc = next_timer->tc;
			/* call timer handler */
			/* need to schedule oneshot timer handler instead of direct 
			   call in interrupt context. this may result in broken rb tree.
			   fix me */
			pct->handler(arg);
			del_timer(ptroot, pct);
			kfree((unsigned int)pct);
			/**/
			writel(tc, QTMR_V1_CNTP_TVAL);
			dsb();
#endif
			break;

		default:
			break;
	}

	return 0;
}

uint32_t get_ticks_per_sec()
{
	return ticks_per_sec;
}

void platform_init_timer()
{
	ticks_per_sec = get_timer_freq();

	timertree_init();
	sched_timer_init();

	tc = sched_timer->tc;
	writel(tc, QTMR_V1_CNTP_TVAL);
	dsb();
	gic_register_int_handler(INT_QTMR_FRM_0_PHYSICAL_TIMER_EXP, timer_irq, 0);
	qgic_unmask_interrupt(INT_QTMR_FRM_0_PHYSICAL_TIMER_EXP);
}
