// SPDX-License-Identifier: MIT OR Apache-2.0
//
// Copyright (c) 2024 kwangdo.yi<kwangdo.yi@gmail.com>

#include <stdint.h>

#include <uart.h>
#include <printk.h>

int main(void)
{
	uint32_t current_el= 0;
	init_uart();

	/* read currentEL*/
	asm volatile ("mrs %[cel], CurrentEL" : [cel] "=r" (current_el)::);
	printk("currentEL: 0x%x\n", current_el >> 2); 

	while (1) ;

	return 0;
}


