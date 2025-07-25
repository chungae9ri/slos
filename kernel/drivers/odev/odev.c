// SPDX-License-Identifier: MIT OR Apache-2.0
//
// Copyright (c) 2024 kwangdo.yi<kwangdo.yi@gmail.com>

/**
 * @addtogroup kernel
 * @{
 * @addtogroup kernel_drivers Drivers
 * @{
 * @addtogroup kernel_drivers_odev Outstream device
 * @{
 *
 * @file
 *
 * @brief Outstream Device driver
 *
 */

/** Outstream Device compatible string in devicetree */
#define DEVICE_DT_COMPAT SLOS_ODEV

#include <stdint.h>

#include <error.h>
#include <odev.h>
#include <regops.h>
#include <mem_layout.h>
#include <gic_v1.h>
#include <printk.h>
#include <task.h>
#include <timer.h>
#include <percpu.h>
#include <generated_devicetree_defs.h>

/* CAUTION!
 * O_STREAM_START address is used as the start address of
 * sequence number check in the PL's slave AXI module.
 * Currently it is hardcoded in both (PS and PL) and should
 * be the same. When modify this, it also should be changed
 * in the odev slave module's sig_seq_addr in multiple place.
 */
#define O_STREAM_START	  0x18000000
#define O_STREAM_BURST_SZ 0x00000040 /* 64B */
#define O_STREAM_STEP	  0x00000100 /* 256B */
#define O_STREAM_WRAP	  0x00001000 /* 4096 */

/* register offset */
#define REG_CTRL_OFFSET	   0x0
#define REG_STATUS_OFFSET  0x4
#define REG_ADDR_OFFSET	   0x8
#define REG_LEN_OFFSET	   0xc
#define REG_LATENCY_OFFSET 0x10
/* bit masks */
#define BM_GBL_START	   (0x1)
#define BM_INTR_DONE	   (0x1 << 1)
#define BM_IN_TRANS	   (0x1 << 2)
#define BM_OSTREAM_START   (0x1 << 3)
#define BM_CONSUMER_START  (0x1 << 4)
#define BM_ITAB_UNDER	   (0x1)
#define BM_DATA_BUFF_UNDER (0x1 << 1)
#define BM_ITAB_FULL	   (0x1 << 2)
#define BM_TRANSFER_DONE   (0x1 << 3)

#define O_STREAM_TASK_PRI 4

DEVICE_DEFINE_IDX(odev, 0);

static int32_t start_odev(struct device *dev)
{
	uint32_t ctrl;

	ctrl = read32(dev->base_addr + REG_CTRL_OFFSET);
	ctrl |= BM_GBL_START;
	write32(dev->base_addr + REG_CTRL_OFFSET, ctrl);

	return 0;
}

static int32_t start_odev_stream(struct device *dev)
{
	uint32_t ctrl;

	ctrl = read32(dev->base_addr + REG_CTRL_OFFSET);
	ctrl |= BM_OSTREAM_START;
	write32(dev->base_addr + REG_CTRL_OFFSET, ctrl);

	return 0;
}

static int32_t stop_odev(struct device *dev)
{
	uint32_t ctrl;

	ctrl = read32(dev->base_addr + REG_CTRL_OFFSET);
	ctrl &= ~BM_GBL_START;
	write32(dev->base_addr + REG_CTRL_OFFSET, ctrl);

	return 0;
}

static int32_t stop_odev_stream(struct device *dev)
{
	uint32_t ctrl;

	ctrl = read32(dev->base_addr + REG_CTRL_OFFSET);
	ctrl &= ~BM_OSTREAM_START;
	write32(dev->base_addr + REG_CTRL_OFFSET, ctrl);

	return 0;
}

static int32_t put_to_itab(struct device *dev, uint32_t sAddr, uint32_t sLen)
{
	uint32_t ctrl, status;

	status = read32(dev->base_addr + REG_STATUS_OFFSET);
	/* ITAB is full, return error */
	if (status & BM_ITAB_FULL)
		return -EINVAL;

	write32(dev->base_addr + REG_ADDR_OFFSET, sAddr);
	write32(dev->base_addr + REG_LEN_OFFSET, sLen);

	ctrl = read32(dev->base_addr + REG_CTRL_OFFSET);
	ctrl |= BM_IN_TRANS;
	write32(dev->base_addr + REG_CTRL_OFFSET, ctrl);

	while (!(read32(dev->base_addr + REG_STATUS_OFFSET) & BM_TRANSFER_DONE)) {
		ctrl = read32(dev->base_addr + REG_CTRL_OFFSET);
		/* if stop ODEV, then exit */
		if (!(ctrl & BM_GBL_START))
			return 0;
	}

	/* clear the CTRL_IN_TRANS_MASK bit */
	ctrl = read32(dev->base_addr + REG_CTRL_OFFSET);
	ctrl &= ~BM_IN_TRANS;
	write32(dev->base_addr + REG_CTRL_OFFSET, ctrl);

	return 0;
}

int32_t init_odev(struct device *dev)
{
	dev->name = DT_GET_COMPAT(0);
	dev->base_addr = DT_GET_BASE_ADDR(0);
	dev->irq = DT_GET_IRQ(0);

	gic_register_int_handler(dev->irq, odev_irq, dev);
	/* This also reprogram the distributor
	 * forwarding target cpu in the ICDIPTR register.
	 */
	gic_enable_interrupt(dev->irq);

	return 0;
}

int32_t odev_irq(void *arg)
{
	uint32_t cntl;
	struct device *dev = (struct device *)arg;

	/* stop consumer hw first */
	stop_consumer(dev);

	uint32_t cpuid = smp_processor_id();

	cntl = read32(dev->base_addr + REG_CTRL_OFFSET);
	cntl |= BM_INTR_DONE;
	write32(dev->base_addr + REG_CTRL_OFFSET, cntl);
	printk("odev irq done from cpu: %d!\n", cpuid);

	return 0;
}

int32_t start_consumer(struct device *dev)
{
	uint32_t cntl;

	printk("odev consumer starts!\n");
	cntl = read32(dev->base_addr + REG_CTRL_OFFSET);
	cntl |= BM_CONSUMER_START;
	write32(dev->base_addr + REG_CTRL_OFFSET, cntl);

	return 0;
}

int32_t stop_consumer(struct device *dev)
{
	uint32_t cntl, status;

	status = read32(dev->base_addr + REG_STATUS_OFFSET);
	printk("odev status: 0x%x!\n", status);

	cntl = read32(dev->base_addr + REG_CTRL_OFFSET);
	cntl &= ~BM_CONSUMER_START;
	write32(dev->base_addr + REG_CTRL_OFFSET, cntl);
	printk("odev consumer stops!\n");

	return 0;
}

/* This task is run in the cpu1 triggered
 * by sgi interrupt. sgi interrupt in the
 * cpu1 is triggered by shell 'sgi' cmd.
 */

uint32_t run_odev_task(void)
{
	uint8_t *psrc;
	uint32_t i;
	int32_t ret;
	struct device *dev;
	struct task_struct *this_current = NULL;

#if _ENABLE_SMP_
	this_current = __get_cpu_var(current);
#else
	this_current = current;
#endif

	dev = (struct device *)(this_current->arg);
	psrc = (uint8_t *)O_STREAM_START;

	// To avoid underflow, prepare initial data first
	start_odev(dev);
	for (i = 0; i < O_STREAM_WRAP * 4; i++) {
		/* seq number starting from 1*/
		((uint32_t *)((uint32_t)psrc + O_STREAM_BURST_SZ * i))[0] = i + 1;
	}

	write32(dev->base_addr + REG_LATENCY_OFFSET, 10000);
	start_odev_stream(dev);

	i = 0;

	/* out stream forever */
	for (;;) {
		ret = put_to_itab(dev, O_STREAM_START + O_STREAM_STEP * i, O_STREAM_STEP);
		if (!ret) {
			printk("put_to_itab: %d\n", i);
			mdelay(10);
			i++;

			i = i % O_STREAM_WRAP;
		} else {
			mdelay(100);
		}
	}

	stop_consumer(dev);
	stop_odev_stream(dev);
	stop_odev(dev);

	/* spin forever */
	while (1)
		;

	return 0;
}

int32_t create_odev_task(void *arg)
{
	(void)arg;

	create_cfs_task("odev_worker", run_odev_task, O_STREAM_TASK_PRI, arg);

	return 0;
}

/**
 * @}
 * @}
 * @}
 *
 */
