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

LIBMODULESTEMP:= uartps_v3_3
LIBMODULES:= $(addsuffix /src, $(LIBMODULESTEMP))
LIBSRCDIR := $(addprefix libsrcs/,$(LIBMODULES))
LIBOUTDIR := $(addprefix ../out/libxil/, $(LIBSRCDIR))
LIBCSRC := $(foreach sdir,$(LIBSRCDIR),$(wildcard $(sdir)/*.c))
LIBCOBJ := $(patsubst %.c,../out/libxil/%.o,$(LIBCSRC))
LIBASMSRC := $(foreach sdir,$(LIBSRCDIR),$(wildcard $(sdir)/*.S))
LIBASMOBJ := $(patsubst %.S,../out/libxil/%.o,$(LIBASMSRC))

INC := -I$(TOPDIR)/libxil/include -I$(TOPDIR)/kernel/inc

vpath %.c $(LIBSRCDIR)
vpath %.S $(LIBSRCDIR)

define make-obj
$1/%.o: %.c
	$(CC) $(CFLAGS) $(INC) -o $$@ -c $$< -g -mcpu=cortex-a9 -mfpu=vfpv3 -mfloat-abi=softfp -Wall -fno-omit-frame-pointer

$1/%.o: %.S
	$(CC) $(INC) -o $$@ -c $$< -mcpu=cortex-a9 -mfpu=vfpv3 -mfloat-abi=softfp -Wall -fno-omit-frame-pointer
endef

$(foreach bdir, $(LIBOUTDIR),$(eval $(call make-obj,$(bdir))))

libxil: $(LIBOUTDIR) $(LIBXIL)

$(LIBOUTDIR) :
	mkdir -p $@

$(LIBXIL) : $(LIBCOBJ) $(LIBASMOBJ)
	$(AR) rc $(TOPOUT)/libxil/$@ $(LIBCOBJ) $(LIBASMOBJ)
