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
#$(info ${KERNCSRC})
KERNCOBJ := $(patsubst %.c,out/%.o,$(KERNCSRC))
#$(info ${KERNCOBJ})
KERNASMSRC := $(foreach sdir,$(KERNSRCDIR),$(wildcard $(sdir)/*.S))
KERNASMOBJ := $(patsubst %.S,out/%.o,$(KERNASMSRC))

#LIBMODULESTEMP:= canps_v3_2 coresightps_dcc_v1_3 cpu_cortexa9_v2_3 ddrps_v1_0 devcfg_v3_4 dmaps_v2_3 emacps_v3_3 generic_v2_0 gpiops_v3_1 iicps_v3_4 qspips_v3_3 scugic_v3_5 scutimer_v2_1 scuwdt_v2_1 sdps_v3_1 standalone_v6_1 ttcps_v3_2 uartps_v3_3 usbps_v2_4 xadcps_v2_2
LIBMODULESTEMP:= ddrps_v1_0 devcfg_v3_4 generic_v2_0 scugic_v3_5 uartps_v3_3 
LIBMODULES:= $(addsuffix /src, $(LIBMODULESTEMP))
LIBSRCDIR := $(addprefix libxil/libsrc/,$(LIBMODULES))
LIBOUTDIR := $(addprefix out/libxil/libsrc/,$(LIBMODULES)) 
#$(info ${LIBSRCDIR})
LIBCSRC := $(foreach sdir,$(LIBSRCDIR),$(wildcard $(sdir)/*.c))
#$(info ${LIBCSRC})
LIBCOBJ := $(patsubst %.c,out/%.o,$(LIBCSRC))
#$(info ${LIBCOBJ})
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

all: checkdirs $(LIBXIL) slos.img

checkdirs : $(LIBOUTDIR) $(KERNOUTDIR)

$(LIBOUTDIR) :
	@mkdir -p $@

$(KERNOUTDIR) :
	@mkdir -p $@

$(LIBXIL) : $(LIBCOBJ) $(LIBASMOBJ)
	$(AR) rc $(OUT_TOP)/libxil/$@ $(LIBCOBJ) $(LIBASMOBJ)

slos.img : $(KERNCOBJ) $(KERNASMOBJ)
	$(LD) -T $(LDS) -o $(OUT_TOP)/kernel/kernel.elf $(KERNCOBJ) $(KERNASMOBJ) -L$(OUT_TOP)/libxil -L$(LIBS) -L$(LIBS2) -lxil -lc -lgcc 
#$(OBJCOPY) -O binary $(OUT_TOP)/kernel.elf $(OUT_TOP)/kernel.bin 
#cp mkappfs/ramdisk.img $(OUT_TOP)/
#cp $(MISC_DIR)/emmc_appsboot_jump_to_pilot.mbn  $(OUT_TOP)/
#$(MISC_DIR)/mkbootimg --kernel $(OUT_TOP)/kernel.bin --ramdisk $(OUT_TOP)/ramdisk.img --ramdisk_offset 0x2000000 --pagesize 2048 --base 0x00000000 --kernel_offset 0x00008000 --output $(OUT_TOP)/slos.img


clean :
	@rm -rf $(OUT_TOP) libxil.a
	@rm -f libxil/*.a $(LIBCOBJ) $(LIBASMOBJ) 

