#include <stdlib.h>
#include <clock.h>
#include <acpuclock.h>

/* Configure UART clock based on the UART block id*/
void clock_config_uart_dm(uint8_t id)
{
	int ret;

	ret = clk_get_set_enable("uart3_iface_clk", 0, 1);
	ret = clk_get_set_enable("uart3_core_clk", 7372800, 1);
} 
