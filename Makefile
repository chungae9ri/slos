#GCCINC :=$(HOME)/MentorGraphics/Sourcery_CodeBench_Lite_for_ARM_EABI/arm-none-eabi/include
CPPFLAG := -fno-exceptions -fno-rtti
LIBS := $(HOME)/bin/arm-2014.05/arm-none-eabi/lib
LIBS2 :=$(HOME)/bin/arm-2014.05/lib/gcc/arm-none-eabi/4.8.3
CC := arm-none-eabi-gcc
#CPP := arm-none-eabi-g++
ASM := arm-none-eabi-as
LD := arm-none-eabi-ld
AR := arm-none-eabi-ar
OBJCOPY :=arm-none-eabi-objcopy

TOP_DIR :=$(shell pwd)
OUT_TOP := $(TOP_DIR)/out

MODULES := arch core exception drivers/clock drivers/uart drivers/gpio
SRC_DIR := $(addprefix kernel/,$(MODULES))
OUT_DIR := $(addprefix out/,$(MODULES)) 
LIB_DIR := lib
WORKER_DIR := worker 
HELLO_DIR := helloworld
TEST1_DIR := test1
TEST2_DIR := test2

CPPSRC := $(foreach sdir,$(SRC_DIR),$(wildcard $(sdir)/*.cpp))
CPPOBJ := $(patsubst kernel/%.cpp,out/%.o,$(CPPSRC))

CSRC := $(foreach sdir,$(SRC_DIR),$(wildcard $(sdir)/*.c))
COBJ := $(patsubst kernel/%.c,out/%.o,$(CSRC))

ASMSRC := $(foreach sdir,$(SRC_DIR),$(wildcard $(sdir)/*.S))
ASMOBJ := $(patsubst kernel/%.S,out/%.o,$(ASMSRC))

LIBCSRC := $(foreach sdir,$(LIB_DIR),$(wildcard $(sdir)/*.c))
LIBASMSRC := $(foreach sdir,$(LIB_DIR),$(wildcard $(sdir)/*.S))
LIBCOBJ := $(patsubst lib/%.c,lib/%.o,$(LIBCSRC))
LIBASMOBJ := $(patsubst lib/%.S,lib/%.o,$(LIBASMSRC))

WORKERCSRC := $(foreach sdir,$(WORKER_DIR),$(wildcard $(sdir)/*.c))
WORKERASMSRC := $(foreach sdir,$(WORKER_DIR),$(wildcard $(sdir)/*.S))
WORKERCOBJ := $(patsubst worker/%.c,worker/%.o,$(WORKERCSRC))
WORKERASMOBJ := $(patsubst worker/%.S,worker/%.o,$(WORKERASMSRC))


HELLOCSRC := $(foreach sdir,$(HELLO_DIR),$(wildcard $(sdir)/*.c))
HELLOASMSRC := $(foreach sdir,$(HELLO_DIR),$(wildcard $(sdir)/*.S))
HELLOCOBJ := $(patsubst helloworld/%.c,helloworld/%.o,$(HELLOCSRC))
HELLOASMOBJ := $(patsubst helloworld/%.S,helloworld/%.o,$(HELLOASMSRC))

TEST1CSRC := $(foreach sdir,$(TEST1_DIR),$(wildcard $(sdir)/*.c))
TEST1ASMSRC := $(foreach sdir,$(TEST1_DIR),$(wildcard $(sdir)/*.S))
TEST1COBJ := $(patsubst test1/%.c,test1/%.o,$(TEST1CSRC))
TEST1ASMOBJ := $(patsubst test1/%.S,test1/%.o,$(TEST1ASMSRC))

TEST2CSRC := $(foreach sdir,$(TEST2_DIR),$(wildcard $(sdir)/*.c))
TEST2ASMSRC := $(foreach sdir,$(TEST2_DIR),$(wildcard $(sdir)/*.S))
TEST2COBJ := $(patsubst test2/%.c,test2/%.o,$(TEST2CSRC))
TEST2ASMOBJ := $(patsubst test2/%.S,test2/%.o,$(TEST2ASMSRC))

INC := -I$(TOP_DIR)/kernel/inc
LDS :=$(TOP_DIR)/kernel/linker/kernel.lds
SCL := $(TOP_DIR)/kernel/linker/kernel.scl
MISC_DIR := $(TOP_DIR)/misc

vpath %.c $(SRC_DIR)
vpath %.cpp $(SRC_DIR)
vpath %.S $(SRC_DIR)

define make-obj
$1/%.o: %.c
	$(CC) $(INC) -o $$@ -c $$< -g

#$1/%.o: %.cpp
#	$(CPP) $(INC) $(CPPFLAG) -o $$@ -c $$< -g

$1/%.o: %.S
	$(CC) $(INC) -o $$@ -c $$< 
endef

$(foreach bdir, $(OUT_DIR),$(eval $(call make-obj,$(bdir))))

#TARGET := $(OUT_TOP)/slos.img

all: checkdirs app ramdisk slos

slos : checkdirs app ramdisk $(COBJ) $(CPPOBJ) $(ASMOBJ)
	$(LD) -T $(LDS) -o $(OUT_TOP)/kernel.elf $(COBJ) $(CPPOBJ) $(ASMOBJ) -L$(LIBS) -L$(LIBS2) -lc -lgcc 
	$(OBJCOPY) -O binary $(OUT_TOP)/kernel.elf $(OUT_TOP)/kernel.bin 
	cp mkappfs/ramdisk.img $(OUT_TOP)/
	cp $(MISC_DIR)/emmc_appsboot_jump_to_pilot.mbn  $(OUT_TOP)/
	$(MISC_DIR)/mkbootimg --kernel $(OUT_TOP)/kernel.bin --ramdisk $(OUT_TOP)/ramdisk.img --ramdisk_offset 0x2000000 --pagesize 2048 --base 0x00000000 --kernel_offset 0x00008000 --output $(OUT_TOP)/slos.img

checkdirs : $(OUT_DIR)

$(OUT_DIR) :
	@mkdir -p $@

app : lib/libslos.a worker/worker helloworld/helloworld test1/test1 test2/test2

lib/libslos.a : $(LIBCOBJ) $(LIBASMOBJ)
	$(AR) rc $@ $(LIBCOBJ) $(LIBASMOBJ)

worker/worker: $(WORKERCOBJ)
	$(CC) -o $@ $< -L$(TOP_DIR)/lib -lslos -static
	

helloworld/helloworld : $(HELLOCOBJ)
	$(CC) -o $@ $< -L$(TOP_DIR)/lib -lslos -static
	
test1/test1 : $(TEST1COBJ)
	$(CC) -o $@ $< -L$(TOP_DIR)/lib -lslos -static

test2/test2: $(TEST2COBJ)
	$(CC) -o $@ $< -L$(TOP_DIR)/lib -lslos -static

ramdisk : app mkappfs/mkappfs
	mkappfs/mkappfs mkappfs/ramdisk.img worker/worker helloworld/helloworld test1/test1 test2/test2

mkappfs/mkappfs : mkappfs/mkappfs.cpp
	g++ -o $@ $<

clean :
	@rm -rf $(OUT_TOP) a
	@rm -f lib/*.a $(LIBCOBJ) $(LIBASMOBJ) 
	@rm -f worker/worker $(WORKERCOBJ) $(WORKERASMOBJ) 
	@rm -f helloworld/helloworld $(HELLOCOBJ) $(HELLOASMOBJ) 
	@rm -f test1/test1 $(TEST1COBJ) $(TEST1ASMOBJ) 
	@rm -f test2/test2 $(TEST2COBJ) $(TEST2ASMOBJ)
	@rm -f mkappfs/mkappfs mkappfs/ramdisk.img

