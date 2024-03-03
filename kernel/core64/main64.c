// SPDX-License-Identifier: MIT OR Apache-2.0
//
// Copyright (c) 2024 kwangdo.yi<kwangdo.yi@gmail.com>

#include <stdint.h>

#include <uart.h>
#include <printk.h>

#define ZYNQMP_UART0_BASEADDR		0x00FF000000
#define ZYNQMP_UART1_BASEADDR		0x00FF010000

int main(void)
{
	uint32_t current_el= 0;
	init_uart(ZYNQMP_UART1_BASEADDR);

	/* read currentEL*/
	asm volatile ("mrs %[cel], CurrentEL" : [cel] "=r" (current_el)::);
	printk("currentEL: %d\n", current_el); 

	return 0;
}

#if 0
void entry_el1(void){
    Print (L"running in el1.\n\r");
}

EFI_STATUS
EFIAPI
UefiMain (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{   


    __asm__ volatile (
    "mov x0, #(1 << 31)\n\t"
    "msr hcr_el2, x0\n\t"

    "mov x0, #0x0800\n\t"
    "movk x0, #0x30d0, lsl #16\n\t"
    "msr sctlr_el1, x0\n\t"

    "mov x0, #0x33ff\n\t"
    "msr cptr_el2, x0\n\t"

    "msr hstr_el2, xzr\n\t"

    "mov x0, #0x3c5\n\t" 
    "msr spsr_el2, x0\n\t"

    "mov x0, %0\n\t"  
    "msr elr_el2, x0\n\t"

    "eret" : : "r" (entry_el1) :
    );
	return 0;
}
#endif
