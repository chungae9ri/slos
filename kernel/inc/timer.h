#include <reg.h>
#include <iomap.h>

static void delay(uint64_t ticks);
void mdelay(unsigned msecs);
void udelay(unsigned usecs);
uint32_t get_ticks_per_sec();
uint32_t get_timer_freq(void);
static void qtimer_enable(void);
void qtimer_disable(void);
int timer_irq (void *arg);
uint64_t timer_get_phy_tick_cnt(void);
