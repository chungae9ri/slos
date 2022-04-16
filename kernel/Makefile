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

KERNMODULES := core arm drivers/uart drivers/dma drivers/odev 
KERNOUTDIR := $(addprefix ../out/kernel/,$(KERNMODULES)) 

KERNCSRC := $(foreach sdir,$(KERNMODULES),$(wildcard $(sdir)/*.c))
KERNCOBJ := $(patsubst %.c,../out/kernel/%.o,$(KERNCSRC))
KERNASMSRC := $(foreach sdir,$(KERNMODULES),$(wildcard $(sdir)/*.S))
KERNASMOBJ := $(patsubst %.S,../out/kernel/%.o,$(KERNASMSRC))

INC := -I$(TOPDIR)/libxil/include -I$(TOPDIR)/kernel/inc
LDS :=$(TOPDIR)/kernel/linker/kernel.lds

vpath %.c $(KERNMODULES)
vpath %.S $(KERNMODULES)

define make-obj
$1/%.o: %.c
	$(CC) $(CFLAGS) $(INC) -o $$@ -c $$< -g -D_ENABLE_SMP_ -mcpu=cortex-a9 -mfpu=vfpv3 -mfloat-abi=softfp -Wall -fno-omit-frame-pointer -ffreestanding

$1/%.o: %.S
	$(CC) $(INC) -o $$@ -c $$< -g -D_ENABLE_SMP_ -mcpu=cortex-a9 -mfpu=vfpv3 -mfloat-abi=softfp -Wall -fno-omit-frame-pointer -ffreestanding
endef

$(foreach bdir, $(KERNOUTDIR),$(eval $(call make-obj,$(bdir))))

kernel: $(KERNOUTDIR) $(KERNCOBJ) $(KERNASMOBJ)
	$(LD) -T $(LDS) -o $(TOPOUT)/kernel/kernel.elf $(KERNCOBJ) $(KERNASMOBJ) -Map=$(TOPOUT)/kernel/kernel.map -nostdlib

$(KERNOUTDIR):
	mkdir -p $@
