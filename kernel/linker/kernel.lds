OUTPUT_ARCH(arm)
ENTRY(exceptions) 
KERNEL_HEAP_SIZE = 0xC00000; /* 12M */
KERNEL_HEAP_START = 0x400000; /* 4M */
USER_HEAP_SIZE = 0x7000000; /* 112M */
USER_HEAP_START = 0x1000000; /* 16M */
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
	/*
	.heap : {
		__heap_start__ = KERNEL_HEAP_START;
		*(.heap)
		. = __heap_start__ + KERNEL_HEAP_SIZE + USER_HEAP_SIZE;
		__heap_end__ = .;
	}
	*/
}
