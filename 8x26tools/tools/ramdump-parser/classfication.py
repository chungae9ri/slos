# =======================================================
# hagisiro.py
# -------------------------------------------------------
# 2013/04/16 v0.1 Initial release by darkwood.kim
# =======================================================
import sys
import os

def process_dmesg(dmesg):
	start_oops=0
	end_oops=0
	start_regs=0
	time_table=[]
	dmesg_sorted=[]

	#make list which include time and order
	for i, data in enumerate(dmesg):
		if (data != '' and data[0] == '<' and len(data) > 16):
			time_table.append([data[4:].split()[0], i])

	#sort dmesg by time
	time_table.sort()

	for i, data in enumerate(time_table):
		#make dmesg which sorted by time
		dmesg_sorted.append(dmesg[time_table[i][1]])
		#find starting point of oops message
		if (dmesg[time_table[i][1]].find("Internal error: ") > 0):
			start_oops = i - 3
			start_regs = i + 5
		elif (start_oops == 0) and (dmesg[time_table[i][1]].find("Kernel panic - not syncing: ") > 0):
			start_oops = i - 2
		#find end point of oops message
		elif (dmesg[time_table[i][1]].find("Rebooting in 5 seconds..") > 0):
			end_oops = i - 1

	#write oops message
	output_file = open("kernel_crash.log", "w")
	for i in range(end_oops - start_oops + 1):
		output_file.write(dmesg[time_table[start_oops+i][1]] + '\n')
	output_file.close()

	#write dmesg which sorted by time
	output_file = open("dmesg.log", "w")
	for i, data in enumerate(dmesg_sorted):
		output_file.write(dmesg_sorted[i] + '\n')
	output_file.close()
	
	#write regs_panic.cmm
	if(start_regs != 0):
		search_list = [["pc", "lr"], ["sp", "ip", "fp"], ["r10", "r9", "r8"], ["r7", "r6", "r5", "r4"], ["r3", "r2", "r1", "r0"]]
		write_list = [["pc", "r14"], ["r13", "r12", "r11"], ["r10", "r9", "r8"], ["r7", "r6", "r5", "r4"], ["r3", "r2", "r1", "r0"]]
		search_offset = [[7, 7], [5, 5, 5], [5, 5, 5], [5, 5, 5, 5], [5, 5, 5, 5]]
		output_file = open("regs_panic.cmm", "w")
		for i in range(5):
			for j in range(len(search_offset[i])):
				index = dmesg[time_table[start_regs+i][1]].find(search_list[i][j]) + search_offset[i][j]
				output_file.write("r.s " + write_list[i][j] + " 0x" + dmesg[time_table[start_regs+i][1]][index:index+8] + '\n') 
		output_file.close()
	
def process_kernel_crash(dmseg_TZ):
	start = dmseg_TZ.find("---- dmesg ----")
	start += dmseg_TZ[start:].find('\n') + 1
	end = dmseg_TZ.find("---- end dmesg----") - 1

	dmesg = dmseg_TZ[start:end].split('\n')

	process_dmesg(dmesg)

def process_apps_watchdog(dmseg_TZ):
	start = dmseg_TZ.find("------ watchdog state ------")
	start += dmseg_TZ[start:].find('\n') + 1
	end = dmseg_TZ.find("-------- end watchdog state -------") - 1
	
	output_file = open("apps_watchdog.log", "w")
	output_file.write(dmseg_TZ[start:end])
	output_file.close()

def process_other_crash(other_crash):
	output_file = open(other_crash + '.log', "w")
	output_file.write(other_crash)
	output_file.close()
	
def find_crash_type(input_file):
	idx=0
	
	dmseg_TZ = input_file.read()

	#check apps_watchdog
	if (dmseg_TZ.find("Core 0 recieved the watchdog interrupt") > 0):
		process_apps_watchdog(dmseg_TZ)
	#check kernel crash
	elif (dmseg_TZ.find("Internal error: ") > 0):
		process_kernel_crash(dmseg_TZ)
	elif (dmseg_TZ.find("mdm_errfatal: Received err fatal from mdm") > 0):
		process_other_crash("external_modem")
	else:
		#check subsystem_crash
		idx = dmseg_TZ.find("subsys-restart: Resetting the SoC - ")
		if (idx > 0):
			process_other_crash(dmseg_TZ[idx+36:idx+36+14].split()[0])
		#check kernel crash again for panic() call
		elif (dmseg_TZ.find("Kernel panic - not syncing: ") > 0):
			process_kernel_crash(dmseg_TZ)
		else:
			process_other_crash("unknown reset")

if __name__ == "__main__":
	#open files
	input_file = open(sys.argv[1], "r")

	#find crash type
	find_crash_type(input_file)

	#close files
	input_file.close()
