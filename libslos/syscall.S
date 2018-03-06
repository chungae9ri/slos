/*
  libslos/syscall.S slos system call
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

.global exit
exit:
	svc #0
loooop:
    b       loooop

.global syscmd
syscmd:
	mov r12, lr
	svc #1
	mov pc, r12

.global write
write:
	mov r12, lr
	svc #2
	mov pc, r12

.global read
read:
	mov r12, lr
	svc #3
	mov pc, r12

.global sleep
sleep:
	mov r12, lr
	svc #4
	mov pc, r12
.end
