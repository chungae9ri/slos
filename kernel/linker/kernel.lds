OUTPUT_ARCH(arm)
ENTRY(exceptions) 
KERNEL_HEAP_SIZE = 0x800000; /* 8M */
KERNEL_HEAP_START = 0x800000; /* 8M */
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
	.kheap : {
		. = KERNEL_HEAP_START;
		__kernel_heap_start__ = KERNEL_HEAP_START;
		*(.kheap)
		. = __kernel_heap_start__ + KERNEL_HEAP_SIZE;
		__kernel_heap_end__ = .;

	}
	.uheap : {
		. = USER_HEAP_START;
		__user_heap_start__ = USER_HEAP_START;
		*(.uheap)
		. = __user_heap_start__ + USER_HEAP_SIZE;
		__user_heap_end__ = .;

	}
}
