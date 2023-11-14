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
