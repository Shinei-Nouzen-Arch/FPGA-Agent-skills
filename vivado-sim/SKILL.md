---
name: vivado-sim
description: Use this skill when the user needs help with Vivado simulation strategy, flow selection, and debugging. This includes behavioral simulation (RTL functional verification), post-synthesis simulation (netlist functional verification), post-implementation timing simulation (SDF back-annotation, timing verification), Vivado Simulator xsim usage (xvlog/xvhdl/xelab/xsim three-step flow, launch_simulation Project Mode), third-party simulator integration (Questa/ModelSim/VCS/Xcelium/Riviera/ActiveHDL, compile_simlib, export_simulation), SAIF/VCD power simulation (open_saif/log_saif, open_vcd/log_vcd, read_saif + report_power), simulation netlist generation (write_verilog -mode funcsim/timesim, write_sdf), xsim debugging commands (add_force, add_wave, log_wave, add_bp, step, run, restart, get_objects, get_value), simulation properties and settings, glbl.v usage, or any question about choosing the right simulation approach for verification or power analysis.
---

# Vivado Simulation Decision Guide

Based on UG900 (v2025.2). For complete command syntax and property tables, see REFERENCE.md.

## Simulation Flow Overview

```
RTL + Testbench ──→ Behavioral Sim (功能验证, 最快)
       ↓ synth_design
综合网表 ──→ Post-Synth Sim (综合正确性验证, 可选)
       ↓ place_design + route_design
实现网表 + SDF ──→ Timing Sim (时序验证, 最慢)
       ↓
SAIF/VCD ──→ report_power (功耗分析)
```

### Three Simulation Stages

| Stage | Input | Purpose | Speed | Accuracy |
|-------|-------|---------|-------|----------|
| Behavioral (RTL) | RTL source + TB | Functional verification | Fastest | No timing |
| Post-Synthesis | Synth netlist | Verify synthesis correctness | Medium | Functional only |
| Post-Implementation | Impl netlist + SDF | Timing verification | Slowest | Full timing |

### When to Use Each Stage

- **Daily development** → Behavioral sim only
- **Functional mismatch after synthesis** → Post-synthesis sim to isolate
- **Timing closure verification** → Post-implementation timing sim
- **Power estimation** → Any stage + SAIF dump → `report_power`

---

## Vivado Simulator (xsim) — Project Mode

Recommended for interactive development with Vivado GUI/project.

```tcl
# Configure simulation settings
set_property -name {xsim.simulate.runtime} -value {1000ns} -objects [get_filesets sim_1]
set_property -name {xsim.simulate.log_all_signals} -value {true} -objects [get_filesets sim_1]
set_property -name {xsim.elaborate.debug_level} -value {typical} -objects [get_filesets sim_1]

# Launch behavioral simulation
launch_simulation
run 100ns
close_sim
```

### Simulation Modes in Project Mode
```tcl
launch_simulation -mode behavioral          ;# RTL sim (default)
launch_simulation -mode post-synthesis -type functional   ;# Post-synth functional
launch_simulation -mode post-synthesis -type timing       ;# Post-synth timing
launch_simulation -mode post-implementation -type functional  ;# Post-impl functional
launch_simulation -mode post-implementation -type timing      ;# Post-impl timing
```

### Generate Scripts Without Running
```tcl
launch_simulation -scripts_only             ;# Generate sim scripts only
launch_simulation -scripts_only -absolute_path  ;# With absolute paths
```

---

## Vivado Simulator (xsim) — Non-Project Mode (Scripted)

Three-step flow: **Compile → Elaborate → Simulate**

```tcl
# Step 1: Compile
exec xvlog design.v tb.v                    ;# Verilog
exec xvlog -sv design.sv                    ;# SystemVerilog
exec xvhdl design.vhd                       ;# VHDL
exec xvhdl -2008 design.vhd                 ;# VHDL-2008

# Step 2: Elaborate (link)
exec xelab -debug typical tb_top -s sim_snap

# Step 3: Simulate
exec xsim sim_snap -runall
exec xsim sim_snap -t run.tcl               ;# With TCL script
exec xsim sim_snap -gui                     ;# With GUI
```

### xelab Debug Level Decision

| Scenario | Option | Effect |
|----------|--------|--------|
| Waveform + breakpoints | `-debug typical` | Standard debugging (recommended) |
| All signals observable | `-debug all` | Full visibility, slower elaboration |
| Fastest simulation | `-debug off` | No debug, maximum speed |
| Line stepping | `-debug line` | Source-level stepping |

### xelab Key Options

| Option | Purpose |
|--------|---------|
| `-d MACRO=value` | Verilog \`define |
| `-i <path>` | Verilog include search path |
| `-L <library>` | Library search order |
| `-s <snapshot>` | Output snapshot name |
| `-timescale 1ns/1ps` | Default timescale |
| `-mt <N>` | Multi-threading (2, 4, 8, off) |
| `-transport_int_delays` | Transport delay mode (timing sim) |
| `-pulse_r 0 -pulse_e 0` | Pulse rejection/error (timing sim) |
| `-sdfroot <instance>` | SDF annotation root instance |
| `-generic_top <param=val>` | Override top-level generics (VHDL) |
| `-override_timeunit` | Override module timeunit with -timescale |

---

## xsim Runtime / Debug Commands

| Command | Description |
|---------|-------------|
| `run <time>` | Run simulation for specified time |
| `run -all` | Run until `$finish` or breakpoint |
| `restart` | Reset simulation to time 0 |
| `step` / `step <N>` | Single-step (N statements) |
| `current_time` | Return current simulation time |
| `add_force <signal> <value> [<time> <value>...]` | Force signal value (supports sequences) |
| `remove_forces <signal>` | Remove forced values |
| `add_wave <signal>` | Add signal to waveform viewer |
| `log_wave -r /` | Log all signals to WDB (waveform database) |
| `add_bp <file> <line>` | Add breakpoint at source line |
| `add_bp -condition {<expr>}` | Conditional breakpoint |
| `remove_bp <id>` | Remove breakpoint |
| `get_objects -r *` | List all objects (hierarchical) |
| `get_value <signal>` | Read current signal value |
| `set_value <signal> <value>` | Set signal value (non-persistent) |
| `report_values` | Report all signal values |

### add_force Examples
```tcl
# Constant force
add_force clk 0
# Clock pattern: 0 at 0ns, 1 at 5ns, repeat every 10ns
add_force clk {0} {1 5ns} -repeat_every 10ns
# Reset pulse: 1 at 0ns, 0 at 100ns
add_force rst {1} {0 100ns}
# With radix
add_force -radix hex data_in 0xFF
```

---

## Timing Simulation (Post-Implementation)

### Netlist Generation
```tcl
# Post-synthesis functional netlist
open_checkpoint post_synth.dcp
write_verilog -mode funcsim -force post_synth_func.v

# Post-implementation timing netlist + SDF
open_checkpoint post_route.dcp
write_verilog -mode timesim -sdf_anno true -force post_impl_timing.v
write_sdf -force post_impl_timing.sdf

# VHDL variants
write_vhdl -mode funcsim -force post_synth_func.vhd
```

### write_verilog -mode Options

| Mode | Purpose |
|------|---------|
| `funcsim` | Functional simulation netlist (no timing) |
| `timesim` | Timing simulation netlist (with SDF annotation) |
| `design` | Design netlist (non-simulation) |
| `synth_stub` | Synthesis stub for OOC |
| `pin_planning` | I/O pin planning |

### SDF Back-Annotation

xelab handles SDF automatically when `-sdf_anno true` is used in `write_verilog`:
```tcl
# xelab with timing simulation netlist
exec xelab -debug typical tb_top glbl \
    -transport_int_delays \
    -pulse_r 0 -pulse_e 0 \
    -s timing_sim_snap
```

For manual SDF specification:
```tcl
exec xelab -debug typical tb_top glbl \
    -sdfroot /tb_top/uut \
    -transport_int_delays \
    -pulse_r 0 -pulse_e 0 \
    -s timing_sim_snap
```

### Process Corner Selection
```tcl
write_sdf -process_corner fast -force fast_corner.sdf   ;# Best case
write_sdf -process_corner slow -force slow_corner.sdf   ;# Worst case (default)
```

### glbl.v Requirements

| Stage | glbl.v Needed? | Reason |
|-------|---------------|--------|
| Behavioral | Only if using MMCM/PLL/GT primitives | Global set/reset signals |
| Post-Synthesis | Yes | Netlist references global signals |
| Timing Sim | Required | SDF annotation depends on it |

Path: `$XILINX_VIVADO/data/verilog/src/glbl.v`

```tcl
# Include glbl.v in compilation and elaboration
exec xvlog $::env(XILINX_VIVADO)/data/verilog/src/glbl.v
exec xelab tb_top glbl -debug typical -s sim_snap
```

---

## Third-Party Simulator Integration

### Supported Simulators

| Simulator | `-simulator` value | Vendor |
|-----------|-------------------|--------|
| Questa Advanced Simulator | `questa` | Siemens EDA |
| ModelSim | `modelsim` | Siemens EDA |
| VCS | `vcs` | Synopsys |
| Xcelium | `xcelium` | Cadence |
| Riviera-PRO | `riviera` | Aldec |
| Active-HDL | `activehdl` | Aldec |

### Step 1: Compile Simulation Libraries (One-Time)
```tcl
compile_simlib -simulator questa -directory /path/to/compiled_libs \
    -family all -language all
```

### Step 2: Set Target Simulator
```tcl
set_property target_simulator Questa [current_project]
# Or: ModelSim, VCS, Xcelium, Riviera, ActiveHDL
```

### Step 3: Export Simulation Scripts
```tcl
# Option A: export_simulation (flexible, supports Non-Project)
export_simulation -simulator questa -directory ./sim_scripts \
    -use_ip_compiled_libs

# Option B: launch_simulation -scripts_only (Project Mode only)
launch_simulation -scripts_only
```

### export_simulation vs launch_simulation -scripts_only

| Feature | export_simulation | launch_simulation -scripts_only |
|---------|-------------------|-------------------------------|
| Custom output dir | Yes | Project default |
| Non-Project Mode | Yes | No |
| IP compiled libs | `-use_ip_compiled_libs` | Automatic |
| All sim types | Yes | Current mode only |

---

## SAIF/VCD Power Simulation

### SAIF Collection (Recommended for report_power)
```tcl
# During simulation (xsim)
open_saif /path/to/output.saif
log_saif [get_objects -r *]       ;# Or specific hierarchy
run 1000ns
close_saif

# Power analysis
open_checkpoint post_route.dcp
read_saif /path/to/output.saif
report_power -file power.rpt
```

### VCD Collection
```tcl
open_vcd /path/to/output.vcd
log_vcd [get_objects -r *]
run 1000ns
close_vcd
```

### SAIF vs VCD

| Feature | SAIF | VCD |
|---------|------|-----|
| File size | Small (statistical summary) | Large (full waveform) |
| report_power support | Direct `read_saif` | Needs conversion |
| Best for | Vivado power analysis | Third-party tools / waveform viewing |
| Recommended | Yes (for power) | When waveform needed |

---

## Simulation Properties Quick Reference

```tcl
# Runtime
set_property -name {xsim.simulate.runtime} -value {1000ns} -objects [get_filesets sim_1]

# Log all signals
set_property -name {xsim.simulate.log_all_signals} -value {true} -objects [get_filesets sim_1]

# Debug level: typical | all | off
set_property -name {xsim.elaborate.debug_level} -value {typical} -objects [get_filesets sim_1]

# Additional xsim options
set_property -name {xsim.simulate.xsim.more_options} -value {-testplusarg FAST} -objects [get_filesets sim_1]

# Additional xelab options
set_property -name {xsim.elaborate.xelab.more_options} -value {-mt 4} -objects [get_filesets sim_1]

# Waveform database file
set_property -name {xsim.simulate.wdb} -value {my_sim.wdb} -objects [get_filesets sim_1]

# Target simulator
set_property target_simulator Questa [current_project]
```

### Simulation Fileset Management
```tcl
# Add simulation source
add_files -fileset sim_1 tb_top.v

# Set top module
set_property top tb_top [get_filesets sim_1]

# Set top library
set_property top_lib work [get_filesets sim_1]

# Create additional sim fileset
create_fileset -simset sim_2
```

### Language Support

| Language | Standard | xsim Support |
|----------|----------|-------------|
| Verilog | IEEE 1364-2001/2005 | Full |
| SystemVerilog | IEEE 1800-2012 (partial 2017) | Full |
| VHDL | IEEE 1076-1993/2008 | Full |
| Mixed Language | Verilog/SV + VHDL | Supported |
