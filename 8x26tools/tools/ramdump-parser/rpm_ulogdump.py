# Copyright (c) 2013, The Linux Foundation. All rights reserved.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 and
# only version 2 as published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

import os
import re
import string
import sys
from print_out import *
from rpm_log_bfam import *

rpm_mem_offset = 0xfc000000
ULOG_VER3_STATUS_DEFINED = 1
ULOG_VER3_STATUS_ENABLED = 2
ULOG_VER3_STATUS_MPM_ACTIVE = 4

ULOG_SUBTYPE_RESERVED_FOR_RAW = 0


#offsets_rpm_ulog = [
#    ("((struct RPM_ULOG_STRUCT *)0x0)", "version", 0, 0),
#	("((struct RPM_ULOG_STRUCT *)0x0)", "name", 0, 0),
#	("((struct RPM_ULOG_STRUCT *)0x0)", "logStatus", 0, 0),
#]

def DumpRaw8(ramdump, buffer, mask, localRead, msgLength, ulog_out) :
	raw8DataLength = msgLength - 4
	ulog_out.write("- ")
	while (raw8DataLength != 0) :
		outChar = ramdump.read_byte(buffer + (localRead & mask), False)
		ulog_out.write("0x{0:0>2x}, ".format(outChar))
		localRead = localRead + 1
		raw8DataLength = raw8DataLength -1
	ulog_out.write("\n")
	
def DumpULogMsg(ramdump, buffer, mask, read, msgLength, ulog_out) :
	localRead = read + 4
	DumpRaw8(ramdump, buffer, mask, localRead, msgLength, ulog_out)

def DumpOneLog(ramdump, currentLog, ulog_out) :
#	logStatus_offset = ramdump.get_offset_struct("((struct RPM_ULOG_STRUCT *)0x0)", "logStatus")
	logStatus_offset = 32
	logStatus = ramdump.read_word(currentLog + logStatus_offset, False)
	if ((logStatus & ULOG_VER3_STATUS_ENABLED) != 0) :
#		buffer_offset = ramdump.get_offset_struct("((struct RPM_ULOG_STRUCT *)0x0)", "buffer")
		buffer_offset = 36
		buffer = ramdump.read_word(currentLog + buffer_offset, False)
		buffer = buffer + rpm_mem_offset
#		bufSize_offset = ramdump.get_offset_struct("((struct RPM_ULOG_STRUCT *)0x0)", "bufSize")
		bufSize_offset = 40
		logsize = ramdump.read_word(currentLog + bufSize_offset, False)
#		bufSizeMask_offset = ramdump.get_offset_struct("((struct RPM_ULOG_STRUCT *)0x0)", "bufSizeMask")
		bufSizeMask_offset = 44
		mask = ramdump.read_word(currentLog + bufSizeMask_offset, False)
		
#		readWriter_offset = ramdump.get_offset_struct("((struct RPM_ULOG_STRUCT *)0x0)", "readWriter")
		readWriter_offset = 60
		read = ramdump.read_word(currentLog + readWriter_offset, False)
#		read_offset = ramdump.get_offset_struct("((struct RPM_ULOG_STRUCT *)0x0)", "read")
		read_offset = 48
		readers_read = ramdump.read_word(currentLog + read_offset, False)
#		write_offset = ramdump.get_offset_struct("((struct RPM_ULOG_STRUCT *)0x0)", "write")
		write_offset = 56
		write = ramdump.read_word(currentLog + write_offset, False)
#		usageData_offset = ramdump.get_offset_struct("((struct RPM_ULOG_STRUCT *)0x0)", "usageData")
		usageData_offset = 64
		usageData = ramdump.read_word(currentLog + usageData_offset, False)
#		resetCount_offset = ramdump.get_offset_struct("((struct RPM_ULOG_STRUCT *)0x0)", "resetCount")
		resetCount_offset = 76
		resetCount = ramdump.read_word(currentLog + resetCount_offset, False)
		
#		print_out_str ("logStatus({0:x}) buffer({1:x}) logsize({2:x}) read({3:x}) readers_read({4:x}) write({5:x})".format(logStatus, buffer, logsize, read, readers_read, write))
		bytes_in_log = write - read
		if (bytes_in_log > logsize) :
			print_out_str ("DEBUG: Warning... (write - read) size of: {0:x} is bigger than the reported log size : {1:x}".format(bytes_in_log, logsize))
			print_out_str ("DEBUG: The most common cause of this is memory corruption, under-voltage, or writes to unpowered RAM.")
			print_out_str ("DEBUG: Will try to continue with this [&(logName)] ULog decode, but results may be unpredictable.")
		while (read < write) :
			read = read + 2
			msgLength = ramdump.read_halfword(buffer + (read & mask), False)
			read = read - 2
			format = ramdump.read_halfword(buffer + (read & mask), False)
#			print_out_str ("msgLength({0:x}) format({1:x})".format(msgLength, format))
			
			msgExtra = msgLength & 0x03
			if (msgExtra != 0) :
				msgExtra = 4 - msgExtra
		
			DumpULogMsg(ramdump, buffer, mask, read, msgLength, ulog_out)
			
			read = read + msgExtra + msgLength 

def print_rpm_log(ramdump, file_path) :
	rpm_elf_path = file_path + "/RPM_AAAAANAAR.elf"
	if (os.path.exists(rpm_elf_path) == 0) :
		print_out_str ("No RPM_AAAAANAAR.elf. please use proper elf file for rpm-log parsing")
		return

	coderam_path = file_path + "/CODERAM.BIN"
	if os.path.exists(coderam_path) :
		coderam = open(coderam_path,"rb")
		coderam_start = 0xfc100000
		coderam_end = coderam_start + os.path.getsize(coderam_path) - 1
		print_out_str ("Adding {0} {1:x}--{2:x}".format(coderam_path,coderam_start,coderam_end))
		ramdump.ebi_files.append((coderam,coderam_start,coderam_end,coderam_path))

	dataram_path = file_path + "/DATARAM.BIN"
	if os.path.exists(dataram_path) :
		dataram = open(dataram_path,"rb")
		dataram_start = 0xfc190000
		dataram_end = dataram_start + os.path.getsize(dataram_path) - 1
		print_out_str ("Adding {0} {1:x}--{2:x}".format(dataram_path,dataram_start,dataram_end))
		ramdump.ebi_files.append((dataram,dataram_start,dataram_end,dataram_path))

	stream = os.popen(ramdump.nm_path+" -n "+ rpm_elf_path)
	symbols = stream.readlines()
	for line in symbols :
		s = line.split(' ')
		if len(s) == 3:
			ramdump.addr_to_symbol_dict[int(s[0],16)] = s[2].rstrip()
			ramdump.symbol_to_addr_dict[s[2].rstrip()] = int(s[0],16)
			ramdump.lookup_table.append((int(s[0],16),s[2].rstrip()))
	stream.close()

#	ramdump.setup_offset_table(offsets_rpm_ulog)
	ulogContext = ramdump.addr_lookup("rpm_ulogContext")
	ulogContext = ulogContext + rpm_mem_offset
	logHead = ramdump.read_word(ulogContext, False)
	logHead = logHead + rpm_mem_offset
	if (logHead == 0) :
		print_out_str ("No ULogs Found. RPM_ulogContext.loghead (the ULog linked list ptr) is NULL")
		sys.exit(1)
#	version_offset = ramdump.get_offset_struct("((struct RPM_ULOG_STRUCT *)0x0)", "version")
	version_offset = 4
	name_offset = 8
	version = ramdump.read_word(logHead + version_offset, False)
	if (version == 0x2) or (version == 0x3) or (version == 0x4) :
		print_out_str ("This version of rpm_ulogdump.cmm is for RPM ULog version 0x1000 (Originating July 2012)")
		print_out_str ("It appears your logs are version : {0:x}".format(version))
		print_out_str ("Please check your source code for the correct rpm_ulogdump.cmm script")
		sys.exit(1)
	if (version != 0x1000) :
		print_out_str ("This version of rpm_ulogdump.cmm is for RPM ULog version 0x1000 (Originating July 2012)")
		print_out_str ("Your log version value returned : {0:x}".format(version))
		print_out_str ("Which doesn't match the ULog versions this script is familar with.")
		print_out_str ("Maybe the address your T32 session is using for the rpm_ulogContext is invalid?")
		sys.exit(1)
	currentLog = logHead
	while (currentLog != 0) :
#		name_offset = ramdump.get_offset_struct("((struct RPM_ULOG_STRUCT *)0x0)", "name")
		name_offset = 8	
		logName = ramdump.read_cstring(currentLog + name_offset, 24, False)
#LGE_CHANGE_S
		logName = logName.replace(' ', '_')
#LGE_CHANGE_E
#		print_out_str ("{0}".format(logName))
		ulog_out = open("{0}/{1}.ulog".format(ramdump.outdir, logName),"wb")
		
		DumpOneLog(ramdump, currentLog, ulog_out)
#		next_offset = ramdump.get_offset_struct("((struct RPM_ULOG_STRUCT *)0x0)", "next")
		next_offset = 0
		currentLog = ramdump.read_word(currentLog + next_offset, False)
#LGE_CHANGE_S
	ulog_out.close()
#LGE_CHANGE_E

	try :
#LGE_CHANGE_S
		rpm_log_bfam(ramdump, "8974", ramdump.outdir + "/RPM_External_Log.ulog")
#LGE_CHANGE_E
		print_out_str ("rpm log was extracted to ./rpm.log")
		return
	except :
		print_out_str ("Warning: something was wrong!")
