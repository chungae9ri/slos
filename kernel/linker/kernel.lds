OUTPUT_ARCH(arm)
ENTRY(ssbl) 
KERNEL_HEAP_START = 0xC4000000;
KERNEL_HEAP_SIZE = 0x4000000;

SECTIONS
{
	. = 0x100000;
	.ssbl : {
		*(SSBL);
		*(PGT_INIT);
	}
	. = 0xC0101000;
	.boot : AT(ADDR(.boot) - 0xC0000000) {
		*(EXCEPTIONS);
		*(.text);
	}
	.data : AT(ADDR(.data) - 0xC0000000) { 
		*(.data)
	}
	.bss : AT(ADDR(.bss) - 0xC0000000) { 
		*(.bss) 
	}
	. = KERNEL_HEAP_START;
	.kheap : AT(ADDR(.kheap) - 0xC0000000) {
		__kernel_heap_start__ = .;
		*(.kheap)
		. = __kernel_heap_start__ + KERNEL_HEAP_SIZE;
		__kernel_heap_end__ = .;
	}
}
