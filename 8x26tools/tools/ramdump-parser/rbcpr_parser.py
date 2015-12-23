import sys, re, os
from base_parser import Parser

corner_update = {
    'rail' : {
        0 : 'Cx',
        1 : 'Gfx',
    },
    'corner' : {
        0 : 'OFF [0]',
        1 : 'SVS [1]',
        2 : 'NOMINAL [2]',
        3 : 'TURBO [3]',
        4 : 'COUNT [4]',
    },
}

class RBCPRPreSwitchEntry:
    __metaclass__ = Parser
    id = 0x29E
    def parse(self, data):
        return 'Place Holder, should not print'

class RBCPRCornerUpdateRec:
    __metaclass__ = Parser
    id = 0x29F
    def parse(self, data):
        rail = corner_update['rail'][data[0]]
        corner = corner_update['corner'][data[1]]
        return 'rbcpr_corner_update_rec: (rail: %s) (corner: %s) (step error[%s] %d)' % (rail, corner, ('step up' if data[2] == 2 else ('step down' if data[2] == 1 else 'no step')), data[3])
        
class RBCPRCornerUpdateAct:
    __metaclass__ = Parser
    id = 0x2A0
    def parse(self, data):
        rail = corner_update['rail'][data[0]]
        return 'rbcpr_corner_update_act: (rail: %s) (hit floor? %s) (hit ceiling? %s) (result microvolts: %d)' % (rail, 'yes' if data[1] == 1 else 'no', 'yes' if data[2] == 1 else 'no', data[3])
