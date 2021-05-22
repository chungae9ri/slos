#include <stdint.h>
#include <slos_error.h>
#include <odev.h>
#include <regops.h>
#include <mem_layout.h>
#include <gic.h>
#include <xil_printf.h>
#include <task.h>

int32_t init_odev(void)
{
	gic_register_int_handler(ODEV_IRQ_ID, odev_irq, NULL);
	/* This also reprogram the distributor 
	 * forwarding target cpu in the ICDIPTR register.
	 */
	gic_mask_interrupt(ODEV_IRQ_ID);

	return ERR_NO;
}

int32_t start_odev(void)
{
	uint32_t ctrl;

	ctrl = readl(ODEV_REG_CTRL);
	ctrl |= CTRL_GBL_START_MASK;
	writel(ctrl, ODEV_REG_CTRL);

	return ERR_NO;
}

int32_t start_odev_stream(void)
{
	uint32_t ctrl;

	ctrl = readl(ODEV_REG_CTRL);
	ctrl |= CTRL_OSTREAM_START_MASK;
	writel(ctrl, ODEV_REG_CTRL);

	return ERR_NO;
}

int32_t stop_odev(void)
{
	uint32_t ctrl;

	ctrl = readl(ODEV_REG_CTRL);
	ctrl &= ~CTRL_GBL_START_MASK;
	writel(ctrl, ODEV_REG_CTRL);
	return ERR_NO;
}

int32_t stop_odev_stream(void)
{
	uint32_t ctrl;

	ctrl = readl(ODEV_REG_CTRL);
	ctrl &= ~CTRL_OSTREAM_START_MASK;
	writel(ctrl, ODEV_REG_CTRL);

	return ERR_NO;
}

int32_t put_to_itab(uint32_t sAddr, uint32_t sLen)
{
	uint32_t ctrl, status;

	status = readl(ODEV_REG_STATUS);
	/* ITAB is full, return error */
	if (status & STAT_ITAB_FULL_MASK)
		return ERR_ITAB_FULL;

	writel(sAddr, ODEV_REG_ADDR);
	writel(sLen, ODEV_REG_LEN);

	ctrl = readl(ODEV_REG_CTRL);
	ctrl |= CTRL_IN_TRANS_MASK;
	writel(ctrl, ODEV_REG_CTRL);
	
	/* spin forever until ODEV core saves the data to Itab entry */
#if 0	
	while (1) {
		status = readl(ODEV_REG_STATUS);
		if((status & STAT_TRANSFER_DONE_MASK) == STAT_TRANSFER_DONE_MASK) {
			xil_printf("status after: 0x%x\n", status);
			break;
		}
		ctrl = readl(ODEV_REG_CTRL);
		/* if stop ODEV, then exit */
		if (!(ctrl & CTRL_GBL_START_MASK))
			return ERR_NO;
	}

#else
	while (!(readl(ODEV_REG_STATUS) & STAT_TRANSFER_DONE_MASK)) {
		ctrl = readl(ODEV_REG_CTRL);
		/* if stop ODEV, then exit */
		if (!(ctrl & CTRL_GBL_START_MASK))
			return ERR_NO;
	}
#endif

	/* clear the CTRL_IN_TRANS_MASK bit */
	ctrl = readl(ODEV_REG_CTRL);
	ctrl &= ~CTRL_IN_TRANS_MASK;
	writel(ctrl, ODEV_REG_CTRL);
	/*xil_printf("ctrl after: 0x%x\n", ctrl);*/

	return ERR_NO;
}

extern uint32_t smp_processor_id(void);

int odev_irq(void *arg)
{
	uint32_t cntl;

	/* stop consumer hw first */
	stop_consumer();

	uint32_t cpuid = smp_processor_id();

	cntl = readl(ODEV_REG_CTRL);
	cntl |= CTRL_INTR_DONE_MASK;
	writel(cntl, ODEV_REG_CTRL);
	xil_printf("odev irq done from cpu: %d!\n", cpuid);

	return ERR_NO;
}

int32_t set_consume_latency(uint32_t lat)
{
	writel(lat, ODEV_REG_LATENCY);

	return ERR_NO;
}

int32_t start_consumer(void)
{
	uint32_t cntl;

	xil_printf("odev consumer starts!\n");
	cntl = readl(ODEV_REG_CTRL);
	cntl |= CTRL_CONSUMER_START_MASK;
	writel(cntl, ODEV_REG_CTRL);

	return ERR_NO;
}

int32_t stop_consumer(void)
{
	uint32_t cntl, status;

	status = readl(ODEV_REG_STATUS);
	xil_printf("odev status: 0x%x!\n", status);

	cntl = readl(ODEV_REG_CTRL);
	cntl &= ~CTRL_CONSUMER_START_MASK;
	writel(cntl, ODEV_REG_CTRL);
	xil_printf("odev consumer stops!\n");

	return ERR_NO;

}

/* This task is run in the cpu1 triggered 
 * by sgi interrupt. sgi interrupt in the 
 * cpu1 is triggered by shell 'sgi' cmd.
 */

uint32_t run_odev_task(void)
{
	uint8_t *psrc;
	uint32_t i, j;

	psrc = (uint8_t *)O_STREAM_START;

	// To avoid underflow, prepare initial data first
	start_odev();
	for (i = 0; i < O_STREAM_WRAP * 4; i++) {
		/* seq number starting from 1*/
		((uint32_t *)((uint32_t)psrc + O_STREAM_BURST_SZ * i))[0] = i + 1;
	}
	// 
	xil_printf("start odev \n");
	set_consume_latency(10000);

	start_odev_stream();

	i = j = 0;
	/* out stream forever */
	for (;;) {
		/*((uint32_t *)((uint32_t)psrc + O_STREAM_STEP * i))[0] = k++;*/
		if (!put_to_itab(O_STREAM_START + O_STREAM_STEP * i, O_STREAM_STEP)) {
			xil_printf("put_to_itab: %d\n", i);
			/* spin for a while */
			j = 0;
			while (j < 100) 
				j++;
			i++;
#if 0
			/* make a sequence error */
			if (i == O_STREAM_WRAP) {
				*(uint32_t *)psrc = 0xFFFFFFFF;
			}
#endif

			i = i % O_STREAM_WRAP;
		} else {
			/* spin for a while */
			j = 0;
			while (j < 10000) 
				j++;
		}
		/*xil_printf("i: %d\n", i);*/
	}

	stop_consumer();
	stop_odev_stream();
	stop_odev();

	/* spin forever */
	while (1) ;
	return ERR_NO;

}


void create_odev_task(void *arg)
{
	create_cfs_task("odev_worker", run_odev_task, O_STREAM_TASK_PRI); 
}
