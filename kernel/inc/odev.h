#ifndef _ODEV_H_
#define _ODEV_H_

#define O_STREAM_TASK_PRI	4

#define O_STREAM_START		0x38000000
#define O_STREAM_END		0x3C000000
#define O_STREAM_BURST_SZ	0x00000040 /* 64B */
#define O_STREAM_STEP		0x00000100 /* 256B */
#define O_STREAM_WRAP		0x00001000 /* 4096 */

#define ODEV_IRQ_ID			62U

#define CTRL_GBL_START_MASK		(0x1)
#define CTRL_INTR_DONE_MASK		(0x1 << 1)
#define CTRL_IN_TRANS_MASK		(0x1 << 2)
#define CTRL_OSTREAM_START_MASK		(0x1 << 3)
#define CTRL_CONSUMER_START_MASK	(0x1 << 4)
#define STAT_ITAB_UNDER_MASK		(0x1)
#define STAT_DATA_BUFF_UNDER_MASK	(0x1 << 1)
#define STAT_ITAB_FULL_MASK		(0x1 << 2)
#define STAT_TRANSFER_DONE_MASK		(0x1 << 3)

int32_t init_odev(void);
int32_t start_odev(void);
int32_t stop_odev(void);
int32_t start_odev_stream(void);
int32_t stop_odev_stream(void);
int32_t put_to_itab(uint32_t sAddr, uint32_t sLen);
int odev_irq(void *arg);
int32_t set_consume_latency(uint32_t lat);
int32_t start_consumer(void);
int32_t stop_consumer(void);
uint32_t run_odev_task(void);
void create_odev_task(void *);
#endif
