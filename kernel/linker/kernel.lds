OUTPUT_ARCH(arm)
ENTRY(exceptions) 
HEAP_SIZE = 0x600000; /* 6M */
HEAP_START = 0x1000000; /* 16M */
SECTIONS
{
	. = 0x8000;
	.text : {
		*(EXCEPTIONS);
		*(.text) 
	}
	.data : { 
		*(.data);
	}
	.bss : { 
		*(.bss); 
	}
	.heap : {
		__heap_start__ = HEAP_START;
		*(.heap)
		. = __heap_start__ + HEAP_SIZE;
		__heap_end__ = .;
	}
}
