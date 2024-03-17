# SPDX-License-Identifier: MIT OR Apache-2.0
#
# Copyright (c) 2024 kwangdo.yi<kwangdo.yi@gmail.com>

SRCS := $(wildcard *.c)
OBJS := $(patsubst %.c,../out/apps/%.o,$(SRCS))
APPSOUT := ../out/apps
APPS := $(APPSOUT)/helloworld

apps: mkdirs $(OBJS) 
	$(LD) -o $(APPS) $(OBJS) -nostdlib -e main -L$(TOPOUT)/libslos -lslos -static
	cp $(APPS) $(RAMDISKOUT)
	$(RAMDISKOUT)/mkfs $(RAMDISKOUT)/ramdisk.img $(RAMDISKOUT)/helloworld

mkdirs:
	mkdir -p $(APPSOUT)

$(OBJS): $(SRCS)
	$(CC) -o $@ -c $< -g 
