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
from subprocess import *
from optparse import OptionParser
from optparse import OptionGroup
from struct import unpack
from ctypes import *
from tempfile import *
from print_out import *
from watchdog import *
from cachedump import *
from qdss import *

QDSS_MAGIC = 0x5D1DB1Bf

print_table = {
        "MSM_CPU_CTXT" : "parse_cpu_ctx",
        "MSM_L1_CACHE" : "parse_l1_cache",
        "MSM_L2_CACHE" : "parse_l2_cache",
        "MSM_OCMEM" : "parse_ocmem",
        "MSM_TMC0_REG" : "parse_qdss_common",
        "MSM_TMC_ETFETB" : "parse_qdss_common",
        "MSM_TMC1_REG" : "parse_qdss_common",
        "MSM_ETM0_REG" : "parse_qdss_common",
        "MSM_ETM1_REG" : "parse_qdss_common",
        "MSM_ETM2_REG" : "parse_qdss_common",
        "MSM_ETM3_REG" : "parse_qdss_common",

}

tag_to_field_name = {
        "MSM_TMC0_REG" : "tmc_etr_start",
        "MSM_TMC_ETFETB" : "etf_start",
        "MSM_TMC1_REG" : "tmc_etf_start",
        "MSM_ETM0_REG" : "etm_regs0",
        "MSM_ETM1_REG" : "etm_regs1",
        "MSM_ETM2_REG" : "etm_regs2",
        "MSM_ETM3_REG" : "etm_regs3",
}

offsets_required_debug_image = [
        ("((struct msm_memory_dump *)0x0)", "dump_table_ptr", 0, 0),
        ("((struct msm_dump_table *)0x0)", "version", 0, 0),
        ("((struct msm_dump_table *)0x0)", "num_entries", 0, 0),
        ("((struct msm_dump_table *)0x0)", "client_entries", 0, 0),
        ("((struct msm_client_dump *)0x0)", "id", 0, 0),
        ("((struct msm_client_dump *)0x0)", "start_addr", 0, 0),
        ("((struct msm_client_dump *)0x0)", "end_addr", 0, 0),
        ("sizeof(struct msm_client_dump)","",0, 1),
]

class DebugImage() :
    def __init__ (self, ram_dump, unwind) :
        self.ram_dump = ram_dump
        ram_dump.qdss = QDSSDump()
        self.unwind = unwind
        self.name_lookup_table = []

    def parse_cpu_ctx (self, start, end, tag) :
        print_out_str ("Parsing CPU context start {0:x} end {1:x}".format(start, end))
        # For historical reasons, we can't rely on the magic number to indicate if there
        # is context dumped. Check the magic number here instead
        magic = self.ram_dump.read_word(start, False)
        if magic != 0x44434151 :
            print_out_str("!!! Magic {0:x} doesn't match! No context was dumped!".format(magic))
            return

        regs = get_regs(self.ram_dump, start)
        for i in range(regs.ncores) :
            regs.dump_core_pc(self.ram_dump,self.unwind, i)
        regs.dump_all_regs(self.ram_dump, self.ram_dump.outdir)

    def parse_l2_cache (self, start, end, tag) :
        print_out_str ("Parsing L2 cache context start {0:x} end {1:x}".format(start, end))
        magic = self.ram_dump.read_word(start, False)
        if magic != 0xcac1ecac :
            try :
                print_out_str ("!!! Magic {0:x} doesn't match! No cache was dumped!".format(magic))
            except ValueError:
                print_out_str ("!!! Magic {%s} doesn't match! No cache was dumped!" % magic)
            return

        parse_cache_dump(self.ram_dump, self.ram_dump.outdir, start)

    def parse_l1_cache (self, start, end, tag) :
        magic = self.ram_dump.read_word(start, False)
        if magic !=  0x314C4151 :
            try :
                print_out_str ("!!! Magic {0:X} doesn't match! No cache was dumped!".format(magic))
            except ValueError:
                print_out_str ("!!! Magic {%s} doesn't match! No cache was dumped!" % magic )
            return
        print_out_str ("Saving L1 cache")
        save_l1_dump(self.ram_dump, self.ram_dump.outdir, start, end - start)

    def parse_ocmem (self, start, end, tag) :
        print_out_str ("[!!!] Parsing not implemented yet start {0:x} end {1:x}".format(start, end))

    def parse_qdss_common(self, start, end, tag) :
        print_out_str ("Parsing {0} context start {1:x} end {2:x}".format(tag, start, end))
        magic = self.ram_dump.read_word(start, False)
        if magic != QDSS_MAGIC :
            print_out_str ("!!! Magic {0:X} doesn't match! Tracing was not dumped!".format(magic))
            return

        setattr(self.ram_dump.qdss, tag_to_field_name[tag], start + 4096)

    def load_numbers(self) :
        gdb_cmd = NamedTemporaryFile(mode='w+t', delete=False)
        gdb_out = NamedTemporaryFile(mode='w+t', delete=False)
        for i in range(0,32) :
            gdb_cmd.write("print ((enum dump_client_type){0})\n".format(i))
        gdb_cmd.flush()
        gdb_cmd.close()
        gdb_out.close()
        stream = os.system("{0} -x {1} --batch {2} > {3}".format(self.ram_dump.gdb_path, gdb_cmd.name, self.ram_dump.vmlinux, gdb_out.name))
        a = open(gdb_out.name)
        results = a.readlines()
        for r in results :
            s = r.split(' ')
            self.name_lookup_table.append(s[2].rstrip())
        a.close()
        os.remove(gdb_out.name)
        os.remove(gdb_cmd.name)

    def debug_image_parse(self) :
        if not self.ram_dump.is_config_defined("CONFIG_MSM_MEMORY_DUMP") :
            print_out_str ("!!! Debug image was not enabled. No debug dump will be provided")
            return

        out_dir = self.ram_dump.outdir
        self.load_numbers()
        self.ram_dump.setup_offset_table(offsets_required_debug_image)
        dump_table_ptr_offset = self.ram_dump.get_offset_struct("((struct msm_memory_dump *)0x0)", "dump_table_ptr")
        version_offset = self.ram_dump.get_offset_struct("((struct msm_dump_table *)0x0)", "version")
        num_entries_offset = self.ram_dump.get_offset_struct("((struct msm_dump_table *)0x0)", "num_entries")
        client_entries_offset = self.ram_dump.get_offset_struct("((struct msm_dump_table *)0x0)", "client_entries")
        id_offset = self.ram_dump.get_offset_struct("((struct msm_client_dump *)0x0)", "id")
        start_addr_offset = self.ram_dump.get_offset_struct("((struct msm_client_dump *)0x0)", "start_addr")
        end_addr_offset = self.ram_dump.get_offset_struct("((struct msm_client_dump *)0x0)", "end_addr")
        client_dump_entry_size = self.ram_dump.get_offset_struct("sizeof(struct msm_client_dump)","")

        mem_dump_data = self.ram_dump.addr_lookup("mem_dump_data")

        dump_table = self.ram_dump.read_word(mem_dump_data +  dump_table_ptr_offset)

        version = self.ram_dump.read_word(dump_table + version_offset)
        num_entries = self.ram_dump.read_word(dump_table + num_entries_offset)

        print_out_str("\nDebug image version: {0}.{1} Number of entries {2}".format(version >> 20, version & 0xFFFFF, num_entries))
        print_out_str("--------")

        for i in range(0,num_entries) :
            this_client = dump_table + client_entries_offset + i * client_dump_entry_size
            client_id = self.ram_dump.read_word(this_client + id_offset)
            client_start = self.ram_dump.read_word(this_client + start_addr_offset)
            client_end = self.ram_dump.read_word(this_client + end_addr_offset)

            if client_id < 0 or client_id > len(self.name_lookup_table) :
                print_out_str("!!! Invalid client id found {0:x}".format(client_id))
                continue

            client_name = self.name_lookup_table[client_id]

            if client_name not in print_table :
                print_out_str("!!! {0} Does not have an associated function. The parser needs to be updated!".format(client_name))
            else :
                print_out_str("Parsing debug information for {0}".format(client_name))
                func = print_table[client_name]
                getattr(DebugImage,func)(self, client_start, client_end, client_name)
            print_out_str("--------")
