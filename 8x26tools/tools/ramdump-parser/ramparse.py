#!/usr/bin/python

# Copyright (c) 2012-2013, The Linux Foundation. All rights reserved.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 and
# only version 2 as published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

import sys
import re
import os
import struct
import datetime
import array
import string
import bisect
import traceback
import platform
from print_out import *
from subprocess import *
from optparse import OptionParser
from optparse import OptionGroup
from struct import unpack
from ctypes import *
from optparse import *
from ramdump import *
from unwind import *
from rtb import *
from slabinfo import *
from irqstate import *
from taskdump import *
from watchdog import *
from workqueue import *
from ramdump import *
from cachedump import *
from vmalloc import *
import iommu
from debug_image import *
from pagetypeinfo import *
from dmesg import *
from gpuinfo import *
# LGE_CHANGE_S
from logger import *
from rpm_ulogdump import *
from TzDumpParser import *
from ramconsole import *
# LGE_CHANGE_E

#Please update version when something is changed!'
VERSION = '2.0'

def which(program):
    for path in os.environ["PATH"].split(os.pathsep):
        exe_file = os.path.join(path, program)
        if os.access(exe_file, os.X_OK):
            return exe_file

    return None

def check_for_panic(dump, unwind) :
    addr = dump.addr_lookup("in_panic")

    result = dump.read_word(addr)

    if (result is not None and result == 1) :
        print_out_str ("-------------------------------------------------")
        print_out_str ("[!] KERNEL PANIC detected!")
        print_out_str ("-------------------------------------------------")
        do_dump_stacks(dump, unwind, 1)
    else :
        print_out_str ("No kernel panic detected")


def parse_ram_file(option, opt_str, value, parser) :
    a = getattr(parser.values,option.dest)
    if a is None:
        a = []
    temp = []
    for arg in parser.rargs:
        if arg[:2] == "--":
            break
        if arg[:1] == "-" and len(arg) > 1:
            break
        temp.append(arg)

    if len(temp) is not 3:
        raise OptionValueError("Ram files must be specified in 'name, start, end' format")

    a.append((temp[0],int(temp[1],16),int(temp[2],16)))
    setattr(parser.values,option.dest, a)

def get_system_type() :
    plat = platform.system()

    if plat == 'Windows' :
       return 'Windows'
    if re.search('CYGWIN',plat) is not None :
       # On certain installs, the default windows shell
       # runs cygwin. Treat cygwin as windows for this
       # purpose
       return 'Windows'
    if plat == 'Linux' :
       return 'Linux'

    if plat == 'Darwin' :
       return 'Darwin'

    print_out_str ("[!!!] This is a target I don't recognize!")
    print_out_str ("[!!!] Some features may not work as expected!")
    print_out_str ("[!!!] Assuming Linux...")

if __name__ == "__main__":
    usage = "usage: %prog [options to print]. Run with --help for more details"
    parser = OptionParser(usage)
    parser.add_option("-d", "--dmesg", action="store_true", dest="dmesg", help="print dmesg", default = False)
    parser.add_option("-t", "--print-tasks", action="store_true", dest="tasks", help="Print all the task information", default = False)
    parser.add_option("-i", "--print-irqs", action="store_true", dest="irqs", help="Print all the irq information", default = False)
    parser.add_option("-c", "--print-kconfig", action="store_true", dest="kconfig", help="print kernel configuration")
    parser.add_option("-q", "--print-workqueues", action="store_true", dest="workqueues", help="Print all the workqueue information", default = False)
    parser.add_option("", "--print-watchdog-time", action="store_true", dest="watchdog_time", help="Print watchdog timing information", default = False)
    parser.add_option("", "--print-iommu-pg-tables", action="store_true", dest="iommu_pg_tables", help="Print IOMMU page tables", default = False)
    parser.add_option("-w", "--check-for-watchdog", action="store_true", dest="watchdog", help="Check for watchdog bark", default = False)
    parser.add_option("-p", "--check-for-panic", action="store_true", dest="panic", help="Check for kernel panic", default = False)
    parser.add_option("-e", "--ram-file", dest="ram_addr", help="List of ram files (name, start, end)", action="callback", callback=parse_ram_file)
    parser.add_option("-v", "--vmlinux", dest="vmlinux", help="vmlinux path")
    parser.add_option("-n", "--nm-path", dest="nm", help="nm path")
    parser.add_option("-g", "--gdb-path", dest="gdb", help="gdb path")
    parser.add_option("-a", "--auto-dump", dest="autodump", help="Auto find ram dumps from the path")
    parser.add_option("-o", "--outdir", dest="outdir", help="Output directory")
    parser.add_option("-s", "--t32launcher", action="store_true", dest="t32launcher", help="Create T32 simulator launcher", default=False)
    parser.add_option("-r", "--print-rtb", action="store_true", dest="rtb", help="Print RTB (if enabled)", default=False)
    parser.add_option("-x", "--everything", action="store_true", dest="everything", help="Output everything (may be slow")
    parser.add_option("-f", "--output-file", dest="outfile", help="Name of file to save output")
    parser.add_option("", "--stdout", action="store_true", dest="stdout", help="Dump to stdout instead of the file")
    parser.add_option("", "--slabinfo", action="store_true", dest="slabinfo", help="print slab information (EXPERIMENTAL)")
    parser.add_option("", "--print-cache-dump", action="store_true", dest="l2dump", help="print L2 cache dump")
    parser.add_option("", "--phys-offset", type="int", dest="phys_offset", help="use custom phys offset")
    parser.add_option("", "--print-vmalloc", action="store_true", dest="print_vmalloc", help="print vmalloc info")
    parser.add_option("", "--force-hardware", type="int", dest="force_hardware", help="Force the hardware detection")
    parser.add_option("", "--force-version", type="int", dest="force_hardware_version", help="Force the hardware detection to a specific hardware version")
    parser.add_option("", "--parse-debug-image", action="store_true", dest="debug_image", help="Parse the debug image")
    parser.add_option("", "--parse-qdss", action="store_true", dest="qdss", help="Parse QDSS")
    parser.add_option("", "--print-pagetypeinfo", action="store_true", dest="pagetypeinfo", help="print pagetypeinfo")
    parser.add_option("", "--print-gpuinfo", action="store_true", dest="gpuinfo", help="print gpu info like ringbuffer,snapshot and pointer addresses", default = False)
# LGE_CHANGE_S
    parser.add_option("-l", "--logger", action="store_true", dest="logger", help="print android logger (LGE)")
    parser.add_option("-z", "--tz-log", action="store_true", dest="tz_log", help="dump tz log")
    parser.add_option("-m", "--rpm-log", action="store_true", dest="rpm_log", help="dump rpm log")
# LGE_CHANGE_E
    (options, args) = parser.parse_args()

    if options.outdir :
        if not os.path.exists(options.outdir) :
            print ("!!! Out directory does not exist. Create it first.")
            sys.exit(1)
    else :
        options.outdir = "."

    if options.outfile is None :
        # dmesg_TZ is a very non-descriptive name and should be changed sometime in the future
        options.outfile = "dmesg_TZ.txt"

    if not options.stdout :
        set_outfile(options.outdir+"/"+options.outfile)

    print_out_str ("Linux Ram Dump Parser Version %s" % VERSION)
    if options.vmlinux is None :
        print_out_str ("No vmlinux given. I can't proceed!")
        parser.print_usage()
        sys.exit(1)

    args = ""
    for arg in sys.argv:
      args = args + arg + " "

    print_out_str ("Arguments: {0}".format(args))

    system_type = get_system_type()

    if not os.path.exists(options.vmlinux) :
        print_out_str ("{0} does not exist. Cannot proceed without vmlinux. Exiting...".format(options.vmlinux))
        sys.exit(1)
    else :
        print_out_str ("using vmlinx file {0}".format(options.vmlinux))

    if options.ram_addr is None and options.autodump is None :
        print_out_str ("Need one of --auto-dump or at least one --ram-file")
        sys.exit(1)

    if options.ram_addr is not None :
        for a in options.ram_addr :
            if os.path.exists(a[0]) :
                print_out_str ("Loading Ram file {0} from {1:x}--{2:x}".format(a[0],a[1],a[2]))
            else :
                print_out_str ("Ram file {0} does not exist. Exiting...".format(a[0]))
                sys.exit(1)

    if options.autodump is not None :
        if os.path.exists(options.autodump) :
            print_out_str ("Looking for Ram dumps in {0}".format(options.autodump))
        else :
            print_out_str ("Path {0} does not exist for Ram dumps. Exiting...".format(options.autodump))
            sys.exit(1)


    if not options.gdb :
        gdb_candidates = ['arm-none-eabi-gdb.exe',
                          'arm-none-linux-gnueabi-gdb',
                          'arm-eabi-gdb',
                          'arm-linux-androideabi-gdb']
        for c in gdb_candidates:
            gdb_path = which(c)
            if gdb_path is not None:
                break

        if gdb_path is None:
            if system_type is 'Windows' :
                gdb_path = "arm-linux-androideabi-gdb.exe"
            elif system_type is 'Linux':
                gdb_path = "arm-eabi-gdb"
            elif system_type is 'Darwin':
                gdb_path = "arm-linux-androideabi-gdb"
            else :
                print_out_str ("This is not a recognized system type! Exiting...")
                sys.exit(1)
        print_out_str ("No gdb path given, using {0}".format(gdb_path))
    else :
        gdb_path = options.gdb
        print_out_str ("gdb path = "+gdb_path)

    if not options.nm :
        nm_candidates = ['arm-none-eabi-nm.exe',
                         'arm-none-linux-gnueabi-nm',
                         'arm-eabi-nm',
                         'arm-linux-androideabi-nm']
        for c in nm_candidates:
            nm_path = which(c)
            if nm_path is not None:
                break

        if nm_path is None:
            if system_type is 'Windows' :
                nm_path =  "arm-linux-androideabi-nm.exe"
            elif system_type is 'Linux' :
                nm_path = "arm-eabi-nm"
            elif system_type is 'Darwin':
                nm_path = "arm-linux-androideabi-nm"
            else :
                print_out_str ("This is not a recognized system type! Exiting...")
                sys.exit(1)
        print_out_str ("No nm path given, using {0}".format(nm_path))
    else :
        nm_path = options.nm
        print_out_str ("nm path + "+nm_path)

    dump = RamDump(options.vmlinux, nm_path, gdb_path, options.ram_addr,
                   options.autodump, options.phys_offset, options.outdir,
                   options.force_hardware, options.force_hardware_version)

    if not dump.print_command_line() :
        print_out_str ("!!! Error printing saved command line.")
        print_out_str ("!!! The vmlinux is probably wrong for the ramdumps")
        print_out_str ("!!! Exiting now...")
        sys.exit(1)

    unwind = Unwinder(dump)

# LGE_CHANGE_S
    print_out_str ("\n---- crash handler log -----")
    print_crash_log(dump)
    print_out_str ("---- end crash handler log -----\n")
# LGE_CHANGE_E

    if options.panic or options.everything :
        check_for_panic(dump, unwind)

    if options.watchdog or options.everything :
        print_out_str ("\n------ watchdog state ------")
        dump_watchdog_state(dump, unwind)
        print_out_str ("-------- end watchdog state -------")

    if options.watchdog_time :
        print_out_str ("\n--------- watchdog time -------")
        get_wdog_timing(dump)
        print_out_str ("---------- end watchdog time-----")

    if options.iommu_pg_tables :
        iommu_obj = iommu.IOMMU(dump)
        iommu_obj.iommu_parse()
    
#RPM_LOG_PARSE_S
    if options.rpm_log or options.everything :
        if options.autodump :
            print_out_str ("\n---- print_rpm_log -----")
            print_rpm_log(dump, options.autodump)
            print_out_str ("---- end print_rpm_log -----")
#RPM_LOG_PARSE_E

    if options.dmesg or options.everything :
        print_out_str ("\n---- dmesg -----")
        extract_dmesg(dump)
        print_out_str ("---- end dmesg----")

# LGE_CHANGE_S
    if options.dmesg or options.everything :
        print_out_str ("\n---- last_kmsg -----")
        print_last_kmsg(dump)
        print_out_str ("---- end last_kmsg -----")

    if options.dmesg or options.everything :
        print_out_str ("\n---- ram console -----")
        print_ramconsole(dump)
        print_out_str ("---- end ram console -----\n")
# LGE_CHANGE_E

    if options.tasks or options.everything :
        do_dump_stacks(dump, unwind, 0)

    if options.workqueues :
        print_workqueue_state(dump)

    if options.irqs or options.everything :
        print_irq_state(dump)

    if options.kconfig or options.everything :
        print_out_str ("\n---------config-------")
        dump.print_config()
        print_out_str ("----------end config-----")

    if options.rtb or options.everything :
        rtb = RTB(dump)
        rtb.rtb_parse()

    if options.t32launcher or options.everything :
        dump.create_t32_launcher()

    if options.slabinfo :
        print_slab_info(dump)

    if options.l2dump :
        print_cache_dump(dump)

    if options.print_vmalloc or options.everything :
        print_vmalloc(dump)

    if options.debug_image or options.everything :
        debugImage = DebugImage(dump, unwind)
        debugImage.debug_image_parse()

    if options.qdss or options.everything :
        dump.dump_qdss()

    if options.pagetypeinfo or options.everything :
        print_out_str ("\n---------pagetypeinfo-------")
        print_pagetypeinfo(dump)
        print_out_str ("----------end pagetypeinfo-----")

    if options.gpuinfo or options.everything :
        print_out_str ("\n---------gpuinfo-------")
        extract_gpuinfo(dump)
        print_out_str ("\n---------gpuinfo-------")
# LGE_CHANGE_S
    if options.logger or options.everything:
        print_logger(dump)

    if options.tz_log or options.everything:
        if options.autodump is not None :
            TzDumpParser(dump, options.autodump)
# LGE_CHANGE_E
