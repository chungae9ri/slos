/*
  kernel/inc/timer.h 
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
uint32_t get_ticks_per_sec(void);
void set_ticks_per_sec(uint32_t tps);
uint32_t get_timer_freq(void);
int timer_irq (void *arg);
uint32_t timer_get_phy_tick_cnt(void);
void init_timer(void);
void timer_enable(void);
