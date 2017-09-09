#include <xparameters.h>
#include <xparameters_ps.h>

#define PRIV_TMR_LD		XPAR_PS7_SCUTIMER_0_BASEADDR + 0x0000
#define PRIV_TMR_CNTR		XPAR_PS7_SCUTIMER_0_BASEADDR + 0x0004
#define PRIV_TMR_CTRL		XPAR_PS7_SCUTIMER_0_BASEADDR + 0x0008
#define PRIV_TMR_INTSTAT	XPAR_PS7_SCUTIMER_0_BASEADDR + 0x000C
#define PRIV_TMR_EN_MASK	0x00000001
#define PRIV_TMR_AUTO_RE_MASK	0x00000002
#define PRIV_TMR_IRQ_EN_MASK	0x00000004
#define PRIV_TMR_PRESCL_MASK	0x0000FF00
#define PRIV_TMR_INT_VEC	XPS_SCU_TMR_INT_ID		


#define SPI_TTC0_0_CLK_CTRL	XPAR_XTTCPS_0_BASEADDR + 0x0000
#define SPI_TTC0_1_CLK_CTRL	XPAR_XTTCPS_0_BASEADDR + 0x0004
#define SPI_TTC0_2_CLK_CTRL	XPAR_XTTCPS_0_BASEADDR + 0x0008

#define SPI_TTC0_0_CNT_CTRL	XPAR_XTTCPS_0_BASEADDR + 0x000C
#define SPI_TTC0_1_CNT_CTRL	XPAR_XTTCPS_0_BASEADDR + 0x0010
#define SPI_TTC0_2_CNT_CTRL	XPAR_XTTCPS_0_BASEADDR + 0x0014

void mdelay(unsigned msecs);
void udelay(unsigned usecs);
uint32_t get_ticks_per_sec();
uint32_t get_timer_freq(void);
int timer_irq (void *arg);
uint32_t timer_get_phy_tick_cnt(void);
void timer_init();
void timer_enable();
