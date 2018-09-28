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

KERNMODULES := core exception drivers/xil_standalone
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
	$(CC) $(CFLAGS) $(INC) -o $$@ -c $$< -g -mcpu=cortex-a9 -mfpu=vfpv3 -mfloat-abi=softfp

$1/%.o: %.S
	$(CC) $(INC) -o $$@ -c $$< -mcpu=cortex-a9 -mfpu=vfpv3 -mfloat-abi=softfp
endef

$(foreach bdir, $(KERNOUTDIR),$(eval $(call make-obj,$(bdir))))
$(foreach bdir, $(LIBOUTDIR),$(eval $(call make-obj,$(bdir))))

all: checkdirs $(LIBXIL) kernel.elf

checkdirs : $(LIBOUTDIR) $(KERNOUTDIR)

$(LIBOUTDIR) :
	@mkdir -p $@

$(KERNOUTDIR) :
	@mkdir -p $@

$(LIBXIL) : $(LIBCOBJ) $(LIBASMOBJ)
	$(AR) rc $(OUT_TOP)/libxil/$@ $(LIBCOBJ) $(LIBASMOBJ)

kernel.elf : $(KERNCOBJ) $(KERNASMOBJ)
	$(LD) -T $(LDS) -o $(OUT_TOP)/kernel/kernel.elf $(KERNCOBJ) $(KERNASMOBJ) -L$(OUT_TOP)/libxil -L$(LIBS) -L$(LIBS2) -lxil -lc -lgcc 

clean :
	@rm -rf $(OUT_TOP) libxil.a
	@rm -f libxil/*.a $(LIBCOBJ) $(LIBASMOBJ) 

