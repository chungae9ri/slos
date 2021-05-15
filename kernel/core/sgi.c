#include <gic.h>
#include <sgi.h>
#include <task.h>
#include <odev.h>
#include <xil_printf.h>

uint32_t cfs_worker4(void);

void enable_sgi_irq(int vec, int (sgi_irq_handler)(void *arg))
{
	gic_register_int_handler(vec, sgi_irq_handler, NULL);
}

int sgi_irq(void *arg)
{
	struct sgi_data *pdat;
	pdat = (struct sgi_data *)arg;
	xil_printf("sgi intr %d from cpu: %d\n", pdat->num, pdat->cpuid);
	enqueue_workq(create_odev_task, NULL);

	return 0;
}


