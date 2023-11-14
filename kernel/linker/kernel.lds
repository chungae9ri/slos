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
VIRT_ADDR_BASE = 0xC0000000;
/* from 14MiB offset, heap start */
KERNEL_HEAP_START = 0xC0E00000;
KERNEL_HEAP_SIZE = 0x00100000;
RAMDISK_PHY_START = 0x00180000;
RAMDISK_VIRT_START = VIRT_ADDR_BASE + RAMDISK_PHY_START;

SECTIONS
{
	/* .ssbl section size 64KiB */
	. = 0x0100000;
	.ssbl : {
		*(SSBL);
		*(SSBL_NS);
		*(PGT_INIT);
	}

	. = 0xC0110000;
	.texts : AT(ADDR(.texts) - 0xC0000000) {
		*(EXCEPTIONS_S);
	}

	. = 0xC0111000;
	.textns : AT(ADDR(.textns) - 0xC0000000) {
		*(EXCEPTIONS_NS);
		*(.text);
	}

	.data : AT(ADDR(.data) - 0xC0000000) { 
		*(.data);
		*(.mailbox);
	}

	.bss : AT(ADDR(.bss) - 0xC0000000) { 
		*(.bss);
	}

	.rodata : AT(ADDR(.rodata) - 0xC0000000) {
		*(.rodata);
	}

	/* .initrd max size is 0x20000 (128KiB) */
	. = RAMDISK_VIRT_START;
	.initrd : AT(ADDR(.initrd) - 0xC0000000) {
		*(.initrd);
	}

	. = 0xC01A0000;
	.data.percpu : AT(ADDR(.data.percpu) - 0xC0000000) {
		*(.data.percpu);
	}

	. = 0xC0200000;
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
