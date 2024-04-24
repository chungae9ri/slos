/* SPDX-License-Identifier: MIT OR Apache-2.0
 *
 * Copyright (c) 2024 kwangdo.yi<kwangdo.yi@gmail.com>
 */

ENTRY(exceptions) 

/* Define Memories in the system */
MEMORY
{
	ROM_REGION (rx): ORIGIN = 0x8000000, LENGTH = 0x1000000	/* 16 MiB*/
	RAM_REGION (rw): ORIGIN = 0x9000000, LENGTH = 0x20000000 /* 512 MiB */
}

SECTIONS
{
	. = 0x8000000;
	.text : {
		KEEP(*(EXCEPTIONS));
		*(.text);
	} > ROM_REGION
	.data : { 
		*(.data);
	} > RAM_REGION
	.bss : { 
		*(.bss); 
	} > RAM_REGION
}
