OUTPUT_ARCH(arm)
ENTRY(exceptions) 

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
}
