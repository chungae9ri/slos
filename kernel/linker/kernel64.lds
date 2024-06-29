/* SPDX-License-Identifier: MIT OR Apache-2.0
 *
 * Copyright (c) 2024 kwangdo.yi<kwangdo.yi@gmail.com>
 */

ENTRY(_start) 

/* Define Memories in the system */
MEMORY
{
	ROM_REGION (rx): ORIGIN = 0x8000000, LENGTH = 512K /* 512 KiB*/
	RAM_REGION (rw): ORIGIN = 0x8080000, LENGTH = 512K /* 512 KiB */
}

/* Stack region */
__stack_size = 0x1000;
__stack_start = ORIGIN(RAM_REGION) + LENGTH(RAM_REGION);
__stack_limit =  __stack_start - __stack_size;

SECTIONS
{
	. = 0x8000000;

	/* Text section: code and read-only data */
	.text : {
		KEEP(*(EXCEPTIONS)); /* Exception handlers */
		*(.text*); 	
		*(.rodata*);
		_etext = .;	/* End of .text section */
	} > ROM_REGION

	/* Data section: initialized data */
	/* FIXME: Not working fsbl */
	/*.data : AT(_etext) { */
	.data : { 
		. = ALIGN(8);
		_sdata = .; /* Start of .data section */
		*(.data*);
		_edata = .; /* End of .data section */
	} > RAM_REGION

	/* BSS section: zero-initialized data */
	/* FIXME: Not working with fsbl */
	/*.bss : AT(_edata) { */
	.bss : { 
		. = ALIGN(8);
		_sbss = .; /* Start of .bss section */
		*(.bss*); 
		_ebss = .; /* End of .bss section */
	} > RAM_REGION

	/* FIXME: This generates assert fail. */
	/*ASSERT(__stack_limit < _ebss, "no enough memory space for stack allocation")*/
}
