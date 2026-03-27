# Vivado Simulation Complete Reference

Full command syntax, property tables, and advanced flows from UG900 (v2025.2).

## 1. xsim Three-Step Flow — Complete Syntax

### xvlog (Verilog/SystemVerilog Compiler)
```tcl
xvlog [-d <macro>[=<value>]]       ;# Define macro
      [-f <file>]                  ;# Read options from file
      [-i <include_path>]          ;# Include search path
      [-L <library>]               ;# Target library (default: work)
      [--include <path>]           ;# Additional include path
      [--define <macro>=<value>]   ;# Define macro (long form)
      [-sv]                        ;# Enable SystemVerilog
      [-v <library_file>]          ;# Verilog library file
      [-y <library_dir>]           ;# Verilog library directory
      [--relax]                    ;# Relax strict language checks
      [-log <file>]                ;# Log file
      [-nolog]                     ;# Suppress log file
      [-work <library>]            ;# Work library name
      [--nosignalhandlers]         ;# Disable signal handlers
      <source_files>
```

### xvhdl (VHDL Compiler)
```tcl
xvhdl [-f <file>]                  ;# Read options from file
      [-L <library>]               ;# Target library
      [-93 | -2008]                ;# VHDL standard (1993 or 2008)
      [--relax]                    ;# Relax strict checks
      [-log <file>]                ;# Log file
      [-nolog]                     ;# Suppress log
      [-work <library>]            ;# Work library name
      <source_files>
```

### xelab (Elaborator/Linker)
```tcl
xelab [-d <macro>[=<value>]]       ;# Verilog define
      [-debug <level>]             ;# off | line | typical | all
      [-f <file>]                  ;# Options file
      [-generic_top <param=val>]   ;# Override VHDL generics
      [-i <include_path>]          ;# Include path
      [-L <library>]               ;# Library search order
      [-log <file>]                ;# Log file
      [-mt <N>]                    ;# Multi-threading: auto | off | 2 | 4 | 8
      [-nolog]                     ;# Suppress log
      [-O0 | -O1 | -O2 | -O3]    ;# Optimization level
      [-override_timeunit]         ;# Override with -timescale
      [-prj <file>]                ;# Project file
      [-pulse_e <value>]           ;# Pulse error limit (timing sim)
      [-pulse_r <value>]           ;# Pulse reject limit (timing sim)
      [-R]                         ;# Run immediately after elaboration
      [-rangecheck]                ;# Enable VHDL range checking
      [-relax]                     ;# Relax strict checks
      [-s <snapshot_name>]         ;# Output snapshot name
      [-sdfroot <instance>]        ;# SDF annotation root
      [-timescale <ts>]            ;# Default timescale (e.g., 1ns/1ps)
      [-transport_int_delays]      ;# Transport interconnect delays
      [-maxdelay | -mindelay | -typdelay]  ;# SDF delay type
      [-stat]                      ;# Print elaboration statistics
      <design_units> [glbl]
```

### xsim (Simulator)
```tcl
xsim <snapshot_name>
     [-f <file>]                   ;# Options file
     [-gui]                        ;# Launch GUI
     [-key <file>]                 ;# Keyfile for waveform config
     [-log <file>]                 ;# Log file
     [-nolog]                      ;# Suppress log
     [-maxdeltaid <N>]             ;# Max delta cycle iterations
     [-onerror <stop|quit>]        ;# Error behavior
     [-R | -runall]                ;# Run all (until $finish)
     [-stats]                      ;# Print simulation statistics
     [-sv_seed <value>]            ;# SystemVerilog random seed
     [-t <tcl_file>]               ;# TCL batch script
     [-testplusarg <arg>]          ;# $test$plusargs / $value$plusargs
     [-tl]                         ;# Enable Tcl line editing
     [-tp]                         ;# Enable Tcl profiling
     [-view <wdb_file>]            ;# Open waveform database
     [-wdb <wdb_file>]             ;# Output waveform database file
```

---

## 2. launch_simulation Complete Syntax

```tcl
launch_simulation
    [-mode <behavioral | post-synthesis | post-implementation>]
    [-type <functional | timing>]
    [-scripts_only]                ;# Generate scripts without running
    [-absolute_path]               ;# Use absolute paths in scripts
    [-install_path <path>]         ;# Simulator install path override
    [-quiet] [-verbose]
```

**Default:** `-mode behavioral`

**Valid combinations:**
| Mode | Type | Description |
|------|------|-------------|
| behavioral | (N/A) | RTL simulation |
| post-synthesis | functional | Synth netlist, no timing |
| post-synthesis | timing | Synth netlist + SDF (rare) |
| post-implementation | functional | Impl netlist, no timing |
| post-implementation | timing | Impl netlist + SDF (most common timing sim) |

---

## 3. Simulation Runtime Commands

### run / control
```tcl
run <time><unit>                   ;# Run for time (e.g., run 100ns)
run -all                           ;# Run until $finish or breakpoint
restart                            ;# Reset to time 0
step                               ;# Step one statement
step <N>                           ;# Step N statements
current_time                       ;# Return current sim time
```

### Signal inspection
```tcl
get_objects [-r] [<pattern>]       ;# List objects (hierarchical with -r)
get_value <signal>                 ;# Read signal value
set_value <signal> <value>         ;# Set signal value (non-persistent)
report_values [-all]               ;# Report signal values
```

### add_force (signal forcing)
```tcl
add_force <signal> <value>
    [-radix <bin|hex|dec|oct|unsigned|ascii>]
    [-repeat_every <time>]
    [-cancel_after <time>]
    {<value> [<time_offset>]}...   ;# Value sequence

# Examples:
add_force clk {0} {1 5ns} -repeat_every 10ns     ;# Clock
add_force rst {1} {0 100ns}                        ;# Reset pulse
add_force -radix hex data 0xAB                     ;# Hex value
remove_forces <signal>                              ;# Remove forces
```

### Waveform commands
```tcl
add_wave <signal>                  ;# Add to waveform viewer
add_wave -r /                      ;# Add all signals recursively
log_wave -r /                      ;# Log all signals to WDB
log_wave <signal>                  ;# Log specific signal
```

### Breakpoints
```tcl
add_bp <file> <line>               ;# Breakpoint at source line
add_bp -condition {<expr>}         ;# Conditional breakpoint
remove_bp <id>                     ;# Remove breakpoint
remove_bp -all                     ;# Remove all breakpoints
report_bps                         ;# List breakpoints
```

### SAIF/VCD commands
```tcl
# SAIF (Switching Activity Interchange Format)
open_saif <file>                   ;# Open SAIF file for writing
log_saif [get_objects -r *]        ;# Log switching activity
log_saif <signal_list>             ;# Log specific signals
close_saif                         ;# Close SAIF file

# VCD (Value Change Dump)
open_vcd <file>                    ;# Open VCD file for writing
log_vcd [get_objects -r *]         ;# Log all value changes
log_vcd <signal_list>              ;# Log specific signals
close_vcd                          ;# Close VCD file
```

---

## 4. Netlist Generation Commands

### write_verilog
```tcl
write_verilog [-mode <mode>]       ;# funcsim | timesim | design | synth_stub | pin_planning
              [-cell <cell>]       ;# Write for specific cell only
              [-sdf_anno <bool>]   ;# Include SDF annotation (timesim)
              [-sdf_file <file>]   ;# Explicit SDF file path in annotation
              [-rename_top <name>] ;# Rename top module
              [-process_corner <fast|slow>]  ;# Process corner
              [-force]             ;# Overwrite existing file
              [-include_xilinx_libs]  ;# Include Xilinx library modules
              [-write_all_overrides]  ;# Write all parameter overrides
              <file>
```

### write_vhdl
```tcl
write_vhdl [-mode <mode>]         ;# funcsim | design | synth_stub
           [-cell <cell>]         ;# Write for specific cell
           [-rename_top <name>]   ;# Rename top entity
           [-force]               ;# Overwrite existing
           [-arch_only]           ;# Architecture only (no entity)
           <file>
```

### write_sdf
```tcl
write_sdf [-cell <cell>]          ;# Write for specific cell
          [-rename_top <name>]    ;# Rename top instance
          [-process_corner <fast|slow>]  ;# Process corner (default: slow)
          [-force]                ;# Overwrite existing
          <file>
```

---

## 5. compile_simlib Complete Syntax

```tcl
compile_simlib
    [-simulator <questa|modelsim|vcs|xcelium|riviera|activehdl>]
    [-simulator_exec_path <path>]  ;# Simulator executable path
    [-family <all|artix7|kintex7|virtex7|zynq|kintexu|virtexu|...>]
    [-language <all|verilog|vhdl>]
    [-library <lib_list>]          ;# Specific libraries to compile
    [-directory <path>]             ;# Output directory
    [-force]                       ;# Recompile all
    [-no_ip_compile]               ;# Skip IP library compilation
    [-32bit]                       ;# 32-bit compilation
    [-gcc_install_dir <path>]      ;# GCC install for SystemC
    [-verbose]
    [-quiet]
```

**Typical usage:**
```tcl
compile_simlib -simulator questa \
    -directory /tools/sim_libs/questa \
    -family all -language all -force
```

---

## 6. export_simulation Complete Syntax

```tcl
export_simulation
    [-simulator <questa|modelsim|vcs|xcelium|riviera|activehdl|xsim>]
    [-directory <path>]            ;# Output directory
    [-of_objects <filesets>]        ;# Specific filesets
    [-ip_user_files_dir <path>]    ;# IP user files directory
    [-ipstatic_source_dir <path>]  ;# IP static source directory
    [-lib_map_path <path>]         ;# Compiled library path
    [-use_ip_compiled_libs]        ;# Use pre-compiled IP libs
    [-force]                       ;# Overwrite existing
    [-generate_hier_access]        ;# Generate hierarchy access
    [-absolute_path]               ;# Use absolute paths
    [-quiet] [-verbose]
```

---

## 7. Simulation Properties Complete List

### xsim Properties (set on simulation fileset)

**Compilation:**
| Property | Values | Default |
|----------|--------|---------|
| `xsim.compile.xvlog.more_options` | string | "" |
| `xsim.compile.xvhdl.more_options` | string | "" |
| `xsim.compile.xvlog.nosort` | bool | false |

**Elaboration:**
| Property | Values | Default |
|----------|--------|---------|
| `xsim.elaborate.debug_level` | off, typical, all | typical |
| `xsim.elaborate.xelab.more_options` | string | "" |
| `xsim.elaborate.mt_level` | auto, off, 2, 4, 8 | auto |
| `xsim.elaborate.rangecheck` | bool | false |
| `xsim.elaborate.load_glbl` | bool | true |

**Simulation:**
| Property | Values | Default |
|----------|--------|---------|
| `xsim.simulate.runtime` | time string | 1000ns |
| `xsim.simulate.log_all_signals` | bool | false |
| `xsim.simulate.wdb` | filename | "" |
| `xsim.simulate.saif` | filename | "" |
| `xsim.simulate.saif_scope` | string | "" |
| `xsim.simulate.saif_all_signals` | bool | false |
| `xsim.simulate.xsim.more_options` | string | "" |
| `xsim.simulate.tcl.post` | filename | "" |
| `xsim.simulate.custom_tcl` | filename | "" |

### Third-Party Simulator Properties

**Questa/ModelSim:**
| Property | Description |
|----------|-------------|
| `questa.compile.vlog.more_options` | Additional vlog options |
| `questa.compile.vcom.more_options` | Additional vcom options |
| `questa.elaborate.vsim.more_options` | Additional vsim elab options |
| `questa.simulate.vsim.more_options` | Additional vsim sim options |
| `questa.simulate.runtime` | Simulation runtime |
| `questa.simulate.log_all_signals` | Log all signals |

**VCS:**
| Property | Description |
|----------|-------------|
| `vcs.compile.vlogan.more_options` | Additional vlogan options |
| `vcs.compile.vhdlan.more_options` | Additional vhdlan options |
| `vcs.elaborate.vcs.more_options` | Additional VCS elab options |
| `vcs.simulate.vcs.more_options` | Additional VCS sim options |
| `vcs.simulate.runtime` | Simulation runtime |

**Xcelium:**
| Property | Description |
|----------|-------------|
| `xcelium.compile.xmvlog.more_options` | Additional xmvlog options |
| `xcelium.compile.xmvhdl.more_options` | Additional xmvhdl options |
| `xcelium.elaborate.xmelab.more_options` | Additional xmelab options |
| `xcelium.simulate.xmsim.more_options` | Additional xmsim options |
| `xcelium.simulate.runtime` | Simulation runtime |

---

## 8. Vivado Simulator Language Support

| Language | Standard | Notes |
|----------|----------|-------|
| Verilog | IEEE 1364-2001 | Full support |
| Verilog | IEEE 1364-2005 | Full support |
| SystemVerilog | IEEE 1800-2012 | Full support |
| SystemVerilog | IEEE 1800-2017 | Partial support |
| VHDL | IEEE 1076-1993 | Full support |
| VHDL | IEEE 1076-2008 | Full support |
| Mixed Language | Verilog/SV + VHDL | Supported via port mapping |

### Encryption Support
| Standard | Description |
|----------|-------------|
| IEEE 1735-2014 | IP encryption (V2 recommended) |
| Xilinx proprietary | Legacy encryption |

### SystemVerilog Coverage (xsim)
- Assertions: `assert`, `assume`, `cover` (immediate and concurrent)
- Functional coverage: `covergroup`, `coverpoint`, `cross`
- Constrained random: `rand`, `randc`, `constraint`
- Classes, interfaces, packages: Full support
- DPI-C: SystemVerilog to C/C++ interface

---

## 9. Timing Simulation Checklist

```
1. Generate netlist:
   write_verilog -mode timesim -sdf_anno true -force <file>.v
   write_sdf -force <file>.sdf

2. Compile:
   xvlog <file>.v $XILINX_VIVADO/data/verilog/src/glbl.v
   xvlog tb.v

3. Elaborate:
   xelab tb_top glbl -debug typical \
       -transport_int_delays -pulse_r 0 -pulse_e 0 \
       -s timing_snap

4. Simulate:
   xsim timing_snap -runall

5. Common issues:
   - Missing glbl.v → "undefined module: glbl"
   - Wrong SDF root → timing not annotated
   - Missing -transport_int_delays → incorrect pulse behavior
   - Testbench timing too tight → setup/hold violations
```
