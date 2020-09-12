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
SRCS := $(wildcard *.c)
OBJS := $(patsubst %.c,../out/apps/%.o,$(SRCS))
APPSOUT := ../out/apps
APPS := $(APPSOUT)/helloworld

apps: mkdirs $(OBJS) 
	$(LD) -o $(APPS) $(OBJS) -nostdlib -e main -L$(TOPOUT)/libslos -lslos -static
	cp $(APPS) $(RAMDISKOUT)
	$(RAMDISKOUT)/mkfs $(RAMDISKOUT)/ramdisk.img $(RAMDISKOUT)/apps

mkdirs:
	mkdir -p $(APPSOUT)

$(OBJS): $(SRCS)
	$(CC) -o $@ -c $< -g 
