# SPDX-License-Identifier: MIT OR Apache-2.0
#
# Copyright (c) 2024 kwangdo.yi<kwangdo.yi@gmail.com>

KERNMODULES := core arch/common arch/ca-9 fs fs/slfs fs/littlefs libs drivers/uart drivers/dma drivers/odev drivers/ramdisk
KERNOUTDIR := $(addprefix $(TOPOUT)/kernel/,$(KERNMODULES)) 
KERNELDIR := $(shell pwd)

KERNCSRC := $(foreach sdir,$(KERNMODULES),$(wildcard $(sdir)/*.c))
KERNCOBJ := $(patsubst %.c,$(TOPOUT)/kernel/%.o,$(KERNCSRC))
KERNASMSRC := $(foreach sdir,$(KERNMODULES),$(wildcard $(sdir)/*.S))
KERNASMOBJ := $(patsubst %.S,$(TOPOUT)/kernel/%.o,$(KERNASMSRC))

INC := -I$(TOPOUT)/kernel/generated -I$(KERNELDIR)/inc -I$(KERNELDIR)/fs/littlefs
LDS :=$(KERNELDIR)/linker/kernel32.lds
CFLAGS := $(CFLAGS) -DAARCH32

.PHONY: gen_devicetree kernel

all: gen_devicetree kernel

vpath %.c $(KERNMODULES)
vpath %.S $(KERNMODULES)

define make-obj
$1/%.o: %.c
	$(CC) $(CFLAGS) $(INC) -o $$@ -c $$< -g -DLITTLEFS -D_ENABLE_SMP_ -mcpu=cortex-a9 -mfpu=vfpv3-d16 -mfloat-abi=hard -Wall -fno-omit-frame-pointer -ffreestanding

$1/%.o: %.S
	$(CC) $(INC) -o $$@ -c $$< -g -D_ENABLE_SMP_ -mcpu=cortex-a9 -mfpu=vfpv3-d16 -mfloat-abi=hard -Wall -fno-omit-frame-pointer -ffreestanding
endef

$(foreach bdir, $(KERNOUTDIR),$(eval $(call make-obj,$(bdir))))

kernel: gen_devicetree gen_config_hdr $(KERNOUTDIR) $(KERNCOBJ) $(KERNASMOBJ)
	$(CC) -Wl,-T -Wl,$(LDS) -o $(TOPOUT)/kernel/kernel32.elf $(KERNCOBJ) $(KERNASMOBJ) -Wl,-Map=$(TOPOUT)/kernel/kernel.map -nostdlib

gen_devicetree:
	mkdir -p $(TOPOUT)/kernel/generated
	python scripts/gen_devicetree_defs.py --soc zynq7000 > $(TOPOUT)/kernel/generated/generated_devicetree_defs.h

gen_config_hdr:
	python scripts/gen_kconfig_defs.py --defconfig configs/zynq7000_defconfig > $(TOPOUT)/kernel/generated/generated_kconfig_defs.h

$(KERNOUTDIR):
	mkdir -p $@
