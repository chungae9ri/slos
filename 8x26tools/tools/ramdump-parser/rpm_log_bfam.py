# vim: set ts=4 sw=4 noexpandtab smarttab autoindent:
from struct import unpack
from optparse import OptionParser
from math import log
import sys, re, os
import clock_parser
import ddr_parser
import pmic_parser
import bus_parser
import rpm_parser
import base_parser
import ocmem_parser
import railway_parser
import rbcpr_parser
from target_data import *

#
# NPA-related pretty-printing
#
npa_client_names     = {}
npa_client_resources = {}
npa_resources        = {}

def npa_lookup(dictionary, handle):
    if handle in dictionary:
        return dictionary[handle]
    else:
        formatted_handle = '0x%0.8x' % handle
        rpmlog_out.write('  ***** WARNING ***** failed to match npa handle %s\n' % formatted_handle)
        return formatted_handle

def npa_not_loaded(handle):
    global npa_client_name_lookup
    global npa_client_resource_lookup
    global npa_resource_lookup

    rpmlog_out.write( '  ***** WARNING ***** no NPA dump loaded.  All NPA name resolutions will fail.\n')
    rpmlog_out.write( '  ***** WARNING ***** switching to silent NPA lookup failure mode.\n')
    npa_client_name_lookup     = lambda h: '0x%0.8x' % h
    npa_client_resource_lookup = lambda h: '0x%0.8x' % h
    npa_resource_lookup        = lambda h: '0x%0.8x' % h
    return '0x%0.8x' % handle

npa_client_name_lookup     = npa_not_loaded
npa_client_resource_lookup = npa_not_loaded
npa_resource_lookup        = npa_not_loaded

npa_client_regex   = re.compile(r'[^:]*:\s+npa_client\s+\(name: (?P<name>[^\)]*)\)\s+\(handle: (?P<handle>[^\)]+)\)\s+\(resource: (?P<resource>[^\)]+)\)')
npa_resource_regex = re.compile(r'[^:]*:\s+npa_resource\s+\(name: "(?P<name>[^"]+)"\)\s+\(handle: (?P<handle>[^\)]+)')

def parse_npa_dump(npa_dump_filename):
    global npa_client_name_lookup
    global npa_client_resource_lookup
    global npa_resource_lookup

    npa_dump = open(npa_dump_filename, 'r')
    for line in npa_dump.readlines():
        m = npa_client_regex.match(line)
        if m:
            npa_client_names[int(m.group('handle'), 16)] = m.group('name')
            npa_client_resources[int(m.group('handle'), 16)] = int(m.group('resource'), 16)
            continue
        m = npa_resource_regex.match(line)
        if m:
            npa_resources[int(m.group('handle'), 16)] = m.group('name')
            continue
    npa_dump.close()

    npa_client_name_lookup     = lambda h: npa_lookup(npa_client_names, h)
    npa_resource_lookup        = lambda h: npa_lookup(npa_resources, h)
    def client_resource_lookup(h):
        resource = npa_lookup(npa_client_resources, h)
        lookup_failed = isinstance(resource, str)
        return '<lookup failed>' if lookup_failed else npa_resource_lookup(resource)
    npa_client_resource_lookup = client_resource_lookup

#
# The parsing core
#

def rpm_log_bfam(ramdump, target, filename, npa_filename = None, raw_timestamp = False, ) :
    if not filename:
        sys.exit(1)

    rpmlog_out = open("{0}/rpm.log".format(ramdump.outdir), "w")
    if npa_filename:
        parse_npa_dump(npa_filename)

    if target not in all_targets_data:
        rpmlog_out.write('Error: unknown target %s' % target)
        sys.exit(1)
    select_target(target)

    # Try to load data from the log file.
    try:
        f = open(filename)
        loglines = f.readlines()
        f.close()
    except:
        rpmlog_out.write('Error loading log data from file: %s\n' % sys.exc_info()[0])
        raise

    # Got data, parse it as well as we can.
    for line in loglines:
        # First try to unpack it into its components.
        try:
            assert '- ' == line[0:2]
            bytestring = ''.join(map(lambda x: chr(int(x, 16)), line[2:line.rfind(',')].split(', ')))
            message = list(unpack('<%iL' % (len(bytestring)/4), bytestring))

            timestamp = (message[1] << 32) | message[0]
            id = message[2]
            data = message[3:]
        except:
            rpmlog_out.write('Error parsing message from logfile: %s\n' % (sys.exc_info()[0]))

        # Then try to find a parser for it.
        if raw_timestamp:
            timestamp = '0x%0.16x' % (timestamp)
        else:
            timestamp = '%f' % (timestamp * (1 / 19200000.0))

        try:
            pretty_message = base_parser.parsers[id]().parse(data)
            rpmlog_out.write("%s: %s\n" % (timestamp, pretty_message))
        except:
            rpmlog_out.write('Error parsing log message with timestamp = %s, id = %i, and data = %s -- %s\n' % (timestamp, id, data, sys.exc_info()[0]))
    rpmlog_out.close()

