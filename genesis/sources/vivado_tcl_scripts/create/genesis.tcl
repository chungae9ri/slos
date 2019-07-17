
# Establish project name
set project genesis 	 

# Now create the project, build block design from script, and build.
start_gui
create_project -force $project $project -part xc7z020clg400-1
set_property target_language VHDL [current_project]
set_property simulator_language VHDL [current_project]
set_property  ip_repo_paths ip_repo [current_project]
update_ip_catalog
source sources/vivado_tcl_scripts/block_designs/genesis.tcl
update_compile_order -fileset sources_1
update_ip_catalog -rebuild -scan_changes
report_ip_status -name ip_status
make_wrapper -files [get_files $project/$project.srcs/sources_1/bd/design_1/design_1.bd] -top
add_files -norecurse $project/$project.srcs/sources_1/bd/design_1/hdl/design_1_wrapper.vhd
update_compile_order -fileset sources_1
update_compile_order -fileset sim_1
regenerate_bd_layout
save_bd_design