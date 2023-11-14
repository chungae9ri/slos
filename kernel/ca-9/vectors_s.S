/*
  kernel/ca-9/vectors_s.S kernel exception handler
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

.include "mem_layout.h"

/* arm exception code */
.section EXCEPTIONS_S, "ax"
.arm
.global exceptions_s

exceptions_s:
	b reset_handler_s		
	b .
	b .
	b .
	b .
	b .
	b .
	b .

reset_handler_s:
	/*set VBAR (banked) with 0x110000 for secure world */
	ldr	r0, =KERNEL_EXCEPTION_BASE_S
	mcr	p15,0,r0,c12,c0,0 

	b init_monitor
;
.end
