# Vivado TCL Complete Command Reference

Extracted from UG835 (TCL Command Reference) and UG892 (Design Flows Overview), v2025.2.

## 1. Execution Models

### Batch Mode (recommended for automation)
```
vivado -mode batch -source <script.tcl>
vivado -mode batch -source script.tcl -tclargs "FPGA=I15-2"
```
Vivado exits after script completes. Journal file `vivado.jou` records all commands.

### TCL Shell Mode (interactive)
```
vivado -mode tcl
```

### Key Files
- `vivado.log` — full session log
- `vivado.jou` — journal of TCL commands only (reusable as script)
- `Vivado_init.tcl` — user init script (searched in install dir, then `$HOME/.Xilinx/Vivado/<version>/`)

---

## 2. Two Flow Modes — Commands Are NOT Interchangeable

### Project Mode
Vivado manages files, runs, status, reports automatically. Use wrapper commands.

| Stage | TCL Commands |
|-------|-------------|
| Create project | `create_project <name> <dir> -part <part>` |
| Add sources | `add_files`, `import_files`, `set_property` |
| Set fileset properties | `set_property library <lib> [get_files ...]` |
| Launch synthesis | `launch_runs synth_1` |
| Wait for completion | `wait_on_run synth_1` |
| Open synth results | `open_run synth_1 -name netlist_1` |
| Reports after synth | `report_timing_summary`, `report_power` |
| Launch implementation+bitstream | `launch_runs impl_1 -to_step write_bitstream` |
| Wait for implementation | `wait_on_run impl_1` |
| Open impl results | `open_run impl_1` |
| GUI | `start_gui` / `stop_gui` |
| Close | `close_design`, `close_project` |

### Non-Project Mode
User has full control. Each step is an explicit TCL command. No project infrastructure.

| Stage | TCL Commands |
|-------|-------------|
| Read sources | `read_verilog`, `read_vhdl`, `read_edif`, `read_ip`, `read_bd`, `read_xdc` |
| Set parameters | `set_param`, `set_property` |
| Link design | `link_design` (if netlist sources) |
| Synthesis | `synth_design -top <top> -part <part>` |
| Save checkpoint | `write_checkpoint -force <file>.dcp` |
| Optimization | `opt_design` |
| Power optimization | `power_opt_design` (optional) |
| Placement | `place_design` |
| Physical optimization | `phys_opt_design` (optional) |
| Routing | `route_design` |
| Reports | `report_timing_summary`, `report_utilization`, `report_power`, `report_drc`, etc. |
| Write outputs | `write_bitstream`, `write_verilog`, `write_xdc`, `write_checkpoint` |
| Read checkpoint | `read_checkpoint <file>.dcp` / `open_checkpoint <file>.dcp` |

**CRITICAL RULE**: Do NOT mix mode-specific commands. In Project Mode, do NOT use `synth_design`, `opt_design`, `place_design`, `route_design`. In Non-Project Mode, do NOT use `add_files`, `import_files`, `launch_runs`.

---

## 3. TCL Command Categories (48 categories, ~700+ commands)

### Core Design Flow
- **Project**: `create_project`, `open_project`, `close_project`, `add_files`, `import_files`, `launch_runs`, `wait_on_run`, `open_run`, `create_run`, `current_run`, `set_property`, `get_property`, `get_files`, `get_runs`, `save_project_as`, `archive_project`, `close_design`, `current_project`, `reset_project`, `update_design`
- **Tools** (Non-Project flow): `synth_design`, `opt_design`, `place_design`, `place_design_advanced`, `phys_opt_design`, `route_design`, `link_design`, `iphys_opt_design`, `config_linter`, `finalize_eco`, `update_clock_routing`
- **FileIO** (read/write): `read_verilog`, `read_vhdl`, `read_edif`, `read_ip`, `read_xdc`, `read_bd`, `read_checkpoint`, `read_mem`, `read_csv`, `read_saif`, `write_bitstream`, `write_checkpoint`, `write_verilog`, `write_vhdl`, `write_edif`, `write_xdc`, `write_sdf`, `write_mem_info`, `write_csv`, `write_schematic`, `write_debug_probes`, `write_hw_platform`, `generate_pblock`, `generate_base_platform`, `encrypt`, `decrypt_bitstream`

### Timing & Constraints (SDC/XDC)
- **SDC**: `create_clock`, `create_generated_clock`, `set_clock_groups`, `set_clock_latency`, `set_clock_uncertainty`, `set_input_delay`, `set_output_delay`, `set_false_path`, `set_multicycle_path`, `set_max_delay`, `set_min_delay`, `set_case_analysis`, `set_clock_sense`, `set_disable_timing`, `set_data_check`, `group_path`, `set_logic_dc/one/zero`, `set_load`, `set_operating_conditions`, `all_clocks`, `all_inputs`, `all_outputs`, `all_registers`, `current_design`, `current_instance`, `get_cells`, `get_nets`, `get_pins`, `get_ports`, `get_clocks`, `get_generated_clocks`
- **Timing reports**: `report_timing`, `report_timing_summary`, `report_clock_networks`, `report_clock_interaction`, `report_clock_utilization`, `report_clocks`, `report_config_timing`, `report_constant_paths`, `report_bus_skew`, `report_pulse_width`, `report_datasheet`, `report_disable_timing`, `report_exceptions`, `report_high_fanout_nets`, `check_timing`, `config_timing_analysis`, `config_timing_corners`, `create_slack_histogram`, `delete_timing_results`, `get_timing_arcs`, `get_timing_paths`, `reset_timing`, `update_timing`, `set_delay_model`
- **XDC** (superset of SDC + Xilinx extensions): `create_pblock`, `add_cells_to_pblock`, `resize_pblock`, `create_macro`, `update_macro`, `create_property`, `set_property`, `create_debug_core`, `create_debug_port`, `connect_debug_port`, `create_power_rail`, `create_noc_connection/interface`

### IP & Block Design (IPFlow + IPIntegrator)
- **IPFlow**: `create_ip`, `copy_ip`, `convert_ips`, `config_ip_cache`, `create_ip_run`, `generate_target`, `synth_ip`, `upgrade_ip`, `validate_ip`, `import_ip`, `read_ip`, `report_ip_status`, `update_ip_catalog`, `get_ips`, `get_ipdefs`, `write_ip_tcl`, `open_example_project`, `reset_target`, `update_module_reference`
- **IPIntegrator**: `create_bd_design`, `open_bd_design`, `close_bd_design`, `save_bd_design`, `validate_bd_design`, `create_bd_cell`, `create_bd_port`, `create_bd_pin`, `create_bd_net`, `create_bd_intf_port`, `create_bd_intf_pin`, `connect_bd_net`, `connect_bd_intf_net`, `disconnect_bd_net`, `disconnect_bd_intf_net`, `apply_bd_automation`, `assign_bd_address`, `get_bd_cells`, `get_bd_pins`, `get_bd_ports`, `get_bd_nets`, `get_bd_intf_ports`, `get_bd_intf_nets`, `get_bd_designs`, `get_bd_addr_segs`, `get_bd_addr_spaces`, `current_bd_design`, `current_bd_instance`, `regenerate_bd_layout`, `write_bd_tcl`, `make_bd_intf_pins_external`, `make_bd_pins_external`, `group_bd_cells`, `export_as_example_design`

### Simulation
- `launch_simulation`, `xsim`, `run`, `restart`, `step`, `stop`, `add_force`, `remove_forces`, `add_wave`, `add_bp`, `remove_bps`, `add_condition`, `remove_conditions`, `log_wave`, `log_vcd`, `log_saif`, `open_vcd`, `close_vcd`, `start_vcd`, `stop_vcd`, `open_saif`, `close_saif`, `open_wave_database`, `get_objects`, `get_scopes`, `get_value`, `set_value`, `report_values`, `report_bps`, `report_conditions`, `current_sim`, `current_time`, `current_scope`, `current_frame`, `create_wave_config`, `create_fileset`, `create_testbench`, `export_simulation`, `export_ip_user_files`, `compile_simlib`, `config_compile_simlib`, `get_simulators`, `report_sim_env`, `report_simlib_info`, `describe`

### Hardware & Programming
- **Hardware Manager**: `open_hw_manager`, `close_hw_manager`, `connect_hw_server`, `disconnect_hw_server`, `get_hw_servers`, `get_hw_targets`, `open_hw_target`, `close_hw_target`, `current_hw_server`, `current_hw_target`, `current_hw_device`, `get_hw_devices`, `program_hw_devices`, `refresh_hw_device`, `refresh_hw_server`, `refresh_hw_target`, `boot_hw_device`
- **Bitstream/ILA/VIO**: `create_hw_bitstream`, `delete_hw_bitstream`, `create_hw_probe`, `delete_hw_probe`, `get_hw_probes`, `get_hw_ilas`, `current_hw_ila`, `current_hw_ila_data`, `run_hw_ila`, `display_hw_ila_data`, `read_hw_ila_data`, `upload_hw_ila_data`, `wait_on_hw_ila`, `get_hw_vios`, `refresh_hw_vio`, `commit_hw_vio`, `reset_hw_vio_activity`, `get_hw_sio_commons/gts/iberts/links/scans/sweeps`, `run_hw_sio_scan`, `run_hw_sio_sweep`
- **JTAG**: `runtest_hw_jtag`, `scan_dr_hw_jtag`, `scan_ir_hw_jtag`, `run_state_hw_jtag`
- **SysMon/HBM**: `get_hw_sysmons`, `get_hw_hbms`, `get_hw_hbmmcs`, `get_hw_migs`, `refresh_hw_hbm/hbmmc/mig/sysmon`, `report_hw_mig`

### Debug Core Setup (pre-implementation)
- `create_debug_core`, `delete_debug_core`, `get_debug_cores`, `create_debug_port`, `delete_debug_port`, `get_debug_ports`, `connect_debug_port`, `disconnect_debug_port`, `modify_debug_ports`, `implement_debug_core`, `report_debug_core`, `write_debug_probes`

### DRC & Methodology
- `report_drc`, `report_methodology`, `create_drc_check`, `create_drc_ruledeck`, `add_drc_checks`, `get_drc_checks`, `get_drc_violations`, `create_waiver`, `get_waivers`, `write_waivers`, `report_waivers`

### Reports (general)
- `report_utilization`, `report_power`, `report_timing`, `report_timing_summary`, `report_drc`, `report_methodology`, `report_clock_utilization`, `report_clock_networks`, `report_clock_interaction`, `report_design_analysis`, `report_qor_assessment`, `report_qor_suggestions`, `report_ram_utilization`, `report_route_status`, `report_ssn`, `report_io`, `report_carry_chains`, `report_phys_opt`, `report_incremental_reuse`, `report_config_implementation`, `report_control_sets`, `report_environment`, `report_param`, `report_property`, `report_slr_crossing`, `report_synchronizer_mtbf`, `report_transformed_primitives`, `version`

### Floorplanning
- `create_pblock`, `delete_pblocks`, `get_pblocks`, `resize_pblock`, `add_cells_to_pblock`, `remove_cells_from_pblock`, `move_pblock`, `place_cell`, `unplace_cell`, `swap_locs`, `create_power_rail`, `delete_power_rails`, `get_power_rails`, `add_to_power_rail`, `remove_from_power_rail`, `delete_rpm`

### Device Queries
- `get_bels`, `get_bel_pins`, `get_sites`, `get_site_pins`, `get_site_pips`, `get_tiles`, `get_nodes`, `get_pips`, `get_wires`, `get_clock_regions`, `get_iobanks`, `get_io_standards`, `get_slrs`, `get_speed_models`, `get_pkgpin_nibbles`, `get_pkgpin_bytegroups`, `find_routing_path`

### Board
- `current_board`, `current_board_part`, `get_boards`, `get_board_parts`, `get_board_components`, `get_board_component_interfaces`, `get_board_component_modes`, `get_board_component_pins`, `get_board_bus_nets`, `get_board_buses`, `get_board_interface_ports`, `get_board_parameters`, `get_board_part_interfaces`, `get_board_part_pins`, `get_board_ip_preferences`, `get_board_jumpers`, `apply_board_connection`, `validate_board_files`, `list_board_parameters`

### Object Model & Properties
- `get_cells`, `get_nets`, `get_pins`, `get_ports`, `get_clocks`, `get_property`, `set_property`, `list_property`, `list_property_value`, `report_property`, `reset_property`, `filter`, `get_selected_objects`, `get_highlighted_objects`, `mark_objects`
- Object types: Cell, Pin, Port, Net, Clock, Bel, BelPin, Site, SitePin, Tile, Node, Pip, Wire, ClockRegion, IOBank, IOStandard, PackagePin, TimingPath, TimingArc

### Netlist Editing
- `create_cell`, `remove_cell`, `rename_cell`, `create_net`, `remove_net`, `rename_net`, `create_pin`, `remove_pin`, `rename_pin`, `connect_net`, `disconnect_net`, `rename_port`, `rename_ref`, `resize_net_bus`, `resize_pin_bus`, `tie_unused_pins`, `add_qor_checks`, `create_dataflow_design`, `get_dataflow_paths`

### NOC (Network on Chip, for Versal)
- `create_noc_connection`, `create_noc_interface`, `delete_noc_connection`, `delete_noc_interface`, `get_noc_connections`, `get_noc_interfaces`, `get_noc_logical_instances`, `get_noc_logical_paths`, `get_noc_net_routes`, `clear_noc_solution`, `read_noc_solution`, `write_noc_solution`, `report_noc_qos`, `run_noc_compiler`, `set_noc_phase`, `write_noc_qos`, `update_noc_qos`, `validate_noc`

### Dynamic Function Exchange (DFX/Partial Reconfiguration)
- `create_partition_def`, `create_reconfig_module`, `create_pr_configuration`, `delete_partition_defs`, `delete_reconfig_modules`, `delete_pr_configurations`, `get_partition_defs`, `get_reconfig_modules`, `get_pr_configurations`, `setup_pr_configurations`, `current_pr_configuration`

### Waveform Viewer
- `add_wave`, `add_wave_divider`, `add_wave_group`, `add_wave_marker`, `add_wave_virtual_bus`, `create_wave_config`, `open_wave_config`, `close_wave_config`, `save_wave_config`, `current_wave_config`, `get_wave_configs`, `get_waves`, `move_wave`, `remove_wave`, `select_wave_objects`

### Pin Planning
- `create_port`, `remove_port`, `create_interface`, `delete_interface`, `place_ports`, `set_package_pin_val`, `make_diff_pair_ports`, `split_diff_pair_ports`, `resize_port_bus`

### Power
- `report_power`, `report_power_opt`, `power_opt_design`, `set_operating_conditions`, `reset_operating_conditions`, `set_switching_activity`, `reset_switching_activity`, `read_saif`, `set_power_opt`, `delete_power_results`

### GUIControl
- `start_gui`, `stop_gui`, `show_schematic`, `show_objects`, `select_objects`, `unselect_objects`, `highlight_objects`, `unhighlight_objects`, `mark_objects`, `unmark_objects`, `redo`, `undo`, `startgroup`, `endgroup`, `create_gui_custom_command`

### Platform (Vitis)
- `open_hw_platform`, `write_hw_platform`, `write_hw_platform_metadata`, `validate_hw_platform`

---

## 4. TCL Syntax Essentials for Vivado

### Command naming convention
- `get_*` — query objects
- `set_*` — set values/properties
- `report_*` — generate reports
- `create_*` / `delete_*` — create/destroy objects
- `read_*` / `write_*` — file I/O
- Flat namespace (no sub-commands), underscore-separated verb-noun

### Object queries with filtering
```tcl
# Basic query
get_cells *
get_cells */inst_1
get_cells -hierarchical inst_*

# Filter by property
get_cells * -hierarchical -filter "lib_cell == FD"
get_cells * -hierarchical -filter "lib_cell =~ FD*"
get_cells * -hierarchical -filter {lib_cell =~ FD* && loc != ""}

# Object relationships via -of
get_pins -of [get_cells inst_1]
get_nets -of [get_pins inst_1/D]
get_cells -of [get_nets my_net]

# Property queries
get_property lib_cell [get_cells inst_1]
report_property [get_cells inst_1]
set_property loc OLOGIC_X1Y27 [get_cells inst_1]
```

### Special character handling
```tcl
# Bus indexing — use braces or parentheses
add_wave {bus[4]}     ;# braces for square brackets
add_wave bus(4)       ;# parentheses work without braces

# Verilog escaped identifiers — use braces
add_wave {\my wire }  ;# space before closing brace required

# VHDL extended identifiers — backslash each special char
add_wave \\my\\ sig\\
```

### Error handling
```tcl
if {[catch {<command>} result]} {
    puts "Error: $result"
    puts $ERRORINFO
}
# Commands return TCL_OK or TCL_ERROR
```

### Tcl.pre and Tcl.post hooks
Every design flow step supports pre/post hook scripts for customization.
```tcl
# Hook script paths are relative to:
get_property DIRECTORY [current_project]
get_property DIRECTORY [current_run]
```

---

## 5. Complete Workflow Templates

### Template A: Project Mode — RTL to Bitstream
```tcl
# Create project
create_project my_project ./my_project -part xc7k70tfbg484-2

# Add sources
add_files {./src/top.v ./src/module1.v ./src/module2.v}
add_files -fileset constrs_1 ./constraints/timing.xdc
update_compile_order -fileset sources_1

# Launch synthesis
launch_runs synth_1
wait_on_run synth_1

# Open synthesis results, generate reports
open_run synth_1 -name netlist_1
report_timing_summary -delay_type max -file syn_timing.rpt
report_power -file syn_power.rpt

# Launch implementation through bitstream
launch_runs impl_1 -to_step write_bitstream
wait_on_run impl_1

# Open implementation results, generate reports
open_run impl_1
report_timing_summary -delay_type min_max -file imp_timing.rpt
report_power -file imp_power.rpt
report_utilization -file imp_util.rpt

# Optional: open GUI
start_gui
```

### Template B: Non-Project Mode — RTL to Bitstream
```tcl
# STEP 0: Setup
set outputDir ./output
file mkdir $outputDir

# STEP 1: Read sources
read_verilog {./src/top.v ./src/module1.v}
read_xdc ./constraints/timing.xdc

# STEP 2: Synthesis
synth_design -top top -part xc7k70tfbg484-2
write_checkpoint -force $outputDir/post_synth.dcp
report_timing_summary -file $outputDir/post_synth_timing.rpt
report_power -file $outputDir/post_synth_power.rpt

# STEP 3: Implementation
opt_design
place_design
# Optional: phys_opt_design
write_checkpoint -force $outputDir/post_place.dcp
report_timing_summary -file $outputDir/post_place_timing.rpt

# STEP 4: Route
route_design
write_checkpoint -force $outputDir/post_route.dcp
report_timing_summary -file $outputDir/post_route_timing.rpt
report_timing -sort_by group -max_paths 100 -path_type summary -file $outputDir/post_route_timing_detail.rpt
report_clock_utilization -file $outputDir/clock_util.rpt
report_utilization -file $outputDir/post_route_util.rpt
report_power -file $outputDir/post_route_power.rpt
report_drc -file $outputDir/imp_drc.rpt

# STEP 5: Generate outputs
write_bitstream -force $outputDir/top.bit
write_verilog -force $outputDir/impl_netlist.v
write_xdc -no_fixed_only -force $outputDir/impl.xdc
```

### Template C: IP Integrator Block Design
```tcl
# Create project with board
create_project my_bd_project ./my_bd_project -part xc7z020clg484-1
set_property board_part xilinx.com:zc702:part0:1.4 [current_project]

# Create block design
create_bd_design "system"

# Add IP
create_bd_cell -type ip -vlnv xilinx.com:ip:processing_system7:5.5 ps7_0
create_bd_cell -type ip -vlnv xilinx.com:ip:axi_gpio:2.0 axi_gpio_0

# Run automation
apply_bd_automation -rule xilinx.com:bd_rule:processing_system7 \
    -config {make_external "FIXED_IO, DDR"} [get_bd_cells ps7_0]
apply_bd_automation -rule xilinx.com:bd_rule:axi4 \
    -config {Master "/ps7_0/M_AXI_GP0"} [get_bd_intf_pins axi_gpio_0/S_AXI]

# Assign addresses
assign_bd_address

# Validate and save
validate_bd_design
save_bd_design

# Generate wrapper and build
make_wrapper -files [get_files system.bd] -top
add_files -norecurse ./my_bd_project/my_bd_project.gen/sources_1/bd/system/hdl/system_wrapper.v

# Synthesize and implement
launch_runs synth_1
wait_on_run synth_1
launch_runs impl_1 -to_step write_bitstream
wait_on_run impl_1

# Export hardware for Vitis
write_hw_platform -fixed -include_bit -force ./system_wrapper.xsa
```

### Template D: Hardware Programming
```tcl
# Open hardware manager
open_hw_manager

# Connect to hardware server
connect_hw_server -url localhost:3121
open_hw_target

# Get device
current_hw_device [get_hw_devices xc7k70t_0]
refresh_hw_device [current_hw_device]

# Program device
set_property PROGRAM.FILE {./output/top.bit} [current_hw_device]
set_property PROBES.FILE {./output/top.ltx} [current_hw_device]
program_hw_devices [current_hw_device]

# Read ILA data
refresh_hw_device [current_hw_device]
set ila [get_hw_ilas -of_objects [current_hw_device]]
set_property TRIGGER_COMPARE_VALUE eq1'b1 \
    [get_hw_probes trigger_signal -of_objects $ila]
run_hw_ila $ila
wait_on_hw_ila $ila
display_hw_ila_data [upload_hw_ila_data $ila]

# Disconnect
close_hw_target
disconnect_hw_server
close_hw_manager
```

### Template E: Debug Core Insertion
```tcl
# After synthesis, before implementation
open_run synth_1

# Create ILA core
create_debug_core u_ila_0 ila
set_property C_DATA_DEPTH 1024 [get_debug_cores u_ila_0]
set_property C_TRIGIN_EN false [get_debug_cores u_ila_0]
set_property C_INPUT_PIPE_STAGES 0 [get_debug_cores u_ila_0]

# Connect clock
set_property port_width 1 [get_debug_ports u_ila_0/clk]
connect_debug_port u_ila_0/clk [get_nets clk]

# Add probe signals
set_property port_width 8 [get_debug_ports u_ila_0/probe0]
connect_debug_port u_ila_0/probe0 [get_nets {data[0] data[1] data[2] data[3] data[4] data[5] data[6] data[7]}]

# Create additional probe
create_debug_port u_ila_0 probe
set_property port_width 1 [get_debug_ports u_ila_0/probe1]
connect_debug_port u_ila_0/probe1 [get_nets valid]

# Implement debug core
implement_debug_core

# Write debug probes file
write_debug_probes -force ./output/top.ltx

# Continue to implementation
launch_runs impl_1 -to_step write_bitstream
wait_on_run impl_1
```

---

## 6. Document Reference Map

| Document | Content | When to consult |
|----------|---------|----------------|
| UG835 | TCL Command Reference (all commands, syntax, examples) | Command details, parameters, examples |
| UG892 | Design Flows Overview | Flow architecture, Project vs Non-Project mode |
| UG893 | Vivado IDE | GUI features, Flow Navigator |
| UG894 | Using Tcl Scripting | Tcl programming patterns, advanced scripting |
| UG895 | System-Level Design Entry | RTL entry, source management |
| UG896 | Designing with IP | IP configuration, XCI files, IP catalog |
| UG899 | I/O and Clock Planning | Pin assignment, I/O standards |
| UG900 | Logic Simulation | Vivado simulator, waveform analysis |
| UG901 | Synthesis | Synthesis strategies, attributes, coding guidelines |
| UG903 | Using Constraints | XDC/SDC constraint syntax, timing constraints |
| UG904 | Implementation | Opt/Place/Route strategies, ECO flow |
| UG906 | Design Analysis & Closure | Timing closure, analysis techniques |
| UG908 | Programming & Debugging | Hardware Manager, ILA, VIO, JTAG |
| UG994 | IP Subsystems (IP Integrator) | Block design, AXI interconnect |
| UG1399 | Vitis HLS (Chinese) | C-to-RTL synthesis |
