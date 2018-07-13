# (C) 2018 Kwangdo Yi <kwangdo.yi@gmail.com>
 
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

export PATH:=$(HOME)/bin/arm-2017q1/bin:$(PATH)

LIBS := $(HOME)/bin/arm-2017q1/arm-none-eabi/lib
LIBS2 :=$(HOME)/bin/arm-2017q1/lib/gcc/arm-none-eabi/6.3.1
CC := arm-none-eabi-gcc
ASM := arm-none-eabi-as
LD := arm-none-eabi-ld
AR := arm-none-eabi-ar
OBJCOPY :=arm-none-eabi-objcopy
LIBXIL := libxil.a

TOP_DIR :=$(shell pwd)
OUT_TOP := $(TOP_DIR)/out

KERNMODULES := core exception drivers/xil_standalone drivers/dma
KERNSRCDIR := $(addprefix kernel/,$(KERNMODULES))
KERNOUTDIR := $(addprefix out/kernel/,$(KERNMODULES)) 

KERNCSRC := $(foreach sdir,$(KERNSRCDIR),$(wildcard $(sdir)/*.c))
KERNCOBJ := $(patsubst %.c,out/%.o,$(KERNCSRC))
KERNASMSRC := $(foreach sdir,$(KERNSRCDIR),$(wildcard $(sdir)/*.S))
KERNASMOBJ := $(patsubst %.S,out/%.o,$(KERNASMSRC))

LIBMODULESTEMP:= uartps_v3_3 
LIBMODULES:= $(addsuffix /src, $(LIBMODULESTEMP))
LIBSRCDIR := $(addprefix libxil/libsrc/,$(LIBMODULES))
LIBOUTDIR := $(addprefix out/libxil/libsrc/,$(LIBMODULES)) 
LIBCSRC := $(foreach sdir,$(LIBSRCDIR),$(wildcard $(sdir)/*.c))
LIBCOBJ := $(patsubst %.c,out/%.o,$(LIBCSRC))
LIBASMSRC := $(foreach sdir,$(LIBSRCDIR),$(wildcard $(sdir)/*.S))
LIBASMOBJ := $(patsubst %.S,out/%.o,$(LIBASMSRC))

INC := -I$(TOP_DIR)/kernel/inc -I$(TOP_DIR)/libxil/include
LDS :=$(TOP_DIR)/kernel/linker/kernel.lds

vpath %.c $(KERNSRCDIR)
vpath %.S $(KERNSRCDIR)
vpath %.c $(LIBSRCDIR)
vpath %.S $(LIBSRCDIR)

define make-obj
$1/%.o: %.c
	$(CC) $(CFLAGS) $(INC) -o $$@ -c $$< -g -mcpu=cortex-a9 -mfpu=vfpv3 -mfloat-abi=softfp -Wall -fno-omit-frame-pointer

$1/%.o: %.S
	$(CC) $(INC) -o $$@ -c $$< -mcpu=cortex-a9 -mfpu=vfpv3 -mfloat-abi=softfp -Wall -fno-omit-frame-pointer
endef

$(foreach bdir, $(KERNOUTDIR),$(eval $(call make-obj,$(bdir))))
$(foreach bdir, $(LIBOUTDIR),$(eval $(call make-obj,$(bdir))))

all: checkdirs $(LIBXIL) kernel.elf ramdisk 

checkdirs : $(LIBOUTDIR) $(KERNOUTDIR)

$(LIBOUTDIR) :
	mkdir -p $@

$(KERNOUTDIR) :
	mkdir -p $@
	mkdir -p $(OUT_TOP)/ramdisk
	mkdir -p $(OUT_TOP)/libslos

$(LIBXIL) : $(LIBCOBJ) $(LIBASMOBJ)
	$(AR) rc $(OUT_TOP)/libxil/$@ $(LIBCOBJ) $(LIBASMOBJ)

kernel.elf : $(KERNCOBJ) $(KERNASMOBJ)
	$(LD) -T $(LDS) -o $(OUT_TOP)/kernel/kernel.elf $(KERNCOBJ) $(KERNASMOBJ) -L$(OUT_TOP)/libxil -L$(LIBS) -L$(LIBS2) -lxil -lc -lgcc 

APPS := $(OUT_TOP)/ramdisk/helloworld

ramdisk : $(OUT_TOP)/libslos/libslos.a $(OUT_TOP)/ramdisk/mkfs $(APPS)
	$(OUT_TOP)/ramdisk/mkfs $(OUT_TOP)/ramdisk/ramdisk.img $(APPS)

$(OUT_TOP)/libslos/libslos.a : $(OUT_TOP)/libslos/syscall.o $(OUT_TOP)/libslos/print_mesg.o 
	$(AR) rc $@ $^

$(OUT_TOP)/libslos/syscall.o : $(TOP_DIR)/libslos/syscall.S
	$(CC) -o $@ -c $<

$(OUT_TOP)/libslos/print_mesg.o : $(TOP_DIR)/libslos/print_mesg.c
	$(CC) -o $@ -c $<
	
$(OUT_TOP)/ramdisk/mkfs : mkfs/mkfs.cpp
	g++ -o $@ $<

$(APPS) : apps/helloworld.c
	$(CC) -o $(OUT_TOP)/ramdisk/helloworld.o -c $< -g 
	$(LD) -o $(APPS) $(OUT_TOP)/ramdisk/helloworld.o -nostdlib -e main -L$(OUT_TOP)/libslos -lslos -static

clean :
	rm -rf $(OUT_TOP) libxil.a
	rm -f libxil/*.a $(LIBCOBJ) $(LIBASMOBJ) 
