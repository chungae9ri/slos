#include <gic.h>
#include <sgi.h>
#include <task.h>
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
	create_cfs_task("cfs_worker4", cfs_worker4, 4);

	return 0;
}


