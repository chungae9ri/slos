# SPDX-License-Identifier: MIT OR Apache-2.0
#
# Copyright (c) 2024 kwangdo.yi<kwangdo.yi@gmail.com>

KERNMODULES := core arch/ca-9 fs fs/slfs fs/littlefs libs drivers/uart drivers/dma drivers/odev drivers/ramdisk
KERNOUTDIR := $(addprefix ../out/kernel/,$(KERNMODULES)) 
KERNELDIR := $(shell pwd)

KERNCSRC := $(foreach sdir,$(KERNMODULES),$(wildcard $(sdir)/*.c))
KERNCOBJ := $(patsubst %.c,../out/kernel/%.o,$(KERNCSRC))
KERNASMSRC := $(foreach sdir,$(KERNMODULES),$(wildcard $(sdir)/*.S))
KERNASMOBJ := $(patsubst %.S,../out/kernel/%.o,$(KERNASMSRC))

INC := -I$(KERNELDIR)/inc -I$(KERNELDIR)/fs/littlefs
LDS :=$(KERNELDIR)/linker/kernel32.lds

vpath %.c $(KERNMODULES)
vpath %.S $(KERNMODULES)

define make-obj
$1/%.o: %.c
	$(CC) $(CFLAGS) $(INC) -o $$@ -c $$< -g -DLITTLEFS -DFS_USE_SLFS -D_ENABLE_SMP_ -mcpu=cortex-a9 -mfpu=vfpv3 -mfloat-abi=hard -Wall -fno-omit-frame-pointer -ffreestanding

$1/%.o: %.S
	$(CC) $(INC) -o $$@ -c $$< -g -D_ENABLE_SMP_ -mcpu=cortex-a9 -mfpu=vfpv3 -mfloat-abi=hard -Wall -fno-omit-frame-pointer -ffreestanding
endef

$(foreach bdir, $(KERNOUTDIR),$(eval $(call make-obj,$(bdir))))

kernel: $(KERNOUTDIR) $(KERNCOBJ) $(KERNASMOBJ)
	$(CC) -Wl,-T -Wl,$(LDS) -o $(TOPOUT)/kernel/kernel.elf $(KERNCOBJ) $(KERNASMOBJ) -Wl,-Map=$(TOPOUT)/kernel/kernel.map -nostdlib

$(KERNOUTDIR):
	mkdir -p $@
