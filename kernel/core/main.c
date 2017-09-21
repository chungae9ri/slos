#include <xil_printf.h>
#include <gic.h>
#include <timer.h>

void cpuidle(void)
{
	/* do nothing for now */
	for (;;);
}

int main(void) 
{
	xil_printf("###hello world\n");
	gic_init();
	timer_init();
	cpuidle();

	return 0;
}
