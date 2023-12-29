# (C) 2020 Kwangdo Yi <kwangdo.yi@gmail.com>
 
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.

#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.

#  You should have received a copy of the GNU General Public License
#  along with this program; if not, see <http://www.gnu.org/licenses/>

export PATH:=$(HOME)/bin/gcc-arm-none-eabi-10.3-2021.10/bin:$(PATH)

export LIBS := $(HOME)/bin/gcc-arm-none-eabi-10.3-2021.10/arm-none-eabi/lib
export LIBS2 :=$(HOME)/bin/gcc-arm-none-eabi-10.3-2021.10/lib/gcc/arm-none-eabi/10.3.1
export CC := arm-none-eabi-gcc
export ASM := arm-none-eabi-as
export LD := arm-none-eabi-ld
export AR := arm-none-eabi-ar
export OBJCOPY :=arm-none-eabi-objcopy

export TOPDIR := $(shell pwd)
export TOPOUT := $(TOPDIR)/out
export RAMDISKOUT := $(TOPDIR)/kernel/ramdisk

SUBDIRS := libslos mkfs apps kernel 

all:
	mkdir -p $(TOPOUT)
	mkdir -p $(RAMDISKOUT)
	for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir $$dir; \
	done

clean :
	@echo clean
	rm -rf out
	rm -rf $(RAMDISKOUT)
