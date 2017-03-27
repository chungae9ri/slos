#include <xil_printf.h>


void cpuidle()
{
	/* do nothing for now */
	for (;;);
}

int main(void) 
{
	xil_printf("###hello world\n");
	cpuidle();

	return 0;
}
