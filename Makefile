# SPDX-License-Identifier: MIT OR Apache-2.0
#
# Copyright (c) 2024 kwangdo.yi<kwangdo.yi@gmail.com>

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
