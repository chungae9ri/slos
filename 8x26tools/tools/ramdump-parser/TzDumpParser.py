import sys
import re
import os
import struct
import datetime
import array
import string
import bisect
import traceback
import StringIO
from subprocess import *
from optparse import OptionParser
from optparse import OptionGroup
from struct import unpack
from ctypes import *


#========================================================
# All Macros,
# ToDo - TZBSP_DIAG_AREA, SHARED_IMEM_TZ_BASE_OFF to set
#        as per TZ Build file
#========================================================
SHARED_IMEM_TZ_BASE_OFF = 0x0005f720      # Used when IMEM dump is taken in 1 file
SHARED_IMEM_TZ_BASE_OFF_2 = 0x00000720    # Used when IMEM is divided into 2 part, 1 for TZ & other for SMEM
MSGRAM_TZ_BASE_OFF = 0x00003720
TZBSP_DIAG_NUM_OF_VMID = 16
TZBSP_DIAG_RING_LEN = 2584
TZBSP_DIAG_VMID_DESC_LEN = 7
TZBSP_MAX_INT_DESC = 16

#========================================================
# Supported chipsets
#========================================================
a_family_chipsets = ('msm8960', 'msm8930', 'apq8064')
b_family_chipsets = ('MSM8974', 'msm8974', 'mdm9x25')

#========================================================
# TZBSP Diag structures
#========================================================
tzbsp_diag_vmid_s = "".join([
  "B",        # vmid
  "B" * TZBSP_DIAG_VMID_DESC_LEN,     # vmid desc
  ])

tzbsp_diag_boot_info_s = "".join([
  "I",      # /* Warmboot entry CPU Counter */ uint32 warm_entry_cnt;

  "I",      # /* Warmboot exit CPU Counter */  uint32 warm_exit_cnt;

  "I",      # /* Power collapse termination entry counter. */ uint32 term_entry_cnt;

  "I",      # /* Power collapse termination exit counter. The number of times CPU fell
            #  * through the WFI without entering power collapse. */ uint32 term_exit_cnt;

  "I",      # /* Last Warmboot Jump Address */ uint32 warm_jmp_addr;

  "I"       # /* Last Warmboot Jump Address Instruction */ uint32 warm_jmp_instr;
  ])

tzbsp_diag_reset_info_s = "".join([
  "I",      # /* Reset Reason - Security Violation */  uint32 reset_type;

  "I",      # /* Number of resets that occured for above CPU */ uint32 reset_cnt;
  ])

tzbsp_diag_int_s = "".join([
  "H",      # /* Type of Interrupt/exception */ uint16 int_info;

  "B",      # /* Availability of the slot */  uint8 avail;

  "B",      # /* Reserved for future use. */ uint8 spare;

  "I",      # /* Interrupt Number for IRQ and FIQ interrupts */ uint32 int_num;

  "B" * TZBSP_MAX_INT_DESC,     # /* uint8 int_desc[TZBSP_MAX_INT_DESC]; */

#  "Q" * TZBSP_CPU_COUNT         # /* Number of this interrupt seen per CPU. */ uint64 int_count[TZBSP_CPU_COUNT];

  ])

tzbsp_diag_s ="".join ([
  "I",      # Magic Number  uint32 magic_num;

  "I",      # Major.Minor version uint32 version;

  "I",      # Number of CPU's uint32 cpu_count;

  "I",      # Offset of VMID Table uint32 vmid_info_off;

  "I",      # Offset of Boot Table uint32 boot_info_off;

  "I",      # Offset of Reset Table uint32 reset_info_off;

  "I",      # Offset of Interrupt Table uint32 int_info_off;

  "I",      # Ring Buffer Offset uint32 ring_off;

  "I",      # Ring Buffer Len uint32 ring_len;

# tzbsp_diag_vmid_s * TZBSP_DIAG_NUM_OF_VMID,   # /* VMID to EE Mapping. */  tzbsp_diag_vmid_t vmid[TZBSP_DIAG_NUM_OF_VMID];

#  tzbsp_diag_boot_info_s * TZBSP_CPU_COUNT,     # /* Boot Info */ boot_info[TZBSP_CPU_COUNT];

#  tzbsp_diag_reset_info_s * TZBSP_CPU_COUNT,    # /* Reset Info */ tzbsp_diag_reset_info_t reset_info[TZBSP_CPU_COUNT];

#  "I",      # Length of the interrupt table uint32 num_interrupts;

#  tzbsp_diag_int_s * TZBSP_DIAG_INT_NUM,         # tzbsp_diag_int_t  int_info[TZBSP_DIAG_INT_NUM];

#  "B" *  TZBSP_DIAG_RING_LEN,    # ring buffer

  ])

#========================================================
# Create the error table
#========================================================
Tzbsp_Error_table = {

    #TZBSP Error Codes
"(1":"TZBSP_EC_SUBSYS_BRING_UP_INVALID_SUBSYS",
"(2":"TZBSP_EC_SUBSYS_BRING_UP_FAILED",
"(3":"TZBSP_EC_SUBSYS_TEAR_DOWN_INVALID_SUBSYS",
"(4":"TZBSP_EC_SUBSYS_TEAR_DOWN_FAILED",
"(5":"TZBSP_EC_SUBSYS_IS_SUPPORTED_INVALID_SUBSYS",
"(6":"TZBSP_EC_SUBSYS_IS_MANDATED_INVALID_SUBSYS",
"(7":"TZBSP_EC_IS_BIND_IMAGE_INVALID_SUBSYS",
"(8":"TZBSP_EC_GET_MMAP_AREAS_INVALID_SUBSYS",
"(9":"TZBSP_EC_SUBSYS_IS_VALID_AREA_INVALID_SUBSYS",
"(10":"TZBSP_EC_SUBSYS_IS_VALID_AREA_INVALID_SEGMENT",
"(11":"TZBSP_EC_SUBSYS_LOCK_INVALID_SUBSYS",
"(12":"TZBSP_EC_SUBSYS_LOCK_INVALID_BLIST",
"(13":"TZBSP_EC_SUBSYS_LOCK_XPU_LOCK_AREA_FAILED",
"(14":"TZBSP_EC_SUBSYS_LOCK_CONFIG_S_AREA_FAILED",
"(15":"TZBSP_EC_SUBSYS_LOCK_ENABLE_S_AREA_FAILED",
"(16":"TZBSP_EC_SUBSYS_UNLOCK_INVALID_SUBSYS",
"(17":"TZBSP_EC_SUBSYS_UNLOCK_XPU_UNLOCK_FAILED",
"(18":"TZBSP_EC_SUBSYS_UNLOCK_DISABLE_S_AREA_FAILED",
"(19":"TZBSP_EC_PIL_MEM_AREA_UNSUPPORTED_PROC",
"(20":"TZBSP_EC_PIL_MEM_AREA_NULL_START_ADDR",
"(21":"TZBSP_EC_PIL_MEM_AREA_ZERO_IMAGE_LEN",
"(22":"TZBSP_EC_PIL_MEM_AREA_NULL_PROG_HDR",
"(23":"TZBSP_EC_PIL_MEM_AREA_INVALID_IMAGE_AREA",
"(24":"TZBSP_EC_PIL_UNLOCK_AREA_USUPPORTED_PROC",
"(25":"TZBSP_EC_PIL_UNLOCK_AREA_TEAR_DOWN_FAILED",
"(26":"TZBSP_EC_PIL_UNLOCK_AREA_UNLOCK_XPU_FAILED",
"(27":"TZBSP_EC_PIL_UNLOCK_AREA_UNLOCK_XPU_ZERO_FAILED",
"(28":"TZBSP_EC_PIL_UNLOCK_AREA_UNLOCK_XPU_FAILED2",
"(29":"TZBSP_EC_PIL_INIT_IMAGE_NULL_ELF_HDR",
"(30":"TZBSP_EC_PIL_INIT_IMAGE_UNSUPPORTED_PROC",
"(31":"TZBSP_EC_PIL_INIT_IMAGE_ELF_HDR_NOT_NONSECURE",
"(32":"TZBSP_EC_PIL_INIT_IMAGE_IS_NOT_ELF",
"(33":"TZBSP_EC_PIL_INIT_IMAGE_POPULATE_ELF_INFO_FAILED",
"(34":"TZBSP_EC_PIL_INIT_IMAGE_VERIFY_SIG_FAILED",
"(35":"TZBSP_EC_POPULATE_ELF_NO_PROG_HDR",
"(36":"TZBSP_EC_POPULATE_ELF_HASH_SEG_TOO_SMALL",
"(37":"TZBSP_EC_POPULATE_ELF_ELF_HDR_NOT_IN_NS_MEMORY",
"(38":"TZBSP_EC_POPULATE_ELF_PROG_HDR_NOT_IN_NS_MEMORY",
"(39":"TZBSP_EC_POPULATE_ELF_HASH_SEG_NOT_IN_NS_MEMORY",
"(40":"TZBSP_EC_POPULATE_ELF_IMAGE_HDR_TOO_SMALL",
"(41":"TZBSP_EC_POPULATE_ELF_MALLOC_FAILED",
"(42":"TZBSP_EC_POPULATE_ELF_MI_BOOT_INVALID",
"(43":"TZBSP_EC_PIL_AUTH_RESET_UNSUPPORTED_PROC",
"(44":"TZBSP_EC_PIL_AUTH_RESET_PROC_NOT_IN_RESET",
"(45":"TZBSP_EC_PIL_AUTH_RESET_DECRYPT_FAILED",
"(46":"TZBSP_EC_PIL_AUTH_RESET_AUTH_SEGMENTS_FAILED",
"(47":"TZBSP_EC_PIL_IS_ELF_MAGIC_FAILED",
"(48":"TZBSP_EC_PIL_IS_ELF_INVALID_CLASS",
"(49":"TZBSP_EC_PIL_IS_ELF_INVALID_VERSION",
"(50":"TZBSP_EC_PIL_IS_ELF_INVALID_EHSIZE",
"(51":"TZBSP_EC_PIL_IS_ELF_INVALID_PHENTSIZE",
"(52":"TZBSP_EC_MI_BOOT_VALID_MI_NULL",
"(53":"TZBSP_EC_MI_BOOT_VALID_ZERO_SIZE",
"(54":"TZBSP_EC_MI_BOOT_VALID_LENGTH_MISMATCH",
"(55":"TZBSP_EC_MI_BOOT_VALID_UNSUPPORTED_PROC",
"(56":"TZBSP_EC_IS_SUBSYSTEM_SUPPORTED_VERDICT_SIZE",
"(57":"TZBSP_EC_IS_SUBSYSTEM_SUPPORTED_VERDICT_NOT_IN_NS_MEMORY",
"(58":"TZBSP_EC_IS_SUBSYSTEM_MANDATED_VERDICT_SIZE",
"(59":"TZBSP_EC_IS_SUBSYSTEM_MANDATED_VERDICT_NOT_IN_NS_MEMORY",
"(60":"TZBSP_EC_AUTH_ELF_HEADERS_ELF_IS_NULL",
"(61":"TZBSP_EC_AUTH_ELF_HEADERS_PROG_HDR_NUM_ZERO",
"(62":"TZBSP_EC_AUTH_ELF_HEADERS_HASH_FAILED",
"(63":"TZBSP_EC_AUTH_ELF_HEADERS_NULL_HASH",
"(64":"TZBSP_EC_AUTH_ELF_HEADERS_INVALID_DIGEST",
"(65":"TZBSP_EC_PIL_ZERO_SEGMENTS_SIZE_MISMATCH",
"(66":"TZBSP_EC_AUTH_SEGMENT_ZERO_ARGS",
"(67":"TZBSP_EC_AUTH_SEGMENT_INVALID_HASH_LEN",
"(68":"TZBSP_EC_AUTH_SEGMENT_HASH_FAILED",
"(69":"TZBSP_EC_AUTH_SEGMENT_INVALID_DIGEST",
"(70":"TZBSP_EC_PIL_AUTH_SEGMENTS_NULL_ELF",
"(71":"TZBSP_EC_PIL_AUTH_SEGMENTS_ZERO_SEGMENTS_FAILED",
"(72":"TZBSP_EC_PIL_AUTH_SEGMENTS_HASH_NULL",
"(73":"TZBSP_EC_PIL_AUTH_SEGMENTS_LOCK_XPU_FAILED",
"(74":"TZBSP_EC_PIL_AUTH_SEGMENTS_INVALID_HASH_LEN",
"(75":"TZBSP_EC_PIL_AUTH_SEGMENTS_SEGMENT_AUTH_FAILED",
"(76":"TZBSP_EC_PIL_AUTH_SEGMENTS_REMAINING_HASH_BYTES",
"(77":"TZBSP_EC_PIL_VERIFY_SIG_NULL_ELF",
"(78":"TZBSP_EC_PIL_VERIFY_SIG_ZERO_ARGS",
"(79":"TZBSP_EC_PIL_VERIFY_SIG_ROT_NOT_FOUND",
"(80":"TZBSP_EC_PIL_VERIFY_SIG_MPU_LOCK_FAILED",
"(81":"TZBSP_EC_PIL_VERIFY_SIG_NO_ATTESTATION_CERT",
"(82":"TZBSP_EC_PIL_VERIFY_SIG_BIND_IMAGE_FAILED",
"(83":"TZBSP_EC_PIL_VERIFY_SIG_INVALID_SIG_SZ",
"(84":"TZBSP_EC_PIL_VERIFY_SIG_MALLOC_FAILED",
"(85":"TZBSP_EC_PIL_VERIFY_SIG_INVALID_SIGNATURE",
"(86":"TZBSP_EC_PIL_VERIFY_SIG_INVALID_ELF_HEADERS",
"(87":"TZBSP_EC_VALIDATE_IMAGE_AREA_INVALID_ENTRY",
"(88":"TZBSP_EC_VALIDATE_IMAGE_AREA_INVALID_SEGMENT",
"(89":"TZBSP_EC_PIL_LOCK_XPU_IMAGE_AREA_INVALID",
"(90":"TZBSP_EC_PIL_LOCK_XPU_SUBSYS_LOCK_FAILED",
"(91":"TZBSP_EC_PIL_UNLOCK_XPU_TEAR_DOWN_FAILED",
"(92":"TZBSP_EC_PIL_UNLOCK_XPU_UNLOCK_FAILED",
"(93":"TZBSP_EC_PIL_UNLOCK_ZERO_XPU_TEAR_DOWN_FAILED",
"(94":"TZBSP_EC_PIL_UNLOCK_ZERO_XPU_UNLOCK_FAILED",
"(95":"TZBSP_EC_PIL_INIT_IMAGE_SSD_PARSE_MD_FAILED",
"(96":"TZBSP_EC_PIL_AUTH_RESET_SUBSYS_BRING_UP_FAILED",
"(97":"TZBSP_EC_PIL_ROLLBACK_VERIFY_VERSION",
"(98":"TZBSP_EC_VIDEO_TEAR_DOWN_SMMU_DEINIT_FAIL",
"(99":"TZBSP_EC_VIDEO_TEAR_DOWN_SMMU_CONFIG_FAIL",
"(100":"TZBSP_EC_VIDEO_BRING_UP_SMMU_CONFIG_FAIL",
"(101":"TZBSP_EC_VIDEO_BRING_UP_SMMU_INIT_FAIL",
"(102":"TZBSP_EC_SMMU_CONFIG2_ENABLE_FAIL",
"(103":"TZBSP_EC_SMR_CPDATA_INIT_FAIL",
"(104":"TZBSP_EC_SMMU_CONFIG_ENABLE_FAIL",
"(105":"TZBSP_EC_SMR_SMEM_INIT_FAIL",
"(106":"TZBSP_EC_SMMU_CONFIG2_RESTORE_FAIL",
"(107":"TZBSP_EC_SMMU_CONFIG_RESTORE_FAIL",
"(108":"TZBSP_EC_SMR_CPDATA_DEINIT_FAIL",
"(109":"TZBSP_EC_SMR_SMEM_DEINIT_FAIL",
"(110":"TZBSP_EC_TZBSP_CONFIG_S_AREA_FAIL",
"(111":"TZBSP_EC_TZBSP_ENABLE_S_AREA_FAIL",
"(112":"TZBSP_EC_TZBSP_DISABLE_S_AREA_FAIL",
"(113":"TZBSP_EC_CPDATA_TZBSP_VIDEO_INIT_FAILED",
"(114":"TZBSP_EC_SMEM_TZBSP_VIDEO_INIT_FAILED",
"(115":"TZBSP_EC_SMR_TZBSP_VIDEO_DEINIT_FAILED",
"(116":"TZBSP_EC_PIL_INIT_SEC_APP_ELF_HDR_NOT_SECURE",
"(117":"TZBSP_EC_INIT_MMU_INIT_FAILED",
"(118":"TZBSP_EC_INIT_DEVICE_UNMAP_FAILED",
"(119":"TZBSP_EC_INIT_DDR_INIT_FAILED",
"(120":"TZBSP_EC_INIT_HW_INIT_FAILED",
"(121":"TZBSP_EC_INIT_CHIPSET_INIT_FAILED",
"(122":"TZBSP_EC_INIT_COLD_INIT_HANDLER_FAILED",
"(123":"TZBSP_EC_INIT_PIL_INIT_HANDLER_FAILED",
"(124":"TZBSP_EC_INIT_BOOT_TAMPER_CHECK_FAILED",
"(125":"TZBSP_EC_SUBSYS_INVALID_PROC_SW_TYPE",
"(126":"TZBSP_EC_XPU_STATIC_CFG_FAIL",
"(127":"TZBSP_EC_VMIDMT_CFG_FAIL",
"(128":"TZBSP_EC_PMIC_INIT_FAIL",
"(129":"TZBSP_EC_DEHR_CFG_FAIL",
"(130":"TZBSP_EC_BAM_INIT_FAIL",
"(131":"TZBSP_EC_SMMU_STATIC_INIT_FAIL",
"(132":"TZBSP_EC_HW_CRYPTO_INIT_FAIL",
"(133":"TZBSP_EC_PIL_INIT_IMAGE_SYBSYS_IMAGE_INIT_OK_FAILED",
"(134":"TZBSP_EC_INIT_SECURE_CHANNEL_KEY_INIT_FAILED",
"(135":"TZBSP_EC_INIT_RESET_EXEC_FAILED",
"(136":"TZBSP_EC_STACK_CHECK_FAILED",
"(137":"TZBSP_EC_RESTORE_LPASS_VMIDMT_FAILED",
"(138":"TZBSP_EC_RESTORE_LPASS_BAMCFG_FAILED",

    #QSEE Error codes
"{1":"QSEE_RESULT_INCOMPLETE",
"{FFFFFFFF":"QSEE_RESULT_FAILURE",
"{FFFFFFFE":"QSEE_RESULT_FAIL_SET_RSP_NO_THREAD ",
"{FFFFFFFD":"QSEE_RESULT_FAIL_PROTECT_APP_RGN ",
"{FFFFFFFC":"QSEE_RESULT_FAIL_SYSCALL_NOT_ALLOWED ",
"{FFFFFFFB":"QSEE_RESULT_FAIL_REQUEST_NOT_NS ",
"{FFFFFFFA":"QSEE_RESULT_FAIL_RESPONSE_NOT_NS ",
"{FFFFFFF9":"QSEE_RESULT_FAIL_REQRSP_NOT_ALIGNED ",
"{FFFFFFF8":"QSEE_RESULT_FAIL_PROTECT_REQRSP_RGN ",
"{FFFFFFF7":"QSEE_RESULT_FAIL_VALIDATE_SEG_OUTSIDE_APP_RGN ",
"{FFFFFFF6":"QSEE_RESULT_FAIL_VALIDATE_SEG_OVERLAP_APP_RGN ",
"{FFFFFFF5":"QSEE_RESULT_FAIL_LOAD_APP_EXISTS ",
"{FFFFFFF4":"QSEE_RESULT_FAIL_PIL_INIT ",
"{FFFFFFF3":"QSEE_RESULT_FAIL_PIL_AUTH_RESET ",
"{FFFFFFF2":"QSEE_RESULT_FAIL_APP_START ",
"{FFFFFFF1":"QSEE_RESULT_FAIL_REG_LISTENER_ALIGNMENT ",
"{FFFFFFF0":"QSEE_RESULT_FAIL_REG_LISTENER_FULL ",
"{FFFFFFEF":"QSEE_RESULT_FAIL_PROTECT_REQRSP_RGN_OCCUPIED ",
"{FFFFFFEE":"QSEE_RESULT_FAIL_PROTECT_REQRSP_RGN_NO_THREAD_FOR_APP ",
"{FFFFFFED":"QSEE_RESULT_FAIL_SEND_CMD_NO_THREAD ",
"{FFFFFFEC":"QSEE_RESULT_FAIL_NO_HASH_SEG ",
"{FFFFFFEB":"QSEE_RESULT_FAIL_MALLOC_FAILED_FOR_PIL_BUF ",
"{FFFFFFEA":"QSEE_RESULT_FAIL_WORD_OVERFLOW ",
"{FFFFFFE9":"QSEE_ERROR_PRINT_APP_FAULTED ",
"{FFFFFFE8":"QSEE_ERROR_PRINT_SET_APP_FAULTED ",
"{FFFFFFE7":"QSEE_ERROR_PRINT_DEREG_LISTENER ",
"{FFFFFFE6":"QSEE_ERROR_PRINT_TOO_MANY_SB ",
"{FFFFFFE5":"QSEE_ERROR_PRINT_MAP_RGN_FAIL ",
"{FFFFFFE4":"QSEE_ERROR_PRINT_SHUTDOWN_APP_BLOCKED_ON_LISTENER ",
"{FFFFFFE3":"QSEE_ERROR_PRINT_NULL_APP_ID ",
"{FFFFFFE2":"QSEE_ERROR_PRINT_NO_THREAD_FOR_APP ",
"{FFFFFFE1":"QSEE_ERROR_PRINT_PROTECT_REQRSP_FAIL ",
}

Error_Fatal_table = [
"TZBSP_ERR_FATAL_NONE",
"TZBSP_ERR_FATAL_NON_SECURE_WDT",
"TZBSP_ERR_FATAL_SECURE_WDT",
"TZBSP_ERR_FATAL_AHB_TIMEOUT",
"TZBSP_ERR_FATAL_RPM_WDOG",
"TZBSP_ERR_FATAL_RPM_ERR",
"TZBSP_ERR_FATAL_NOC_ERROR"
]

#========================================================
# Function Definition's
#========================================================

# String printing into out file
def print_out_str_tz(out_file, in_string) :
  if out_file is None :
    print (in_string)
  else :
    filtered_string = filter(lambda x: x in string.printable, in_string)
    if filtered_string not in ('\r','\n'):
      out_file.write(filtered_string + "\n")
  return

# Converting ascii byte in a tuple to a printable string
def printable_string (Tup):
  strvar = ""
  for i in Tup :
    if i:
      strvar += ("%c" % i)
  return strvar

# map the error code into TZBSP or QSEE error code.
# assumed TZBSP Error Log format = (Error_Code)
# QSEE Error log format = {Error_Code, ...}
def map_to_error (line):
  in_line = str(line)
  Error_val = re.search("^[( }]+[0123456789]+",in_line, re.I)
  if Error_val:
#return Tzbsp_Error_table[(Error_val.group())[0:]]
    if (Error_val.group())[0:] in Tzbsp_Error_table :
      return Tzbsp_Error_table[(Error_val.group())[0:]]
    else :
      return "Unknown Error Type Error_val %s" % (Error_val.group())[0:]

def print_usage_and_exit():
  print '\nusage : TzDumpParser.py chipset shared_mem_bin [dump_file_bin] output'
  print '\nA-family \'chipset\': if \'shared_mem_bin\' contains the entire IMEM'
  print '\tcontents, the optional \'dump_file_bin\' can be ignored.  Otherwise,'
  print '\t\'shared_mem_bin\' is the Shared IMEM region and \'dump_file_bin\' is'
  print '\tthe remainder of IMEM.'
  print 'B-family \'chipset\': \'shared_mem_bin\' is the path/to/MSGRAM.bin'
  print '\tand \'dump_file_bin\' is the path/to/DDRCS0.bin'
  print '\nEx Usage : TzDumpParser.py msm8960 .\IMEM.bin TzDump.txt'
  print 'Ex Usage : TzDumpParser.py msm8960 .\IMEM_C.bin .\IMEM_A.bin TzDump.txt'
  print 'Ex Usage : TzDumpParser.py msm8974 .\MSGRAM.bin .\DDRCS0.bin TzDump.txt'
  sys.exit(1)

# main funtion
def TzDumpParser(ramdump, file_path) :
# Open the Shared Mem file to determine the location of the TZ dump, and prepare
# the dump file for read as well
  try:
    if ramdump.tz_start == 0xFE800000 :
      ShMemDumpFile = open("{0}/OCIMEM.BIN".format(file_path), "rb")
      ShMemDumpFile.seek(0x5720,0)
      #LogDumpFile = open("{0}/OCIMEM.BIN".format(file_path), "rb")
      LogDumpFile = open("{0}/DDRCS0.BIN".format(file_path), "rb")
      TZDiagAddr = (struct.unpack ('<I',ShMemDumpFile.read(4)))[0] & 0x0FFFFFFF
    else :
      ShMemDumpFile = open("{0}/MSGRAM.BIN".format(file_path), "rb")
      ShMemDumpFile.seek(0x3720,0)
      LogDumpFile = open("{0}/DDRCS0.BIN".format(file_path), "rb")
      TZDiagAddr = (struct.unpack ('<I',ShMemDumpFile.read(4)))[0]
    TzOutPutFile = open("{0}/tzdump.txt".format(ramdump.outdir), "wb")
  except IOError, e:
    print '\n%s' % (e)
    sys.exit(1)
 
  # Find TZ Diag Area Offset from SMEM Area .
  if (TZDiagAddr & 0xFFF): # memory offset not aligned one
    print " SHARED IMEM TZ Not Pointing to TZ Log or not aligned to 4K! "
    print_out_str_tz(TzOutPutFile, " SHARED IMEM TZ Not Pointing to TZ Log! ")
    sys.exit(0)

  while True:
#try:
      # read the TZDiag area and check for magic no. tzda.
      LogDumpFile.seek(TZDiagAddr,0)
      g_tzbsp_diag = struct.unpack(tzbsp_diag_s,LogDumpFile.read(36))  # reading offset variables from dump
      if (g_tzbsp_diag[0] != 0x747A6461):  #0x747A6461 == tzda
        print " TZBSP Log Not available or corrupted! "
        print_out_str_tz(TzOutPutFile, " TZBSP Log Not available or corrupted! ")
        sys.exit(0)

      # -----------------------------------------------
      # Prepare the Log File Now
      # ----------------------------------------------
      print_out_str_tz(TzOutPutFile, "===================================================== ")
      print_out_str_tz(TzOutPutFile, "Extracted TZBSP Log ")
      print_out_str_tz(TzOutPutFile, "===================================================== \n")

      # print the No of CPU's
      print_out_str_tz(TzOutPutFile, ("CPU Count -> %d " % g_tzbsp_diag[2]))

      # print the VMID's
      print_out_str_tz(TzOutPutFile, "\n-------- VMID Status --------- ")
      print_out_str_tz(TzOutPutFile, "VMID : VMID Desc")
      LogDumpFile.seek(TZDiagAddr + g_tzbsp_diag[3],0)   # g_tzbsp_diag[3] == VMID Tab Offset
      for i in range (0, TZBSP_DIAG_NUM_OF_VMID) :
        g_tzbsp_diag_vmid = struct.unpack(tzbsp_diag_vmid_s,LogDumpFile.read(struct.calcsize(tzbsp_diag_vmid_s)))
        if g_tzbsp_diag_vmid[0] != 0xff:
          print_out_str_tz(TzOutPutFile, ("%4x : %s " % (g_tzbsp_diag_vmid[0], printable_string (g_tzbsp_diag_vmid[1:7]))))

      # print the Boot status
      print_out_str_tz(TzOutPutFile, "\n---------------------------------- Boot Status -------------------------- ")
      print_out_str_tz(TzOutPutFile, "CPU : WarmEntry  : WarmExit   : PCEntry   : PCExit    : Warm JumpAddr: JumpInstr ")
      LogDumpFile.seek(TZDiagAddr + g_tzbsp_diag[4],0)   # g_tzbsp_diag[4] == Boot Table Off
      for i in range(0,g_tzbsp_diag[2]) :                 # g_tzbsp_diag[2] == CPU Count
        g_tzbsp_diag_boot_info = struct.unpack(tzbsp_diag_boot_info_s,LogDumpFile.read(struct.calcsize(tzbsp_diag_boot_info_s)))
        print_out_str_tz(TzOutPutFile, ("%3x : 0x%08x : 0x%08x : 0x%08x: 0x%08x: 0x%08x   : 0x%08x  " % (i,
                                    g_tzbsp_diag_boot_info[0], g_tzbsp_diag_boot_info[1], g_tzbsp_diag_boot_info[2],
                                    g_tzbsp_diag_boot_info[3], g_tzbsp_diag_boot_info[4], g_tzbsp_diag_boot_info[5],)))

      # print the Reset Table
      print_out_str_tz(TzOutPutFile, "\n-------- Reset Status --------- ")
      print_out_str_tz(TzOutPutFile, "CPU : Reset Reason/Type : Reset Count ")
      LogDumpFile.seek(TZDiagAddr + g_tzbsp_diag[5],0) # g_tzbsp_diag[5] == Reset reason Table Offset
      for i in range (0, g_tzbsp_diag[2]) :  # g_tzbsp_diag[2] == CPU Count
        g_diag_reset_info = struct.unpack(tzbsp_diag_reset_info_s,LogDumpFile.read(struct.calcsize(tzbsp_diag_reset_info_s)))
        try:
          print_out_str_tz(TzOutPutFile, ("%3x : 0x%08x (%s) : 0x%08x" % (i, g_diag_reset_info[0], Error_Fatal_table[g_diag_reset_info[0]].ljust(30), g_diag_reset_info[1])))
        except IndexError:
          print_out_str_tz(TzOutPutFile, ("%3x : 0x%08x (%s) : 0x%08x" % (i, g_diag_reset_info[0], "UNKNOWN ERROR".ljust(30), g_diag_reset_info[1])))


      # print the Interupt Table. Interrupt no available at the end of reset table in the dump.
      num_interrupts = (struct.unpack ('<I',LogDumpFile.read(4)))[0] & 0x000FFFFF
      print_out_str_tz(TzOutPutFile, "\n-------- Per Cpu Interupt Status --------- ")
      print_out_str_tz(TzOutPutFile, "Index: IntInfo : Avail : IntType: IntDesc         : IntrCnts => CPU0              : CPU1              : CPU2              : CPU3 ")
      LogDumpFile.seek(TZDiagAddr + g_tzbsp_diag[6],0) # g_tzbsp_diag[6] == Interrupt info Table Offset
      for i in range (0, num_interrupts) :
        g_tzbsp_diag_int = struct.unpack(tzbsp_diag_int_s,LogDumpFile.read(struct.calcsize(tzbsp_diag_int_s)))
        if 1==g_tzbsp_diag[2]:  # for CPU Count 1 , interupt counter will be 4
          g_tzbsp_diag_int_cnt1 = struct.unpack("Q",LogDumpFile.read(8))
          print_out_str_tz(TzOutPutFile, ("%5d: 0x%04x  : 0x%03x : 0x%03x  : %15s : IntrCnts => 0x%-16x"
                                             % (i, g_tzbsp_diag_int[0], g_tzbsp_diag_int[1], g_tzbsp_diag_int[3],
                                             printable_string(g_tzbsp_diag_int[4:3+TZBSP_MAX_INT_DESC]),
                                             g_tzbsp_diag_int_cnt1[0])))
        if 2==g_tzbsp_diag[2]:  # for CPU Count 2 , interupt counter will be 4
          g_tzbsp_diag_int_cnt2 = struct.unpack("QQ",LogDumpFile.read(16))
          print_out_str_tz(TzOutPutFile, ("%5d: 0x%04x  : 0x%03x : 0x%03x  : %15s : IntrCnts => 0x%-16x: 0x%-16x"
                                             % (i, g_tzbsp_diag_int[0], g_tzbsp_diag_int[1], g_tzbsp_diag_int[3],
                                             printable_string(g_tzbsp_diag_int[4:3+TZBSP_MAX_INT_DESC]),
                                             g_tzbsp_diag_int_cnt2[0], g_tzbsp_diag_int_cnt2[1])))

        if 4==g_tzbsp_diag[2]:  # for CPU Count 4, interupt counter will be 4
          g_tzbsp_diag_int_cnt4 = struct.unpack("QQQQ",LogDumpFile.read(32))
          print_out_str_tz(TzOutPutFile, ("%5d: 0x%04x  : 0x%03x : 0x%03x  : %15s : IntrCnts => 0x%-16x: 0x%-16x: 0x%-16x: 0x%-16x"
                                             % (i, g_tzbsp_diag_int[0], g_tzbsp_diag_int[1], g_tzbsp_diag_int[3],
                                             printable_string(g_tzbsp_diag_int[4:3+TZBSP_MAX_INT_DESC]),
                                             g_tzbsp_diag_int_cnt4[0], g_tzbsp_diag_int_cnt4[1],
                                             g_tzbsp_diag_int_cnt4[2],g_tzbsp_diag_int_cnt4[3])))

      # Ring Buffer Contents
      print_out_str_tz(TzOutPutFile, "\n------------------- ******************************* ----------------- ")
      print_out_str_tz(TzOutPutFile, "------ TZBSP DIAG RING BUFF, each line for an individual log ---- ")
      print_out_str_tz(TzOutPutFile, "------------------- ******************************* ----------------- ")
      LogDumpFile.close()
      LogDumpFile = open(LogDumpFile.name, "r")
      LogDumpFile.seek(TZDiagAddr + g_tzbsp_diag[7],0) # g_tzbsp_diag[7 / 8] == Ring Buff Offset / len
      g_tzbsp_ringbuff = StringIO.StringIO(LogDumpFile.read(g_tzbsp_diag[8]))
      line = g_tzbsp_ringbuff.readline()

      # process each line of log against Error Code
      while line :
        error_code = map_to_error(line)
        if error_code:
          line = line + " =====>  TZBSP/QSEE Error!!! => " + error_code
        if "Error Fatal" not in line:
          print_out_str_tz(TzOutPutFile, line)
        line = g_tzbsp_ringbuff.readline()

      # Close files and return
      ShMemDumpFile.close()
      LogDumpFile.close()
      TzOutPutFile.close()
      break;  #exit from while TRUE

#except :
#print '\n !!! Oops - TZBSP Log Extraction error !!!'
#sys.exit(0)

  sys.exit(0)
