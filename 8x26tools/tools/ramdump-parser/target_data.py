

#
# Target-specific pretty-printing
#

# This variable starts as a dictionary of targets.  When command line arguments
# are read, it is overwritten with only the specific dictionary for the target
# selected.
all_targets_data = {
    '8974' : {
        'masters' : {
            0   : '"APSS"',
            1   : '"MSS SW"',
            2   : '"LPASS"',
            3   : '"PRONTO"',
        },
        'sleep_modes' : {
            0   :  '"XO Shutdown"',
            1   :  '"VDD Minimization"',
        },
        'mpm_ints' : {
        },
        'plls' : {
            1  : 'XO',
            2  : 'GPLL0',
            3  : 'GPLL1',
            4  : 'GPLL2',
            5  : 'GPLL3',
            6  : 'MMPLL0',
            7  : 'MMPLL1',
            8  : 'MMPLL2',
            9  : 'MMPLL3',
        },
        'Voltages' : {
            0  : 'OFF         ',
            1  : 'RETENTION   ',
            3  : 'LOW         ',
            4  : 'NONIMAL     ',
            5  : 'NONIMAL_PLUS',
            6  : 'HIGH        ',
        },
        'icb_masters' : {
            38 : 'MASTER_BAM_DMA',
            21 : 'MASTER_BIMC_SNOC',
            41 : 'MASTER_BLSP_1',
            39 : 'MASTER_BLSP_2',
            22 : 'MASTER_CNOC_SNOC',
            18 : 'MASTER_LPASS_AHB',
            25 : 'MASTER_LPASS_PROC',
            54 : 'MASTER_OVIRT_SNOC',
            29 : 'MASTER_PNOC_SNOC',
            43 : 'MASTER_PNOC_CFG',
            33 : 'MASTER_SDCC_1',
            35 : 'MASTER_SDCC_2',
            34 : 'MASTER_SDCC_3',
            36 : 'MASTER_SDCC_4',
            3  : 'MASTER_SNOC_BIMC',
            52 : 'MASTER_SNOC_CNOC',
            44 : 'MASTER_SNOC_PNOC',
            37 : 'MASTER_TSIF',
            42 : 'MASTER_USB_HS',
            40 : 'MASTER_USB_HSIC',
        },
        'icb_slaves' : {
            1  : 'SLAVE_APPSS_L2',
            36 : 'SLAVE_BAM_DMA',
            2  : 'SLAVE_BIMC_SNOC',
            39 : 'SLAVE_BLSP_1',
            37 : 'SLAVE_BLSP_2',
            75 : 'SLAVE_CNOC_SNOC',
            0  : 'SLAVE_EBI1',
            55 : 'SLAVE_MESSAGE_RAM',
            26 : 'SLAVE_OCIMEM',
            18 : 'SLAVE_OCMEM',
            41 : 'SLAVE_PDM',
            45 : 'SLAVE_PNOC_SNOC',
            59 : 'SLAVE_PMIC_ARB',
            69 : 'SLAVE_PNOC_CFG',
            44 : 'SLAVE_PRNG',
            30 : 'SLAVE_QDSS_STM',
            74 : 'SLAVE_RPM',
            31 : 'SLAVE_SDCC_1',
            33 : 'SLAVE_SDCC_2',
            32 : 'SLAVE_SDCC_3',
            34 : 'SLAVE_SDCC_4',
            24 : 'SLAVE_SNOC_BIMC',
            25 : 'SLAVE_SNOC_CNOC',
            27 : 'SLAVE_SNOC_OVIRT',
            28 : 'SLAVE_SNOC_PNOC',
            51 : 'SLAVE_TLMM',
            35 : 'SLAVE_TSIF',
            40 : 'SLAVE_USB_HS',
            38 : 'SLAVE_USB_HSIC',
        },
    },
}

try:
	target_data
except NameError:
	target_data = None

def select_target(target_name):
	global target_data
	target_data = all_targets_data[target_name]
	
def get_resource_name(resource):
    out = ''
    for c in reversed([hex(resource)[i:i+2] for i in xrange(2,10,2)]):
        out = out + chr(int(c, 16))
    return out;

def get_master_name(master):
    return target_data['masters'].get(master, '"Unknown master %i"' % master)

def get_sleep_mode(mode):
    return target_data['sleep_modes'].get(mode, '"Unknown mode %i"' % mode)

def get_pll(pll):
    return target_data['plls'].get(pll, '"Unknown PLL %i"' % pll)

def get_Voltage(Voltage):
    return target_data['Voltages'].get(Voltage, '"Unknown voltage level %i"' % Voltage)


def get_interrupt_name(interrupt):
    rpm_interrupt_ids = {
        0   : '"SPM Shutdown Handshake"',
        1   : '"SPM Bringup Handshake"',
    }
    return rpm_interrupt_ids.get(interrupt, '"Unknown interrupt %i"' % interrupt)

def get_set_name(set):
    rpm_set_ids = {
        0   : '"Active Set"',
        1   : '"Sleep Set"',
    }
    return rpm_set_ids.get(set, '"Unknown set %i"' % set)

def decode_bitfield(name, bit_definitions, data):
    known_bits = 0
    for id in bit_definitions:
        known_bits |= (1 << id)
    unknown_data = data - (data & known_bits)
    string = ' | '.join(['[' + bit_definitions[x] + ']' for x in bit_definitions if (1 << x) & data])
    if unknown_data:
        if string:
            string += ' ...and '
        multi_or_single = ''
        if log(unknown_data, 2) != int(log(unknown_data, 2)):
            multi_or_single = 's'
        string += 'unknown %s%s 0x%0.8x' % (name, multi_or_single, unknown_data)
    return string

def get_action_names(actions):
    rpm_action_ids = {
        0   : 'Request',
        1   : 'Notification',
    }
    return decode_bitfield('action', rpm_action_ids, actions)

def get_interrupt_names(interrupts):
    return decode_bitfield('interrupt', target_data['mpm_ints'], interrupts)

def get_icb_master_name(iden):
    return target_data['icb_masters'].get(iden, '"Unknown master %i"' % iden)

def get_icb_slave_name(iden):
    return target_data['icb_slaves'].get(iden, '"Unknown slave %i"' % iden)
