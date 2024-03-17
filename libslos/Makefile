# SPDX-License-Identifier: MIT OR Apache-2.0
#
# Copyright (c) 2024 kwangdo.yi<kwangdo.yi@gmail.com>

LIBSRCDIR := libslos
LIBOUTDIR := ../out/$(LIBSRCDIR)
LIBCSRC := $(wildcard *.c)
LIBCOBJ := $(patsubst %.c,../out/libslos/%.o,$(LIBCSRC))
LIBASMSRC := $(wildcard *.S)
LIBASMOBJ := $(patsubst %.S,../out/libslos/%.o,$(LIBASMSRC))
LIBSLOS := $(LIBOUTDIR)/libslos.a

$(LIBCOBJ): $(LIBCSRC)
	$(CC) $(CFLAGS) $(INC) -o $@ -c $< -g -mcpu=cortex-a9 -mfpu=vfpv3 -mfloat-abi=softfp -Wall -fno-omit-frame-pointer

$(LIBASMOBJ): $(LIBASMSRC)
	$(CC) $(INC) -o $@ -c $< -mcpu=cortex-a9 -mfpu=vfpv3 -mfloat-abi=softfp -Wall -fno-omit-frame-pointer

libslos: $(LIBOUTDIR) $(LIBSLOS)

$(LIBOUTDIR) :
	mkdir -p $@

$(LIBSLOS) : $(LIBCOBJ) $(LIBASMOBJ)
	$(AR) rc $@ $^
	cp $@ $(RAMDISKOUT)
