import sys, re, os
from base_parser import Parser
from target_data import get_master_name, get_set_name, get_sleep_mode, get_resource_name

class RPMBootEnter:
    __metaclass__ = Parser
    id = 0xC0
    def parse(self, data):
        return 'rpm_boot_started' 

class RPMBootExit:
    __metaclass__ = Parser
    id = 0xC1
    def parse(self, data):
        return 'rpm_boot_finished'

class RPMBringupReq:
    __metaclass__ = Parser
    id = 0xC2
    def parse(self, data):
        message = 'rpm_bringup_req (master: %s)' % (get_master_name(data[0]))
        if len(data) > 1:
            message += ' (core: %i)' % data[1]
        return message

class RPMBringupAck:
    __metaclass__ = Parser
    id = 0xC3
    def parse(self, data):
        message = 'rpm_bringup_ack (master: %s)' % (get_master_name(data[0]))
        if len(data) > 1:
            message += ' (core: %i)' % data[1]
        return message

class RPMShutdownReq:
    __metaclass__ = Parser
    id = 0xC4
    def parse(self, data):
        message = 'rpm_shutdown_req (master: %s)' % (get_master_name(data[0]))
        if len(data) > 1:
            message += ' (core: %i)' % data[1]
        return message

class RPMShutdownAck:
    __metaclass__ = Parser
    id = 0xC5
    def parse(self, data):
        message = 'rpm_shutdown_ack (master: %s)' % (get_master_name(data[0]))
        if len(data) > 1:
            message += ' (core: %i)' % data[1]
        return message

class RPMTransitionQueued:
    __metaclass__ = Parser
    id = 0xC6
    def parse(self, data):
        message = 'rpm_transition_queued (master: %s) ' % get_master_name(data[0])
        if data[1] == 0:
            message += '(scheduled: "no")'
        elif data[1]:
            message += '(scheduled: "yes") (deadline: 0x%0.8x%0.8x)' % (data[3], (data[2]))
        return message

class RPMMasterSetTransition:
    __metaclass__ = Parser
    id = 0xC7
    def parse(self, data):
        return 'rpm_master_set_transition (master: %s) (leaving: %s) (entering: %s)' % (get_master_name(data[0]),
                get_set_name(data[1]), get_set_name(data[2]))

class RPMTransitionComplete:
    __metaclass__ = Parser
    id = 0xC8
    def parse(self, data):
        return 'rpm_master_set_transition_complete (master: %s)' % get_master_name(data[0])

class RPMHashMismatch:
    __metaclass__ = Parser
    id = 0xC9
    def parse(self, data):
        if data[0] == 0:
            result = ' (system_state: %d) (cache_result_state: %d) (result_state: %d)' % (data[1], data[2], data[3])
        else:
            result = ' (next_task: %d) (pre_state: %d) (next_state: %d) (system_hash: %d)' % (data[0], data[1], data[2], data[3])
        return 'rpm_hash_mismatch' + result

class RPMSvsFastExternalVote:
    __metaclass__ = Parser
    id = 0xCA
    def parse(self, data):
        return 'rpm_svs (mode: RPM_SVS_FAST) (reason: external vote)'

class RPMSvsFastImminentProcessing:
    __metaclass__ = Parser
    id = 0xCB
    def parse(self, data):
        return 'rpm_svs (mode: RPM_SVS_FAST) (reason: imminent processing)'

class RPMSvsFastScheduleIsFull:
    __metaclass__ = Parser
    id = 0xCC
    def parse(self, data):
        return 'rpm_svs (mode: RPM_SVS_FAST) (reason: schedule is full)'

class RPMSvsSlowIdle:
    __metaclass__ = Parser
    id = 0xCD
    def parse(self, data):
        return 'rpm_svs (mode: RPM_SVS_SLOW) (reason: idle)'

class RPMSvsFastSpeedup:
    __metaclass__ = Parser
    id = 0xCE
    def parse(self, data):
        return 'rpm_svs (mode: RPM_SVS_FAST) (reason: speedup) (old_duration: 0x%0.8x) (new_duration: 0x%0.8x) (switch_time: 0x%0.8x)' % (data[0], data[1], data[2])

class RPMSvsFastNoSpeedup:
    __metaclass__ = Parser
    id = 0xCF
    def parse(self, data):
        return 'rpm_svs (mode: RPM_SVS_SLOW) (reason: no speedup) (old_duration: 0x%0.8x) (new_duration: 0x%0.8x) (switch_time: 0x%0.8x)' % (data[0], data[1], data[2])

class RPMMessageReceived:
    __metaclass__ = Parser
    id = 0xD0
    def parse(self, data):
        return 'rpm_message_received (master: %s) (message id: %s)' % (get_master_name(data[0]), data[1])

class RPMProcessRequest:
    __metaclass__ = Parser
    id = 0xD1
    def parse(self, data):
        return 'rpm_process_request (master: %s) (resource type: %s) (id: %s)' % (get_master_name(data[0]), get_resource_name(data[1]), data[2])

class RPMMessageReceived:
    __metaclass__ = Parser
    id = 0xD2
    def parse(self, data):
        return 'rpm_send_message_response (master: %s)' % (get_master_name(data[0]))

class RPMErrFatal:
    __metaclass__ = Parser
    id = 0xD3
    def parse(self, data):
        return 'rpm_err_fatal (lr: 0x%0.8x) (ipsr: 0x%0.8x)' % (data[0], data[1])

class RPMXlateRequest:
    __metaclass__ = Parser
    id = 0xD4
    def parse(self, data):
        return 'rpm_xlate_request (resource type: %s) (resource id: %s)' % (get_resource_name(data[0]), data[1])

class RPMApplyRequest:
    __metaclass__ = Parser
    id = 0xD5
    def parse(self, data):
        return 'rpm_apply_request (resource type: %s) (resource id: %s)' % (get_resource_name(data[0]), data[1])

class RPMEstimateCacheHit:
    __metaclass__ = Parser
    id = 0xD6
    def parse(self,data):
        return 'rpm_estimate_cache_hit (estimate: 0x%0.8x)' % (data[0])

class RPMTransitionBypass:
    __metaclass__ = Parser
    id = 0xD8
    def parse(self, data):
        return 'rpm_transition_bypass (master: %s) (wake time: 0x%0.8x)' % (get_master_name(data[0]), data[1])

class RPMResourceSettling:
    __metaclass__ = Parser
    id = 0xD9
    def parse(self, data):
        name = get_resource_name(data[1])
        if get_name_from_resource_id(name, data[2]) != 'Unknown':
            full = ' (full name: %s)' % get_name_from_resource_id(name, data[2])
        else:
            full = ''
        message = 'rpm_resource_settling (master: %s) (resource type: %s) (resource id: %s)' % (get_master_name(data[0]), 
                               get_resource_name(data[1]), data[2])
        message += '%s' % (full)
        if data[3] != 0:
            return 'rpm_message_postponed: ' + message +' (msg id: %s)' % (data[3])
        else:
            return message

class RPMResourceSettlingTime:
    __metaclass__ = Parser
    id = 0xDA
    def parse(self, data):
        return 'rpm_resource_settling_time (settling estimate: 0x%0.8x%0.8x)' % (data[1], data[0]) 

class RPMResourceSettlingComplete:
    __metaclass__ = Parser
    id = 0xDB
    def parse(self, data):
        name = get_resource_name(data[1])
        if get_name_from_resource_id(name, data[2]) != 'Unknown':
            full = ' (full name: %s)' % get_name_from_resource_id(name, data[2])
        else:
            full = ''
        message = 'rpm_resource_settling_complete (master: %s) (resource type: %s) (resource id: %s)' % (get_master_name(data[0]), 
                               get_resource_name(data[1]), data[2])
        message += '%s' % (full)
        if data[3] != 0:
            message += ' (msg id: %s)' % (data[3])
        return message

class RPMResourceSettlingSpin:
    __metaclass__ = Parser
    id = 0xDC
    def parse(self, data):
        return 'rpm_resource_settling_spin master: %s)'

class RPMSettlingMessageRequeued:
    __metaclass__ = Parser
    id = 0xDD
    def parse(self, data):
        return 'rpm_settling_message_requeued (master: %s) (message id: %s)' % (get_master_name(data[0]), data[1])

class RPMSettlingTransitionRequeued:
    __metaclass__ = Parser
    id = 0xDE
    def parse(self, data):
        message = 'rpm_settling_transition_requeued (master: %s) ' % get_master_name(data[0])
        if data[1] == 0:
            message += '(scheduled: "no")'
        elif data[1]:
            message += '(scheduled: "yes") (deadline: 0x%0.8x%0.8x)' % (data[3], (data[2]))
        return message

class RPMTransitionPostponed:
    __metaclass__ = Parser
    id = 0xDF
    def parse(self, data):
        return 'rpm_transition_postponed (master: %s) ' % get_master_name(data[0])
        
class RPMCPRSensorDisabled:
    __metaclass__ = Parser
    id = 0xE0
    def parse(self, data):
        return 'rpm_cpr_sensor_disabled (rail: %d) (disabled: %d)' % (data[0], data[1])

class SLEEPDeepSleepEnter:
    __metaclass__ = Parser
    id = 0x140
    def parse(self, data):
        return 'deep_sleep_enter: (mode: %s) (count: %i)' % (get_sleep_mode(data[0]), data[1]) 

class SLEEPDeepSleepExit:
    __metaclass__ = Parser
    id = 0x141
    def parse(self, data):
        return 'deep_sleep_exit: (mode: %s)' % get_sleep_mode(data[0]) 

class SLEEPNoDeepSleep:
    __metaclass__ = Parser
    id = 0x142
    def parse(self, data):
        if data[1] == 0:
            return 'no deep sleep, not enough time (mode: %s)' % get_sleep_mode(data[0])
        else:
            return 'no deep sleep, pending interrupt (mode: %s)' % get_sleep_mode(data[0])

class SLEEPRpmHaltEnter:
    __metaclass__ = Parser
    id = 0x143
    def parse(self, data):
        return 'rpm_halt_enter (until: 0x%0.16x)' % ((data[0] << 32) | data[1])

class SLEEPRpmHaltExit:
    __metaclass__ = Parser
    id = 0x144
    def parse(self, data):
        return 'rpm_halt_exit'

class SLEEP_mpm_ints:
    __metaclass__ = Parser
    id = 0x145
    def parse(self, data):
        return 'mpm_wakeup_ints (ints: 0x%0.8x 0x%0.8x)' % (data[0], data[1])

class SLEEPDeepSleepEnterComplete:
    __metaclass__ = Parser
    id = 0x146
    def parse(self, data):
        return 'deep_sleep_enter_complete: (mode: %s)' % (get_sleep_mode(data[0])) 

class SLEEPDeepSleepExitComplete:
    __metaclass__ = Parser
    id = 0x147
    def parse(self, data):
        return 'deep_sleep_exit_complete: (mode: %s)' % get_sleep_mode(data[0]) 

