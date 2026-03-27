---
name: vivado-constraints
description: Use this skill when the user needs help writing XDC/SDC timing or physical constraints for Vivado FPGA designs. This includes clock definitions (create_clock, create_generated_clock, virtual clocks), I/O delay constraints (set_input_delay, set_output_delay, DDR timing), timing exceptions (set_false_path, set_multicycle_path, set_max_delay, set_min_delay), clock domain crossing (CDC) constraints (set_clock_groups, set_bus_skew), clock uncertainty/jitter/latency, physical constraints (IOSTANDARD, PACKAGE_PIN, LOC, Pblock, placement, routing), XDC precedence rules, constraint scoping (SCOPED_TO_REF), constraint ordering optimization, or constraint debugging (check_timing, report_exceptions, report_clock_interaction). Trigger when the user mentions XDC, SDC, timing constraints, clock constraints, IO delay, false path, multicycle path, clock groups, or physical pin assignment. For timing report interpretation and analysis use vivado-analysis.
---

# Vivado XDC Constraints Decision Guide

Based on UG903 (v2025.2). For complete syntax examples, see REFERENCE.md.

## XDC File Management

### Synthesis vs Implementation Constraints
```tcl
# Project Mode: set on XDC file objects
set_property USED_IN_SYNTHESIS TRUE [get_files timing.xdc]
set_property USED_IN_IMPLEMENTATION TRUE [get_files timing.xdc]
set_property USED_IN_SYNTHESIS FALSE [get_files physical.xdc]  ;# impl only

# Non-Project Mode: just read in appropriate order
read_xdc timing.xdc        ;# both synth and impl
read_xdc physical.xdc      ;# read after synth only
```

### Constraint Scoping (for IP / sub-modules)
```tcl
# Project Mode
set_property SCOPED_TO_REF uart_tx_ctl [get_files uart_tx_ctl.xdc]
set_property SCOPED_TO_CELLS uart_tx_i0/uart_tx_ctl_i0 [get_files uart_tx_ctl.xdc]

# Non-Project Mode
read_xdc -ref uart_tx_ctl uart_tx_ctl.xdc
read_xdc -cells uart_tx_i0/uart_tx_ctl_i0 uart_tx_ctl.xdc
```

### Object Naming Rules
- Single-bit register `myReg` → instance name: `myReg_reg`
- Multi-bit register `myBus[2:0]` → `myBus_reg[0]`, `myBus_reg[1]`, `myBus_reg[2]`
- Query multi-bit: `get_cells myBus_reg[*]` (NOT `myBus_reg[2:0]`)
- Hierarchical names: use explicit `/` separator, NOT wildcards with `-hierarchical`
- **Recommended:** Use `get_cells inst_A/inst_B/*_reg` without `-hierarchical`

---

## Clock Definition Guide

### Primary Clocks
```tcl
# Board clock on input port (RECOMMENDED)
create_clock -period 10 [get_ports sysclk]

# Named clock with custom waveform (25% duty cycle, 90° phase shift)
create_clock -name devclk -period 10 -waveform {2.5 5} [get_ports ClkIn]

# Differential clock — define on POSITIVE pin ONLY
create_clock -name sysclk -period 3.33 [get_ports SYS_CLK_clk_p]

# GT recovered clock
create_clock -name rxclk -period 3.33 [get_pins gt0/RXOUTCLK]

# Virtual clock (no netlist object — for I/O delay reference)
create_clock -name clk_virt -period 10
```

**Rules:**
- Define primary clocks on input ports, NOT on BUFG outputs
- Primary clocks must be defined first — other constraints reference them
- Virtual clocks must be defined before set_input_delay/set_output_delay that use them

### Generated Clocks
```tcl
# MMCM/PLL outputs → AUTO-DERIVED, no manual constraint needed
# User logic divider → must define manually:
create_generated_clock -name clkdiv2 -source [get_ports clkin] -divide_by 2 [get_pins REGA/Q]

# Using -edges (edge indices of master clock)
create_generated_clock -name clkdiv2 -source [get_pins REGA/C] -edges {1 3 5} [get_pins REGA/Q]

# Duty cycle change + phase shift via -edges and -edge_shift
create_generated_clock -name clkshift -source [get_pins mmcm0/CLKIN] \
    -edges {1 2 3} -edge_shift {2.5 0 2.5} [get_pins mmcm0/CLKOUT]

# Multiply + divide (for MMCM manual definition)
create_generated_clock -name clk43 -source [get_pins mmcm0/CLKIN] \
    -multiply_by 4 -divide_by 3 [get_pins mmcm0/CLKOUT]

# Combinational path only (MUX output)
create_generated_clock -name clkout -source [get_pins mmcm0/CLKIN] \
    -combinational [get_pins MUX/O]

# Rename auto-derived clock (name + source_object only)
create_generated_clock -name clk_rx [get_pins clk_gen_i0/clk_core_i0/inst/mmcm_adv_inst/CLKOUT0]
```

**Rules:**
- `-source` accepts pin/port ONLY, not clock objects
- MMCM/PLL outputs are auto-derived — only define manually if you need custom settings
- Auto-derived clocks can only be renamed at CMB output pins
- Use `get_clocks -of_objects [get_pins <pin>]` to query auto-derived clock names

### Clock Groups
```tcl
# Asynchronous clocks (most common — separate oscillators)
set_clock_groups -name async_clk0_clk1 -asynchronous \
    -group {clk0 usrclk itfclk} -group {clk1 gtclkrx gtclktx}

# With -include_generated_clocks (auto-include derived clocks)
set_clock_groups -name async_grp -asynchronous \
    -group [get_clocks -include_generated_clocks clk0] \
    -group [get_clocks -include_generated_clocks clk1]

# Exclusive clocks (BUFGMUX — only one active at a time)
set_clock_groups -name exclusive_clk0_clk1 -physically_exclusive \
    -group clk0 -group clk1
```

**Rules:**
- `set_clock_groups` has **HIGHEST priority** among timing exceptions
- Cannot be overridden by `-reset_path`
- Prefer `set_clock_groups` over two `set_false_path` for async CDC
- `-logically_exclusive` and `-physically_exclusive` are equivalent for AMD FPGAs

---

## Clock Latency, Jitter, Uncertainty

```tcl
# Source latency (board-level delay, outside FPGA)
set_clock_latency -source -early 0.2 [get_clocks sysClk]
set_clock_latency -source -late 0.5 [get_clocks sysClk]

# Input jitter (primary clocks only, per-clock)
set_input_jitter [get_clocks -of_objects [get_ports clkin]] 0.1

# System jitter (global, all clocks)
set_system_jitter 0.05

# Additional clock uncertainty (extra timing margin)
set_clock_uncertainty 0.5 [get_clocks clk1]

# Inter-clock uncertainty (MUST define BOTH directions)
set_clock_uncertainty 2.0 -from [get_clocks clk1] -to [get_clocks clk2]
set_clock_uncertainty 2.0 -from [get_clocks clk2] -to [get_clocks clk1]
```

---

## I/O Delay Templates

### SDR Input
```tcl
# Basic (both min and max)
set_input_delay -clock sysClk 2 [get_ports DIN]

# Separate min/max
set_input_delay -clock sysClk -max 4 [get_ports DIN]
set_input_delay -clock sysClk -min 1 [get_ports DIN]

# Relative to virtual clock
create_clock -name clk_port_virt -period 10
set_input_delay -clock clk_port_virt 2 [get_ports DIN]
```

### DDR Input
```tcl
create_clock -name clk_ddr -period 6 [get_ports DDR_CLK_IN]
set_input_delay -clock clk_ddr -max 2.1 [get_ports DDR_IN]
set_input_delay -clock clk_ddr -max 1.9 [get_ports DDR_IN] -clock_fall -add_delay
set_input_delay -clock clk_ddr -min 0.9 [get_ports DDR_IN]
set_input_delay -clock clk_ddr -min 1.1 [get_ports DDR_IN] -clock_fall -add_delay
```

### SDR Output
```tcl
set_output_delay -clock sysClk 6 [get_ports DOUT]

# Separate min/max
set_output_delay -clock sysClk -max 6 [get_ports DOUT]
set_output_delay -clock sysClk -min 1 [get_ports DOUT]
```

### DDR Output
```tcl
create_clock -name clk_ddr -period 6 [get_ports DDR_CLK_IN]
set_output_delay -clock clk_ddr -max 2.1 [get_ports DDR_OUT]
set_output_delay -clock clk_ddr -max 1.9 [get_ports DDR_OUT] -clock_fall -add_delay
set_output_delay -clock clk_ddr -min 0.9 [get_ports DDR_OUT]
set_output_delay -clock clk_ddr -min 1.1 [get_ports DDR_OUT] -clock_fall -add_delay
```

### Combinational Path (in-to-out)
```tcl
create_clock -name sysClk -period 10
set_input_delay -clock sysClk 4 [get_ports DIN]
set_output_delay -clock sysClk 1 [get_ports DOUT]
# Effective budget: 10 - 4 - 1 = 5 ns
```

**Key rules:**
- `-clock` is REQUIRED in Vivado (optional in SDC standard)
- `-clock_fall` refers to the CLOCK edge, not data edge
- `-add_delay` needed for second constraint on same port (DDR)
- Use virtual clock to model different jitter/source latency scenarios

---

## Timing Exceptions

### Multicycle Path — Decision Table

| Scenario | Constraints |
|----------|------------|
| Same clock / same-freq same-phase | `set_multicycle_path N -setup -from CLK1 -to CLK2` |
| | `set_multicycle_path N-1 -hold -from CLK1 -to CLK2` |
| SLOW → FAST | `set_multicycle_path N -setup -from CLK1 -to CLK2` |
| | `set_multicycle_path N-1 -hold -end -from CLK1 -to CLK2` |
| FAST → SLOW | `set_multicycle_path N -setup -start -from CLK1 -to CLK2` |
| | `set_multicycle_path N-1 -hold -from CLK1 -to CLK2` |

**Key rules:**
- `-setup` default: moves destination (capture) clock edge → use `-start` to move source instead
- `-hold` default: moves source (launch) clock edge → use `-end` to move destination instead
- `-start` and `-end` have no effect within same clock domain
- **Always pair setup + hold** multicycle constraints

### False Path
```tcl
# Between async clock domains (prefer set_clock_groups instead)
set_false_path -from [get_clocks CLKA] -to [get_clocks CLKB]
set_false_path -from [get_clocks CLKB] -to [get_clocks CLKA]

# Reset signal
set_false_path -from [get_ports reset] -to [all_registers]

# Through specific path (ORDER of -through matters!)
set_false_path -through [get_pins MUX1/a0] -through [get_pins MUX2/a1]

# Setup-only or hold-only
set_false_path -setup -from [get_clocks CLKA] -to [get_clocks CLKB]
```

**CAUTION:** `-through` without `-from` or `-to` removes ALL paths through that pin/net.

### Max/Min Delay
```tcl
# Override setup requirement
set_max_delay 5 -from [get_pins FD1/C] -to [get_pins FD2/D]

# Override hold requirement
set_min_delay 1 -from [get_pins FD1/C] -to [get_pins FD2/D]

# CDC path with -datapath_only (no clock skew, hold auto false-pathed)
set_max_delay -datapath_only -from [get_cells src_reg*] -to [get_cells dst_reg*] 10.0
```

**-datapath_only differences:**

| | set_max_delay | set_max_delay -datapath_only |
|---|---|---|
| Clock skew | Included | Never included |
| Hold requirement | Untouched | Auto false-pathed |
| -from | Optional | Mandatory |

### Case Analysis
```tcl
set_case_analysis <value> <pins_or_ports>
# Values: 0, 1, zero, one, rise, rising, fall, falling
# rise/rising/fall/falling → only specified transition analyzed

# Example: select clk_2 through BUFGMUX
set_case_analysis 1 [get_pins clock_sel/S]
```

### Disable Timing
```tcl
# Disable cell timing arcs
set_disable_timing -from WCLK -to O [get_cells inst_fifo_gen/gdm.dm/gprl.dout_i_reg[*]]

# Check all disabled arcs
report_disable_timing -file disabled_arcs.rpt
```

---

## XDC Precedence Rules

### Exception Priority (highest → lowest)
1. **set_clock_groups** — cannot be overridden
2. **set_false_path**
3. **set_max_delay / set_min_delay**
4. **set_multicycle_path**

### Object Specificity (highest → lowest)
1. Ports, pins, cells (cells resolved to pins)
2. Clocks

### Filter Specificity (highest → lowest)
1. `-from -through -to`
2. `-from -to`
3. `-from -through`
4. `-from`
5. `-through -to`
6. `-to`
7. `-through`

**`-reset_path`** can override false_path/max_delay/multicycle priority, but NOT clock_groups.

**Last constraint wins** for equivalent constraints (same type, same specificity).

---

## CDC Constraints

### Asynchronous CDC
```tcl
# Option 1: set_clock_groups (RECOMMENDED — covers both directions)
set_clock_groups -asynchronous -group [get_clocks clkA] -group [get_clocks clkB]

# Option 2: set_false_path (need BOTH directions)
set_false_path -from [get_clocks clkA] -to [get_clocks clkB]
set_false_path -from [get_clocks clkB] -to [get_clocks clkA]
```

### CDC with max delay constraint
```tcl
# Limit path delay for CDC (when using synchronizer + set_false_path)
set_max_delay -datapath_only -from [get_cells src_reg*] -to [get_cells dst_sync_reg*] 10.0
```

### Multi-bit CDC with bus skew
```tcl
# CE-controlled CDC (handshake): skew = N_sync_stages * dst_period
set_bus_skew -from [get_cells src_hsdata_ff_reg*] -to [get_cells dest_hsdata_ff_reg*] 10.000

# Gray-coded FIFO: skew = dst_period
set_bus_skew -from [get_cells src_gray_ff_reg*] -to [get_cells {dest_graysync_ff_reg[0]*}] 2.500

# set_bus_skew requires: -from AND -to, at least 2 startpoints + 2 endpoints
# Value should be > 0.5 * min(src_period, dst_period)
```

---

## Constraint Ordering for Performance

Write XDC sections in this order for minimum runtime impact:

| Order | Commands | Reason |
|-------|----------|--------|
| 1 | `set_disable_timing`, `set_case_analysis` | Prune timing graph first |
| 2 | `create_clock`, `create_generated_clock`, `set_clock_sense` | Define clocks before referencing |
| 3 | `set_clock_latency`, `set_propagated_clock`, `set_clock_uncertainty`, `set_input_jitter`, `set_system_jitter` | Clock properties |
| 4 | `set_input_delay`, `set_output_delay` | I/O timing |
| 5 | `set_clock_groups`, `set_false_path`, `set_min_delay`, `set_max_delay`, `set_multicycle_path`, `set_bus_skew` | Timing exceptions |
| 6 | `set_max_time_borrow`, `set_external_delay` | Rarely used |

**Performance tips:**
- Use `get_cells` instead of `get_pins` for large queries
- Cache repeated queries in Tcl variables
- Avoid `all_fanin`/`all_fanout` combined with `set_disable_timing`
- Replace `all_registers -clock clk1` with `get_clocks clk1` where possible

---

## Constraint Validation Commands

```tcl
# Check for unconstrained paths
check_timing -file check_timing.rpt

# Review timing exception coverage, conflicts, ignored constraints
report_exceptions -coverage -file exceptions_coverage.rpt
report_exceptions -ignored -file exceptions_ignored.rpt
report_exceptions -scope_override -file exceptions_scope.rpt
report_exceptions -ignored_objects -file exceptions_ignored_obj.rpt

# Clock domain interaction matrix
report_clock_interaction -file clock_interaction.rpt

# Methodology checks (XDCV-1, XDCV-2 for large constraint collections)
report_methodology -file methodology.rpt
```
