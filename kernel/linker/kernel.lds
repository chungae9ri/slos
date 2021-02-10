/*
  kernel/linker/kernel.lds
  (C) 2018 Kwangdo Yi <kwangdo.yi@gmail.com>
 
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, see <http://www.gnu.org/licenses/>
*/

OUTPUT_ARCH(arm)
ENTRY(ssbl) 
KERNEL_HEAP_START = 0xC4000000;
KERNEL_HEAP_SIZE = 0x4000000;

SECTIONS
{
	. = 0x00100000;
	.ssbl : {
		*(SSBL);
		*(PGT_INIT);
	}

	. = 0xC0101000;
	.text : AT(ADDR(.text) - 0xC0000000) {
		*(EXCEPTIONS);
		*(.text);
	}

	.data : AT(ADDR(.data) - 0xC0000000) { 
		*(.data)
	}
	.bss : AT(ADDR(.bss) - 0xC0000000) { 
		*(.bss) 
	}

	. = 0xC0201000;
	.secexceptions : AT(ADDR(.secexceptions) - 0xC0000000) {
		*(SECONDARY_EXCEPTIONS);
	}

	. = KERNEL_HEAP_START;
	.kheap : AT(ADDR(.kheap) - 0xC0000000) {
		__kernel_heap_start__ = .;
		*(.kheap)
		. = __kernel_heap_start__ + KERNEL_HEAP_SIZE;
		__kernel_heap_end__ = .;
	}
}
