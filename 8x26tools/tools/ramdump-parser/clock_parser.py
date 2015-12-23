import sys, re, os
from base_parser import Parser
from target_data import get_pll, get_Voltage
from ClockTestDict import CLOCK_TEST_DICT

def get_clock(clock):
	if clock in CLOCK_TEST_DICT:
		return CLOCK_TEST_DICT[clock]
	else:
		return "Unknown Clock ID 0x%X" % clock

class CLOCKInit: 
	__metaclass__ = Parser 
	id = 0x181 
	def parse(self, data): 
		return 'Clock Init' 

class CLOCKStatus: 
	__metaclass__ = Parser 
	id = 0x182 
	def parse(self, data): 
		if data[1] == 1: 
			return 'Clock: %-30s Status = Enable' % get_clock( data[0] ) 
		else: 
			return 'Clock: %-30s Status = Disable' % get_clock( data[0] ) 
 
class CLOCKFreq: 
	__metaclass__ = Parser 
	id = 0x183 
	def parse(self, data): 
	  return 'Clock: %-30s Frequency = %3dMHz' % (get_clock(data[0]), data[1]/1000000) 
 
class CLOCKPLLStatus: 
	__metaclass__ = Parser 
	id = 0x184 
	def parse(self, data): 
		if data[1] == 1: 
			return 'Clock PLL: %-26s Status = Enable' % get_pll( data[0] ) 
		else: 
			return 'Clock PLL: %-26s Status = Disable' % get_pll( data[0] ) 
			 
class CLOCKPowerCollapse: 
	__metaclass__ = Parser 
	id = 0x185 
	def parse(self, data): 
	  return 'Clock Power Collapse' 
			 
class CLOCKPowerRestore: 
	__metaclass__ = Parser 
	id = 0x186 
	def parse(self, data): 
	  return 'Clock Power Restore' 
			 
class CLOCKBIMCCollapse: 
	__metaclass__ = Parser 
	id = 0x187 
	def parse(self, data): 
	  return 'Clock BIMC Collapse' 
			 
class CLOCKBIMCRestore: 
	__metaclass__ = Parser 
	id = 0x188 
	def parse(self, data): 
	  return 'Clock BIMC Restore' 
 
class CLOCKCXVoltage: 
	__metaclass__ = Parser 
	id = 0x189 
	def parse(self, data): 
	  return 'Clock CX Level %35s' % get_Voltage ( data[0] )
 
class CLOCKGFXVoltage: 
	__metaclass__ = Parser 
	id = 0x18A 
	def parse(self, data): 
	  return 'Clock GFX Level %34s' % get_Voltage( data[0] )
	  
