/* SPDX-License-Identifier: MIT OR Apache-2.0
 *
 * Copyright (c) 2024 kwangdo.yi<kwangdo.yi@gmail.com>
 */

ENTRY(exceptions) 

/* Define Memories in the system */
MEMORY
{
	DDR_REGION : ORIGIN = 0x8000000, LENGTH = 0x40000000
}

SECTIONS
{
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
