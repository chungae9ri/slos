OUTPUT_ARCH(arm)
ENTRY(exceptions) 
KERNEL_HEAP_SIZE = 0x800000; /* 8M */
KERNEL_HEAP_START = 0x800000; /* 8M */
USER_HEAP_SIZE = 0x7000000; /* 112M */
USER_HEAP_START = 0x1000000; /* 16M */
/* Define Memories in the system */

MEMORY
{
	ps7_ddr_0_S_AXI_BASEADDR : ORIGIN = 0x100000, LENGTH = 0x3FF00000
	ps7_qspi_linear_0_S_AXI_BASEADDR : ORIGIN = 0xFC000000, LENGTH = 0x1000000
	ps7_ram_0_S_AXI_BASEADDR : ORIGIN = 0x0, LENGTH = 0x30000
	ps7_ram_1_S_AXI_BASEADDR : ORIGIN = 0xFFFF0000, LENGTH = 0xFE00
}

SECTIONS
{
	. = 0x100000;
	.text : {
		*(EXCEPTIONS);
		*(.text) 
	} > ps7_ddr_0_S_AXI_BASEADDR
	.data : { 
		*(.data);
	} > ps7_ddr_0_S_AXI_BASEADDR
	.bss : { 
		*(.bss); 
	} > ps7_ddr_0_S_AXI_BASEADDR
	.kheap : {
		. = KERNEL_HEAP_START;
		__kernel_heap_start__ = KERNEL_HEAP_START;
		*(.kheap)
		. = __kernel_heap_start__ + KERNEL_HEAP_SIZE;
		__kernel_heap_end__ = .;

	} > ps7_ddr_0_S_AXI_BASEADDR
	.uheap : {
		. = USER_HEAP_START;
		__user_heap_start__ = USER_HEAP_START;
		*(.uheap)
		. = __user_heap_start__ + USER_HEAP_SIZE;
		__user_heap_end__ = .;

	} > ps7_ddr_0_S_AXI_BASEADDR
}
