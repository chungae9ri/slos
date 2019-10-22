# This script is to be run in a complete zSDP project. It builds everything.
#
# now synthesize design
#
reset_run synth_1
launch_runs synth_1
wait_on_run synth_1
#
#
# run implementation
#
launch_runs impl_1
wait_on_run impl_1
#
#
# generate bitstream
#
launch_runs impl_1 -to_step write_bitstream
wait_on_run impl_1
##
##
## export hardware
##
##
##
## now launch SDK
##
##
## you should now be in SDK ready to download
