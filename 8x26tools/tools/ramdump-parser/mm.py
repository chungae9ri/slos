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

def page_buddy(ramdump, page) :
    mapcount_offset = ramdump.get_offset_struct("((struct page *)0x0)","_mapcount")
    val = ramdump.read_word(page + mapcount_offset)
    # -128 is the magic for in the buddy allocator
    return val == 0xffffff80

def page_zonenum(page_flags) :
    # save this in a variable somewhere...
    return (page_flags >> 26) & 3

def page_to_nid(page_flags) :
    return 0

def page_zone(ramdump, page) :
    contig_page_data = ramdump.addr_lookup("contig_page_data")
    node_zones_offset = ramdump.get_offset_struct("((struct pglist_data *)0x0)", "node_zones")
    page_flags_offset = ramdump.get_offset_struct("((struct page *)0x0)", "flags")
    zone_size = ramdump.get_offset_struct("sizeof(struct zone)","")
    page_flags = ramdump.read_word(page + page_flags_offset)
    if page_flags is None :
        return None
    zone = contig_page_data + node_zones_offset + (page_zonenum(page_flags)*zone_size)
    return zone

def zone_is_highmem(ramdump, zone) :
    if zone is None :
        return False
    # not at all how linux does it but it works for our purposes...
    zone_name_offset = ramdump.get_offset_struct("((struct zone *)0x0)","name")
    zone_name_addr = ramdump.read_word(zone + zone_name_offset)
    if zone_name_addr is None :
        return False
    zone_name = ramdump.read_cstring(zone_name_addr, 48)
    if zone_name is None :
        # XXX do something?
        return False
    if zone_name == "HighMem" :
        return True
    else :
        return False

def hash32(val, bits) :
    chash = c_uint(val * 0x9e370001).value
    return chash >> (32 - bits)

def page_slot(ramdump, page) :
    hashed = hash32(page, 7)
    htable = ramdump.addr_lookup("page_address_htable")
    htable_size = ramdump.get_offset_struct("sizeof(page_address_htable[0])","")
    return htable + htable_size * hashed

def page_to_section(page_flags) :
    # again savefn8n variable
    return (page_flags >> 28) & 0xF

def nr_to_section(ramdump, sec_num) :
    memsection_struct_size = ramdump.get_offset_struct("sizeof(struct mem_section)","")
    sections_per_root = 4096 / memsection_struct_size
    sect_nr_to_root = sec_num / sections_per_root
    masked = sec_num & (sections_per_root - 1)
    mem_section_addr = ramdump.addr_lookup("mem_section")
    mem_section = ramdump.read_word(mem_section_addr)
    if mem_section is None :
        return None
    return mem_section + memsection_struct_size *(sect_nr_to_root * sections_per_root + masked)

def section_mem_map_addr(ramdump, section) :
    map_offset = ramdump.get_offset_struct("((struct mem_section *)0x0)","section_mem_map")
    result = ramdump.read_word(section + map_offset)
    return result & ~((1<<2)-1)

def pfn_to_section_nr(pfn) :
    return pfn >> (28-12)

def pfn_to_section(ramdump, pfn) :
    return nr_to_section(ramdump, pfn_to_section_nr(pfn))

def pfn_to_page_sparse(ramdump, pfn) :
    sec = pfn_to_section(ramdump, pfn)
    sizeof_page = ramdump.get_offset_struct("sizeof(struct page)", "")
    return section_mem_map_addr(ramdump, sec) + pfn * sizeof_page

def page_to_pfn_sparse(ramdump, page) :
    page_flags_offset = ramdump.get_offset_struct("((struct page *)0x0)", "flags")
    sizeof_page = ramdump.get_offset_struct("sizeof(struct page)", "")
    flags = ramdump.read_word(page + page_flags_offset)
    if flags is None :
        return 0
    section = page_to_section(flags)
    nr = nr_to_section(ramdump, section)
    addr = section_mem_map_addr(ramdump, nr)
    # divide by struct page size for division fun
    return (page - addr) / sizeof_page

def page_to_pfn_flat(ramdump, page) :
    mem_map_addr = ramdump.addr_lookup("mem_map")
    mem_map = ramdump.read_word(mem_map_addr)
    page_size = ramdump.get_offset_struct("sizeof(struct page)", "")
    # XXX Needs to change for LPAE
    pfn_offset = ramdump.phys_offset >> 12
    return ((page - mem_map)/page_size) + pfn_offset

def pfn_to_page_flat(ramdump, pfn) :
    mem_map_addr = ramdump.addr_lookup("mem_map")
    mem_map = ramdump.read_word(mem_map_addr)
    page_size = ramdump.get_offset_struct("sizeof(struct page)", "")
    # XXX Needs to change for LPAE
    pfn_offset = ramdump.phys_offset >> 12
    return mem_map + (pfn*page_size) - pfn_offset

def page_to_pfn(ramdump, page) :
    if ramdump.is_config_defined("CONFIG_SPARSEMEM") :
        return page_to_pfn_sparse(ramdump, page)
    else :
        return page_to_pfn_flat(ramdump, page)

def pfn_to_page(ramdump, pfn) :
    if ramdump.is_config_defined("CONFIG_SPARSEMEM") :
        return pfn_to_page_sparse(ramdump, pfn)
    else :
        return pfn_to_page_flat(ramdump, pfn)

def sparsemem_lowmem_page_address(ramdump, page) :
    membank1_start = ramdump.read_word(ramdump.addr_lookup("membank1_start"))
    membank0_size = ramdump.read_word(ramdump.addr_lookup("membank0_size"))
    # XXX currently magic
    membank0_phys_offset = ramdump.phys_offset
    membank0_page_offset = 0xc0000000
    membank1_phys_offset = membank1_start
    membank1_page_offset = membank0_page_offset + membank0_size
    phys = page_to_pfn(ramdump, page) << 12
    if phys >= membank1_start :
        return phys - membank1_phys_offset + membank1_page_offset
    else :
        return phys - membank0_phys_offset + membank0_page_offset

def dont_map_hole_lowmem_page_address(ramdump, page) :
    phys = page_to_pfn(ramdump, page) << 12
    hole_end_addr = ramdump.addr_lookup("memory_hole_end")
    hole_offset_addr = ramdump.addr_lookup("memory_hole_offset")
    hole_end = ramdump.read_word(hole_end_addr)
    hole_offset = ramdump.read_word(hole_offset_addr)
# LGE_CHANGE_S: prevent error if hole_end is None
#    if hole_end != 0 and phys >= hole_end :
    if hole_end != 0 and phys >= hole_end and hole_end != None:
# LGE_CHANGE_E
        return phys - hole_end + hole_offset + 0xc0000000
    else :
        return phys - ramdump.phys_offset + 0xc0000000

def lowmem_page_address(ramdump, page) :
    # keep this 8974 specific for now
    if ramdump.hw_id == 8974 :
        return dont_map_hole_lowmem_page_address(ramdump, page)
    else :
        return sparsemem_lowmem_page_address(ramdump, page)

def page_address(ramdump, page) :
    if not zone_is_highmem(ramdump, page_zone(ramdump, page)) :
        return lowmem_page_address(ramdump, page)

    pas = page_slot(ramdump, page)
    lh_offset = ramdump.get_offset_struct("((struct page_address_slot *)0x0)","lh")
    start = pas + lh_offset
    pam = start
    while True :
        pam = pam - lh_offset
        pam_page_offset = ramdump.get_offset_struct("((struct page_address_map *)0x0)","page")
        pam_virtual_offset = ramdump.get_offset_struct("((struct page_address_map *)0x0)","virtual")
        pam_page = ramdump.read_word(pam + pam_page_offset)
        if pam_page == page :
            ret = ramdump.read_word(pam + pam_virtual_offset)
            return ret
        pam = ramdump.read_word(pam + lh_offset)
        if pam == start :
            return None
