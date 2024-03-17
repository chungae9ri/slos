# SPDX-License-Identifier: MIT OR Apache-2.0
#
# Copyright (c) 2024 kwangdo.yi<kwangdo.yi@gmail.com>

MKFSSRCDIR := mkfs
MKFSOUTDIR := ../out/$(MKFSSRCDIR)
MKFSAPP := $(MKFSOUTDIR)/mkfs

mkfs: $(MKFSOUTDIR) 
	g++ -o mkfs  mkfs.cpp
	mv mkfs $(MKFSOUTDIR)
	cp $(MKFSAPP) $(RAMDISKOUT)

$(MKFSOUTDIR) :
	mkdir -p $@
