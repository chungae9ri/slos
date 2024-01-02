ENTRY(exceptions) 

/* Define Memories in the system */
MEMORY
{
	DDR_REGION : ORIGIN = 0x0, LENGTH = 0x40000000
}

SECTIONS
{
	. = 0x8000000;
	.text : {
		*(EXCEPTIONS);
		*(.text) 
	} > DDR_REGION
	.data : { 
		*(.data);
	} > DDR_REGION
	.bss : { 
		*(.bss); 
	} > DDR_REGION
}
