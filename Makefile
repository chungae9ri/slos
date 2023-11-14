export PATH:=$(HOME)/bin/gcc-arm-none-eabi-10.3-2021.10/bin:$(PATH)
LIBS := $(HOME)/bin/gcc-arm-none-eabi-10.3-2021.10/arm-none-eabi/lib
LIBS2 :=$(HOME)/bin/gcc-arm-none-eabi-10.3-2021.10/lib/gcc/arm-none-eabi/10.3.1
CC := arm-none-eabi-gcc
ASM := arm-none-eabi-as
LD := arm-none-eabi-ld
AR := arm-none-eabi-ar
OBJCOPY :=arm-none-eabi-objcopy

TOP_DIR :=$(shell pwd)
OUT_TOP := $(TOP_DIR)/out

KERNMODULES := core exception drivers/uart
KERNSRCDIR := $(addprefix kernel/,$(KERNMODULES))
KERNOUTDIR := $(addprefix out/kernel/,$(KERNMODULES)) 

KERNCSRC := $(foreach sdir,$(KERNSRCDIR),$(wildcard $(sdir)/*.c))
KERNCOBJ := $(patsubst %.c,out/%.o,$(KERNCSRC))
KERNASMSRC := $(foreach sdir,$(KERNSRCDIR),$(wildcard $(sdir)/*.S))
KERNASMOBJ := $(patsubst %.S,out/%.o,$(KERNASMSRC))

INC := -I$(TOP_DIR)/kernel/inc
LDS :=$(TOP_DIR)/kernel/linker/kernel.lds

vpath %.c $(KERNSRCDIR)
vpath %.S $(KERNSRCDIR)

define make-obj
$1/%.o: %.c
	$(CC) $(CFLAGS) $(INC) -o $$@ -c $$< -g -marm -mcpu=cortex-a9 -mfpu=vfpv3 -mfloat-abi=hard

$1/%.o: %.S
	$(CC) $(INC) -o $$@ -c $$< -marm -mcpu=cortex-a9 -mfpu=vfpv3 -mfloat-abi=hard
endef

$(foreach bdir, $(KERNOUTDIR),$(eval $(call make-obj,$(bdir))))
$(foreach bdir, $(LIBOUTDIR),$(eval $(call make-obj,$(bdir))))

all: checkdirs kernel.elf

checkdirs : $(LIBOUTDIR) $(KERNOUTDIR)

$(LIBOUTDIR) :
	@mkdir -p $@

$(KERNOUTDIR) :
	@mkdir -p $@

kernel.elf : $(KERNCOBJ) $(KERNASMOBJ)
	$(LD) -T $(LDS) -o $(OUT_TOP)/kernel/kernel.elf $(KERNCOBJ) $(KERNASMOBJ) -L$(LIBS) -L$(LIBS2) -lc -lgcc 

clean :
	@rm -rf $(OUT_TOP)
	@rm -f $(LIBCOBJ) $(LIBASMOBJ) 

