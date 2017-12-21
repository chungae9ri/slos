OUTPUT_ARCH(arm)
ENTRY(ssbl) 
KERNEL_HEAP_START = 0x600000;
KERNEL_HEAP_SIZE = 0x200000;

SECTIONS
{
	. = 0x100000;
	.ssbl : {
		*(SSBL);
		*(PGT_INIT);
	}
	. = 0x101000;
	.text : {
		*(EXCEPTIONS);
		*(.text);
	}
	.data : { 
		*(.data)
	}
	.bss : { 
		*(.bss) 
	}
	.kheap : {
		. = KERNEL_HEAP_START;
		__kernel_heap_start__ = KERNEL_HEAP_START;
		*(.kheap)
		. = __kernel_heap_start__ + KERNEL_HEAP_SIZE;
		__kernel_heap_end__ = .;
	}
}
