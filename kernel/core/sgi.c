#include <gic.h>
#include <sgi.h>
#include <task.h>
#include <odev.h>
#include <xil_printf.h>
#include <mailbox.h>

uint32_t cfs_worker4(void);

void enable_sgi_irq(int vec, int (sgi_irq_handler)(void *arg))
{
	gic_register_int_handler(vec, sgi_irq_handler, NULL);
}

int sgi_irq(void *arg)
{
	struct sgi_data *pdat;
	enum letter_type letter;

	pdat = (struct sgi_data *)arg;
	xil_printf("sgi intr %d from cpu: %d\n", pdat->num, pdat->cpuid);


	letter = pull_mail();
	switch (letter) {
	case EMPTY:
		break;

	case TASK_STAT:
		break;

	case TASK_ODEV:
		enqueue_workq(create_odev_task, NULL);
		break;

	default:
		break;
	}

	return 0;
}


