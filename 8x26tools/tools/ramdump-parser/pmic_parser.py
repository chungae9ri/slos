import sys, re, os
from base_parser import Parser

class PMICLdoStartApply:
    __metaclass__ = Parser
    id = 0x201
    def parse(self, data):
        if data[1] == 0:
            return 'START Apply() LDO%dA' % data[0]
        else:
            return 'START Apply() LDO%dB' % data[0]

class PMICLdoStartPreDep:
    __metaclass__ = Parser
    id = 0x202
    def parse(self, data):
        if data[1] == 0:
            return 'START Pre-Dep() LDO%dA' % data[0]
        else:
            return 'START Pre-Dep() LDO%dB' % data[0]

class PMICLdoEndPreDep:
    __metaclass__ = Parser
    id = 0x203
    def parse(self, data):
        if data[1] == 0:
            return 'END Pre-Dep() LDO%dA' % data[0]
        else:
            return 'END Pre-Dep() LDO%dB' % data[0]

class PMICLdoStartPostDep:
    __metaclass__ = Parser
    id = 0x204
    def parse(self, data):
        if data[1] == 0:
            return 'START Post-Dep() LDO%dA' % data[0]
        else:
            return 'START Post-Dep() LDO%dB' % data[0]

class PMICLdoEndPostDep:
    __metaclass__ = Parser
    id = 0x205
    def parse(self, data):
        if data[1] == 0:
            return 'END Post-Dep() LDO%dA' % data[0]
        else:
            return 'END Post-Dep() LDO%dB' % data[0]

class PMICLdoAggregation1:
    __metaclass__ = Parser
    id = 0x206
    def parse(self, data):
        if data[1] == 0:
            return ("START Execute_Driver()\n"
                "\t\tLDO%dA setting:\n"
                "\t\tsw_en = %d\n"
                "\t\tldo_sw_mode = %d") %\
                (data[0], data[2], data[3])
        else:
            return ("START Execute_Driver()\n"
                "\t\tLDO%dB setting:\n"
                "\t\tsw_en = %d\n"
                "\t\tldo_sw_mode = %d") %\
                (data[0], data[2], data[3])

class PMICLdoAggregation2:
    __metaclass__ = Parser
    id = 0x207
    def parse(self, data):
        return ("\tpc_en = %d\n"
            "\t\tpc_mode = %d\n"
            "\t\tis_en_transition = %d\n"
            "\t\tip = %d") %\
            (data[0], data[1], data[2], data[3])

class PMICLdoAggregation3:
    __metaclass__ = Parser
    id = 0x208
    def parse(self, data):
        return ("\tregulated_uvol = %d\n"
            "\t\tbypass_uv = %d\n"
            "\t\tnoise_hr = %d\n"
            "\t\tbyp_allowed = %d") %\
            (data[0], data[1], data[2], data[3])

class PMICLdoAggregation4:
    __metaclass__ = Parser
    id = 0x209
    def parse(self, data):
        return ("\tis_en_transition = %d\n"
            "\t\tnoise_hr = %d\n"
            "\t\ten_byp = %d\n"
            "\t\tbypass_uv = %d") %\
            (data[0], data[1], data[2], data[3])

class PMICLdoAggregation5:
    __metaclass__ = Parser
    id = 0x20a
    def parse(self, data):
        return ("\tbyp_allowed = %d") % data[0]

class PMICLdoPowerMode:
    __metaclass__ = Parser
    id = 0x20b
    def parse(self, data):
        return {
            0: "Setting Mode: IPEAK - NPM",
            1: "Setting Mode: IPEAK - PFM",
            2: "Setting Mode: NPM"
                } [data[0]]

class PMICLdoNoChange:
    __metaclass__ = Parser
    id = 0x20c
    def parse(self, data):
        if data[1] == 0:
            return 'NO CHANGES DETECTED LDO%dA' % data[0]
        else:
            return 'NO CHANGES DETECTED LDO%dB' % data[0]

class PMICSmpsStartApply:
    __metaclass__ = Parser
    id = 0x20d
    def parse(self, data):
        if data[1] == 0:
            return 'START Apply() SMPS%dA' % data[0]    
        else:
            return 'START Apply() SMPS%dB' % data[0]

class PMICSmpsStartPreDep:
    __metaclass__ = Parser
    id = 0x20e
    def parse(self, data):
        if data[1] == 0:
            return 'START Pre-Dep() SMPS%dA' % data[0]
        else:
            return 'START Pre-Dep() SMPS%dB' % data[0]

class PMICSmpsEndPreDep:
    __metaclass__ = Parser
    id = 0x20f
    def parse(self, data):
        if data[1] == 0:
            return 'END Pre-Dep() SMPS%dA' % data[0]
        else:
            return 'END Pre-Dep() SMPS%dB' % data[0]

class PMICSmpsStartPostDep:
    __metaclass__ = Parser
    id = 0x210
    def parse(self, data):
        if data[1] == 0:
            return 'START Post-Dep() SMPS%dA' % data[0]
        else:
            return 'START Post-Dep() SMPS%dB' % data[0]

class PMICSmpsEndPostDep:
    __metaclass__ = Parser
    id = 0x211
    def parse(self, data):
        if data[1] == 0:
            return 'END Post-Dep() SMPS%dA' % data[0]
        else:
            return 'END Post-Dep() SMPS%dB' % data[0]

class PMICSmpsAggregation1:
    __metaclass__ = Parser
    id = 0x212
    def parse(self, data):
        if data[1] == 0:
            return ("START Execute_Driver()\n"
                "\t\tSMPS%dA setting:\n"
                "\t\tsw_en = %d\n"
                "\t\tsmps_sw_mode = %d") %\
                (data[0], data[2], data[3])
        else:
            return ("START Execute_Driver()\n"
                "\t\tSMPS%dB setting:\n"
                "\t\tsw_en = %d\n"
                "\t\tsmps_sw_mode = %d") %\
                (data[0], data[2], data[3])

class PMICSmpsAggregation2:
    __metaclass__ = Parser
    id = 0x213
    def parse(self, data):
        return ("\tpc_en = %d\n"
            "\t\tpc_mode = %d\n"
            "\t\tglobal_byp_en = %d\n"
            "\t\tuvol = %d") %\
            (data[0], data[1], data[2], data[3])

class PMICSmpsAggregation3:
    __metaclass__ = Parser
    id = 0x214
    def parse(self, data):
        return ("\tip = %d\n"
            "\t\tfreq = %d\n"
            "\t\tfreq_reason = %d\n"
            "\t\tquiet_mode = %d") %\
            (data[0], data[1], data[2], data[3])

class PMICSmpsAggregation4:
    __metaclass__ = Parser
    id = 0x215
    def parse(self, data):
        return ("\tbyp_allowed = %d\n"
            "\t\thr = %d") %\
            (data[0], data[1])

class PMICSmpsPowerMode:
    __metaclass__ = Parser
    id = 0x216
    def parse(self, data):
        return {
            0: "Setting Mode: AUTO",
            1: "Setting Mode: IPEAK - NPM",
            2: "Setting Mode: IPEAK - LPM",
            3: "Setting Mode: NPM"
                } [data[0]]

class PMICSmpsNoChange:
    __metaclass__ = Parser
    id = 0x217
    def parse(self, data):
        if data[1] == 0:
            return 'NO CHANGES DETECTED SMPS%dA' % data[0]
        else:
            return 'NO CHANGES DETECTED SMPS%dB' % data[0]

class PMICVsStartApply:
    __metaclass__ = Parser
    id = 0x218
    def parse(self, data):
        if data[1] == 0:
            return 'START Apply() VS%dA' % data[0]
        else:
            return 'START Apply() VS%dB' % data[0]

class PMICVsStartPreDep:
    __metaclass__ = Parser
    id = 0x219
    def parse(self, data):
        if data[1] == 0:
            return 'START Pre-Dep() VS%dA' % data[0]
        else:
            return 'START Pre-Dep() VS%dB' % data[0]

class PMICVsEndPreDep:
    __metaclass__ = Parser
    id = 0x21a
    def parse(self, data):
        if data[1] == 0:
            return 'END Pre-Dep() VS%dA' % data[0]
        else:
            return 'END Pre-Dep() VS%dB' % data[0]


class PMICVsStartPostDep:
    __metaclass__ = Parser
    id = 0x21b
    def parse(self, data):
        if data[1] == 0:
            return 'START Post-Dep() VS%dA' % data[0]
        else:
            return 'START Post-Dep() VS%dB' % data[0]

class PMICVsEndPostDep:
    __metaclass__ = Parser
    id = 0x21c
    def parse(self, data):
        if data[1] == 0:
            return 'END Post-Dep() VS%dA' % data[0]
        else:
            return 'END Post-Dep() VS%dB' % data[0]

class PMICVsAggregation1:
    __metaclass__ = Parser
    id = 0x21d
    def parse(self, data):
        if data[1] == 0:
            return ("START Execute_Driver()\n"
                "\t\tVS%dA setting:\n"
                "\t\tsw_en = %d\n"
                "\t\tpc_en = %d") %\
                (data[0], data[2], data[3])
        else:
            return ("START Execute_Driver()\n"
                "\t\tVS%dB setting:\n"
                "\t\tsw_en = %d\n"
                "\t\tpc_en = %d") %\
                (data[0], data[2], data[3])

class PMICVsAggregation2:
    __metaclass__ = Parser
    id = 0x21e
    def parse(self, data):
        return ("\tuvol = %d\n"
            "\t\tip = %d\n"
            "\t\tisInitialized = %d") %\
            (data[0], data[1], data[2])

class PMICVsNoChange:
    __metaclass__ = Parser
    id = 0x21f
    def parse(self, data):
        if data[1] == 0:
            return 'NO CHANGES DETECTED VS%dA' % data[0]
        else:
            return 'NO CHANGES DETECTED VS%dB' % data[0]

class PMICClkBufStartApply:
    __metaclass__ = Parser
    id = 0x220
    def parse(self, data):
        if data[1] == 0:
            return 'START Apply() CLK BUFFER%dA' % data[0]
        else:
            return 'START Apply() CLK BUFFER%dB' % data[0]

class PMICClkBufStartPreDep:
    __metaclass__ = Parser
    id = 0x221
    def parse(self, data):
        if data[1] == 0:
            return 'START Pre-Dep() CLK BUFFER%dA' % data[0]
        else:
            return 'START Pre-Dep() CLK BUFFER%dB' % data[0]

class PMICClkBufEndPreDep:
    __metaclass__ = Parser
    id = 0x222
    def parse(self, data):
        if data[1] == 0:
            return 'END Pre-Dep() CLK BUFFER%dA' % data[0]
        else:
            return 'END Pre-Dep() CLK BUFFER%dB' % data[0]

class PMICClkBufStartPostDep:
    __metaclass__ = Parser
    id = 0x223
    def parse(self, data):
        if data[1] == 0:
            return 'START Post-Dep() CLK BUFFER%dA' % data[0]
        else:
            return 'START Post-Dep() CLK BUFFER%dB' % data[0]

class PMICClkBufEndPostDep:
    __metaclass__ = Parser
    id = 0x224
    def parse(self, data):
        if data[1] == 0:
            return 'END Post-Dep() CLK BUFFER%dA' % data[0]
        else:
            return 'END Post-Dep() CLK BUFFER%dB' % data[0]

class PMICClkBufAggregation1:
    __metaclass__ = Parser
    id = 0x225
    def parse(self, data):
        if data[1] == 0:
            return ("START Execute_Driver()\n"
                "\t\tCLK BUFFER%dA setting:\n"
                "\t\tsw_en = %d\n"
                "\t\tpc_en = %d") %\
                (data[0], data[2], data[3])
        else:
            return ("START Execute_Driver()\n"
                "\t\tCLK BUFFER%dB setting:\n"
                "\t\tsw_en = %d\n"
                "\t\tpc_en = %d") %\
                (data[0], data[2], data[3])

class PMICClkBufAggregation2:
    __metaclass__ = Parser
    id = 0x226
    def parse(self, data):
        return ("\tisInitialized = %d") % data[0]

class PMICClkBufNoChange:
    __metaclass__ = Parser
    id = 0x227
    def parse(self, data):
        if data[1] == 0:
            return 'NO CHANGES DETECTED CLK BUFFER%dA' % data[0]
        else:
            return 'NO CHANGES DETECTED CLK BUFFER%dB' % data[0]

class PMICDrvPullDownErr:
    __metaclass__ = Parser
    id = 0x228
    def parse(self, data):
        if data[1] == 0x2F:
            return 'PMIC DRV PULL DOWN ERROR -- Resource%d Invalid Resource Index!' % data[0]
        elif data[1] == 0x24:
            return 'PMIC DRV PULL DOWN ERROR -- Resource%d Invalid Cmd!' % data[0]
        elif data[1] == 0x09:
            return 'PMIC DRV PULL DOWN ERROR -- Resource%d Spmi Bus Error!' % data[0]

class PMICDrvSwEnableErr:
    __metaclass__ = Parser
    id = 0x229
    def parse(self, data):
        if data[1] == 0x2F:
            return 'PMIC DRV SW EN ERROR -- Resource%d Invalid Resource Index!' % data[0]
        elif data[1] == 0x24:
            return 'PMIC DRV SW EN ERROR -- Resource%d Invalid Cmd!' % data[0]
        elif data[1] == 0x09:
            return 'PMIC DRV SW EN ERROR -- Resource%d Spmi Bus Error!' % data[0]
        elif data[1] == 0x72:
            return 'PMIC DRV SW EN ERROR -- Resource%d Invalid Pointer!' % data[0]

class PMICDrvPinCtrlErr:
    __metaclass__ = Parser
    id = 0x22a
    def parse(self, data):
        if data[1] == 0x2F:
            return 'PMIC DRV PIN CTRL ERROR -- Resource%d Invalid Resource Index!' % data[0]
        elif data[1] == 0x09:
            return 'PMIC DRV PIN CTRL ERROR -- Resource%d Spmi Bus Error!' % data[0]

class PMICDrvPinCtrlModeErr:
    __metaclass__ = Parser
    id = 0x22b
    def parse(self, data):
        if data[1] == 0x2F:
            return 'PMIC DRV PIN CTRL MODE ERROR -- Resource%d Invalid Resource Index!' % data[0]
        elif data[1] == 0x09:
            return 'PMIC DRV PIN CTRL MODE ERROR -- Resource%d Spmi Bus Error!' % data[0]

class PMICDrvVoltLevelErr:
    __metaclass__ = Parser
    id = 0x22c
    def parse(self, data):
        if data[1] == 0x2F:
            return 'PMIC DRV VOLT LVL ERROR -- Resource%d Invalid Resource Index!' % data[0]
        elif data[1] == 0x09:
            return 'PMIC DRV VOLT LVL ERROR -- Resource%d Spmi Bus Error!' % data[0]
        elif data[1] == 0x72:
            return 'PMIC DRV VOLT LVL ERROR -- Resource%d Invalid Pointer!' % data[0]
        elif data[1] == 0x14:
            return 'PMIC DRV VOLT LVL ERROR -- Resource%d Unsupported Voltage!' % data[0]


class PMICDrvSmpsClkSrc:
    __metaclass__ = Parser
    id = 0x22d
    def parse(self, data):
        if data[1] == 0x2F:
            return 'PMIC DRV SMPS CLK SRC ERROR -- Resource%d Invalid Resource Index!' % data[0]
        elif data[1] == 0x24:
            return 'PMIC DRV SMPS CLK SRC ERROR -- Resource%d Invalid Cmd!' % data[0]
        elif data[1] == 0x09:
            return 'PMIC DRV SMPS CLK SRC ERROR -- Resource%d Spmi Bus Error!' % data[0]

class PMICDrvPwrModeErr:
    __metaclass__ = Parser
    id = 0x22e
    def parse(self, data):
        if data[1] == 0x2F:
            return 'PMIC DRV PWR MODE ERROR -- Resource%d Invalid Resource Index!' % data[0]
        elif data[1] == 0x24:
            return 'PMIC DRV PWR MODE ERROR -- Resource%d Invalid Cmd!' % data[0]
        elif data[1] == 0x09:
            return 'PMIC DRV PWR MODE ERROR -- Resource%d Spmi Bus Error!' % data[0]
        elif data[1] == 0x72:
            return 'PMIC DRV PWR MODE ERROR -- Resource%d Invalid Pointer!' % data[0]

class PMICBoostStartApply:
    __metaclass__ = Parser
    id = 0x22f
    def parse(self, data):
        if data[1] == 0:
            return 'START Apply() BOOST%dA' % data[0]   
        else:
            return 'START Apply() BOOST%dB' % data[0]

class PMICBoostStartPreDep:
    __metaclass__ = Parser
    id = 0x230
    def parse(self, data):
        if data[1] == 0:
            return 'START Pre-Dep() BOOST%dA' % data[0] 
        else:
            return 'START Pre-Dep() BOOST%dB' % data[0]

class PMICBoostEndPreDep:
    __metaclass__ = Parser
    id = 0x231
    def parse(self, data):
        if data[1] == 0:
            return 'END Pre-Dep() BOOST%dA' % data[0]   
        else:
            return 'END Pre-Dep() BOOST%dB' % data[0]

class PMICBoostStartPostDep:
    __metaclass__ = Parser
    id = 0x232
    def parse(self, data):
        if data[1] == 0:
            return 'START Post-Dep() BOOST%dA' % data[0]    
        else:
            return 'START Post-Dep() BOOST%dB' % data[0]

class PMICBoostEndPostDep:
    __metaclass__ = Parser
    id = 0x233
    def parse(self, data):
        if data[1] == 0:
            return 'END Post-Dep() BOOST%dA' % data[0]  
        else:
            return 'END Post-Dep() BOOST%dB' % data[0]

class PMICBoostAggregation1:
    __metaclass__ = Parser
    id = 0x234
    def parse(self, data):
        return "\tsw_en = %d\n" %   data[0]

class PMICBoostNoChange:
    __metaclass__ = Parser
    id = 0x235
    def parse(self, data):
        if data[1] == 0:
            return 'NO CHANGES DETECTED BOOST%dA' % data[0]
        else:
            return 'NO CHANGES DETECTED BOOST%dB' % data[0]

class PMICRailway:
   __metaclass__ = Parser
   id = 0x236
   def parse(self, data):
       if data[1] == 1:
            if data[0] == 0:      
                return 'PMIC RAILWAY -- MX Settling Timeout!'
            elif data[0] == 1: 
                return 'PMIC RAILWAY -- CX Settling Timeout!'      
            elif data[0] == 2: 
                return 'PMIC RAILWAY -- GFX Settling Timeout!'      
       elif data[1] == 2:
            return  'PMIC RAILWAY -- Invalid Pointer'    
       else:
            return 'PMIC RAILWAY -- Success'   

class PMICRpmError:
   __metaclass__ = Parser
   id = 0x237
   def parse(self, data):
       if data[1] == 1:
           if data[0] == 1:      
               return 'PMIC RPM ERROR -- Null Clk Call Back Data!'
           elif data[0] == 2: 
               return 'PMIC RPM ERROR -- Null Ldo Call Back Data!'      
           elif data[0] == 3: 
               return 'PMIC RPM ERROR -- Null Smps Call Back Data!'      
           elif data[0] == 4:
               return 'PMIC RPM ERROR -- Null Vs Call Back Data!'    
           elif data[0] == 5:
               return 'PMIC RPM ERROR -- Null Boost Call Back Data!'  
       elif data[1] == 2:  
           if data[0] == 2: 
               return 'PMIC RPM ERROR -- Ldo Settling Timeout!'      
           elif data[0] == 3: 
               return 'PMIC RPM ERROR -- Smps Settling Timeout!'      
           elif data[0] == 4:
               return 'PMIC RPM ERROR -- Vs Settling Timeout!'    
           elif data[0] == 5:
               return 'PMIC RPM ERROR -- Boost Settling Timeout!'  

class PMICDrvSmpsIlimErr:
    __metaclass__ = Parser
    id = 0x238
    def parse(self, data):
        if data[1] == 0x2F:
            return 'PMIC DRV SMPS ILIM ERROR -- Resource%d Invalid Resource Index!' % data[0]
        elif data[1] == 0x24:
            return 'PMIC DRV SMPS ILIM ERROR -- Resource%d Invalid Cmd!' % data[0]
        elif data[1] == 0x09:
            return 'PMIC DRV SMPS ILIM ERROR -- Resource%d Spmi Bus Error!' % data[0]
        elif data[1] == 0x72:
            return 'PMIC DRV SMPS ILIM ERROR -- Resource%d Invalid Pointer!' % data[0]

class PMICDrvSmpsQmErr:
    __metaclass__ = Parser
    id = 0x239
    def parse(self, data):
        if data[1] == 0x2F:
            return 'PMIC DRV SMPS QM ERROR -- Resource%d Invalid Resource Index!' % data[0]
        elif data[1] == 0x24:
            return 'PMIC DRV SMPS QM ERROR -- Resource%d Invalid Cmd!' % data[0]
        elif data[1] == 0x09:
            return 'PMIC DRV SMPS QM ERROR -- Resource%d Spmi Bus Error!' % data[0]
        elif data[1] == 0x14:
            return 'PMIC DRV SMPS QM ERROR -- Resource%d Unsupported Voltage!' % data[0]

class PMICDrvSmpsSwFreqErr:
    __metaclass__ = Parser
    id = 0x23A
    def parse(self, data):
        if data[1] == 0x2F:
            return 'PMIC DRV SMPS SWITCH FREQ ERROR -- Resource%d Invalid Resource Index!' % data[0]
        elif data[1] == 0x24:
            return 'PMIC DRV SMPS SWITCH FREQ ERROR -- Resource%d Invalid Cmd!' % data[0]
        elif data[1] == 0x09:
            return 'PMIC DRV SMPS SWITCH FREQ ERROR -- Resource%d Spmi Bus Error!' % data[0]
        elif data[1] == 0x72:
            return 'PMIC DRV SMPS SWITCH FREQ ERROR -- Resource%d Invalid Pointer!' % data[0]

class PMICDrvClkErr:
    __metaclass__ = Parser
    id = 0x23B
    def parse(self, data):
        if data[1] == 0x3B:
            return 'PMIC DRV CLK ERROR -- Resource%d Invalid Resource Index!' % data[0]
        elif data[1] == 0x24:
            return 'PMIC DRV CLK ERROR -- Resource%d Invalid Cmd!' % data[0]
        elif data[1] == 0x09:
            return 'PMIC DRV CLK ERROR -- Resource%d Spmi Bus Error!' % data[0]
        elif data[1] == 0x72:
            return 'PMIC DRV CLK ERROR -- Resource%d Invalid Pointer!' % data[0]

class PMICLdoStartSettling:
    __metaclass__ = Parser
    id = 0x23C
    def parse(self, data):
        if data[1] == 0:
            return ("START Settling()\n"
                "\t\tLDO%dA setting_time %d \n") %\
                (data[0], data[2])
        else:
            return ("START Settling()\n"
                "\t\tLDO%dB setting_time %d \n") %\
                (data[0], data[2])

class PMICLdoEndSettling:
    __metaclass__ = Parser
    id = 0x23D
    def parse(self, data):
        if data[1] == 0:
            return 'END Settling() LDO%dA' % data[0]
        else:
            return 'END Settling() LDO%dB' % data[0]

class PMICSmpsStartSettling:
    __metaclass__ = Parser
    id = 0x23E
    def parse(self, data):
        if data[1] == 0:
            return ("START Settling()\n"
                "\t\tSMPS%dA setting_time %d \n") %\
                (data[0], data[2])
        else:
            return ("START Settling()\n"
                "\t\tSMPS%dB setting_time %d \n") %\
                (data[0], data[2])

class PMICSmpsEndSettling:
    __metaclass__ = Parser
    id = 0x23F
    def parse(self, data):
        if data[1] == 0:
            return 'END Settling() SMPS%dA' % data[0]
        else:
            return 'END Settling() SMPS%dB' % data[0]

class PMICVsStartSettling:
    __metaclass__ = Parser
    id = 0x240
    def parse(self, data):
        if data[1] == 0:
            return ("START Settling()\n"
                "\t\tVS%dA setting_time %d \n") %\
                (data[0], data[2])
        else:
            return ("START Settling()\n"
                "\t\tVS%dB setting_time %d \n") %\
                (data[0], data[2])

class PMICVsEndSettling:
    __metaclass__ = Parser
    id = 0x241
    def parse(self, data):
        if data[1] == 0:
            return 'END Settling() VS%dA' % data[0]
        else:
            return 'END Settling() VS%dB' % data[0]

class PMICBoostStartSettling:
    __metaclass__ = Parser
    id = 0x242
    def parse(self, data):
        if data[1] == 0:
            return ("START Settling()\n"
                "\t\tBOOST%dA setting_time %d \n") %\
                (data[0], data[2])
        else:
            return ("START Settling()\n"
                "\t\tBOOST%dB setting_time %d \n") %\
                (data[0], data[2])     
        
class PMICBoostEndSettling:
    __metaclass__ = Parser
    id = 0x243
    def parse(self, data):
        if data[1] == 0:
            return 'END Settling() BOOST%dA' % data[0]
        else:
            return 'END Settling() BOOST%dB' % data[0]             
