# Vivado XDC Constraints Complete Reference

Full syntax, examples, and property tables from UG903 (v2025.2).

## 1. Clock Definition Complete Syntax

### create_clock
```tcl
create_clock -period <period_ns> [-name <name>] [-waveform {<rise> <fall>}] \
    [-add] [<objects>]
```
| Option | Description |
|--------|-------------|
| `-period` | Clock period in nanoseconds (required) |
| `-name` | Clock name (defaults to port/pin name if omitted) |
| `-waveform` | {rise_time fall_time} in ns. Default: {0 period/2} = 50% duty |
| `-add` | Add clock to same object (multiple clocks on one port) |
| `<objects>` | Port or pin. Omit for virtual clock |

### create_generated_clock
```tcl
create_generated_clock -source <master_pin_or_port> [-name <name>] \
    [-divide_by <N>] [-multiply_by <N>] [-edges {<edge_list>}] \
    [-edge_shift {<shift_list>}] [-combinational] [-duty_cycle <pct>] \
    [-invert] [-master_clock <clock>] [-add] [-quiet] <source_object>
```
| Option | Description |
|--------|-------------|
| `-source` | Pin/port through which master clock propagates (NOT clock object) |
| `-divide_by` | Integer frequency divider |
| `-multiply_by` | Integer frequency multiplier |
| `-edges` | Master clock edge indices for generated waveform (e.g., {1 3 5}) |
| `-edge_shift` | Phase shift per edge in ns (same count as -edges) |
| `-combinational` | Trace only combinational paths from master |
| `-duty_cycle` | Duty cycle percentage (with -multiply_by) |
| `-invert` | Invert output waveform |
| `-master_clock` | Disambiguate when multiple clocks on -source pin |
| `-add` | Add generated clock (multiple on same object) |

**Auto-derived clock CMB cells:**

| 7 Series | UltraScale / UltraScale+ |
|----------|-------------------------|
| MMCM* / PLL* | MMCM* / PLL* |
| BUFR | BUFG_GT / BUFGCE_DIV |
| PHASER* | GT*_COMMON / GT*_CHANNEL / IBUFDS_GTE3 |
| | BITSLICE_CONTROL / RX*_BITSLICE |
| | ISERDESE3 |

### Clock renaming limitations
- Only auto-derived clocks can be renamed
- Must be renamed at CMB output pin
- Cannot use -edges/-edge_shift/-divide_by/-multiply_by/-combinational/-duty_cycle/-invert (these create NEW clock instead)
- Primary clocks and user-defined generated clocks CANNOT be renamed

---

## 2. I/O Delay Complete Examples

### SDR Input — All Variations
```tcl
# Both min and max (single value)
set_input_delay -clock sysClk 2 [get_ports DIN]

# Separate min/max
set_input_delay -clock sysClk -max 4 [get_ports DIN]
set_input_delay -clock sysClk -min 1 [get_ports DIN]

# Relative to virtual clock
create_clock -name clk_port_virt -period 10
set_input_delay -clock clk_port_virt 2 [get_ports DIN]

# Relative to falling clock edge
set_input_delay -clock sysClk -clock_fall 3 [get_ports DIN]
```

### DDR Input — Complete Template
```tcl
create_clock -name clk_ddr -period 6 [get_ports DDR_CLK_IN]
# Rising edge data
set_input_delay -clock clk_ddr -max 2.1 [get_ports DDR_IN]
set_input_delay -clock clk_ddr -min 0.9 [get_ports DDR_IN]
# Falling edge data (must use -add_delay for second set on same port)
set_input_delay -clock clk_ddr -max 1.9 [get_ports DDR_IN] -clock_fall -add_delay
set_input_delay -clock clk_ddr -min 1.1 [get_ports DDR_IN] -clock_fall -add_delay
```

### SDR Output — All Variations
```tcl
set_output_delay -clock sysClk 6 [get_ports DOUT]

# Separate min/max
set_output_delay -clock sysClk -max 6 [get_ports DOUT]
set_output_delay -clock sysClk -min 1 [get_ports DOUT]

# Virtual clock
create_clock -name clk_port_virt -period 10
set_output_delay -clock clk_port_virt 6 [get_ports DOUT]
```

### DDR Output — Complete Template
```tcl
create_clock -name clk_ddr -period 6 [get_ports DDR_CLK_IN]
set_output_delay -clock clk_ddr -max 2.1 [get_ports DDR_OUT]
set_output_delay -clock clk_ddr -min 0.9 [get_ports DDR_OUT]
set_output_delay -clock clk_ddr -max 1.9 [get_ports DDR_OUT] -clock_fall -add_delay
set_output_delay -clock clk_ddr -min 1.1 [get_ports DDR_OUT] -clock_fall -add_delay
```

### STARTUPE3 Internal Pins (UltraScale+)
```tcl
# Input from STARTUPE3
create_generated_clock -name clk_sck -source [get_pins -hierarchical *axi_quad_spi_0/ext_spi_clk] \
    [get_pins STARTUP/CCLK] -edges {3 5 7}
set_input_delay -clock clk_sck -max 7 [get_pins STARTUP/DATA_IN[*]] -clock_fall
set_input_delay -clock clk_sck -min 1 [get_pins STARTUP/DATA_IN[*]] -clock_fall

# Output to STARTUPE3
set_output_delay -clock clk_sck -max 6 [get_pins STARTUP/DATA_OUT[*]]
set_output_delay -clock clk_sck -min 1 [get_pins STARTUP/DATA_OUT[*]]
```

---

## 3. Timing Exceptions Complete Syntax

### set_multicycle_path
```tcl
set_multicycle_path <path_multiplier> [-setup|-hold] [-start|-end] \
    [-from <startpoints>] [-to <endpoints>] [-through <pins|cells|nets>] \
    [-rise_from|-fall_from|-rise_to|-fall_to] [-quiet]
```

| Option | Setup default | Hold default |
|--------|--------------|-------------|
| (no flag) | Moves capture edge (destination clock) | Moves launch edge (source clock) |
| `-start` | Moves launch edge (source clock) backward | N/A (default already source) |
| `-end` | N/A (default already destination) | Moves capture edge (destination clock) backward |

### set_false_path
```tcl
set_false_path [-setup|-hold] [-from <startpoints>] [-to <endpoints>] \
    [-through <pins|ports|nets>] [-rise_from|-fall_from|-rise_to|-fall_to] \
    [-reset_path] [-quiet]
```

### set_max_delay / set_min_delay
```tcl
set_max_delay <delay_ns> [-datapath_only] [-from <startpoints>] [-to <endpoints>] \
    [-through <pins|ports|nets>] [-rise_from|-fall_from|-rise_to|-fall_to] \
    [-reset_path] [-quiet]

set_min_delay <delay_ns> [-from <startpoints>] [-to <endpoints>] \
    [-through <pins|ports|nets>] [-rise_from|-fall_from|-rise_to|-fall_to] \
    [-reset_path] [-quiet]
```

### set_case_analysis
```tcl
set_case_analysis <value> <pins_or_ports>
```
| Value | Effect |
|-------|--------|
| `0`, `zero` | Pin/port held at logic 0 |
| `1`, `one` | Pin/port held at logic 1 |
| `rise`, `rising` | Only rising transition analyzed |
| `fall`, `falling` | Only falling transition analyzed |

### set_disable_timing
```tcl
set_disable_timing [-from <lib_pin>] [-to <lib_pin>] [-quiet] [-verbose] <objects>
```
- Pin names must be **library cell pin names**, not design pin names
- Without `-from`/`-to`: all arcs of the cell are disabled
- Use `report_disable_timing` to review

### set_bus_skew
```tcl
set_bus_skew -from <startpoints> -to <endpoints> [-through <pins|nets>] <value_ns>
```
- Both `-from` and `-to` required
- At least 2 startpoints and 2 endpoints required
- Value > 0.5 * min(src_period, dst_period) recommended
- Only optimized by `route_design`
- Does NOT interfere with timing exception precedence

---

## 4. Physical Constraints Properties

### I/O Constraints
| Property | Values | Description |
|----------|--------|-------------|
| `IOSTANDARD` | LVCMOS33, LVCMOS25, LVCMOS18, LVDS, HSTL_I, SSTL15, etc. | I/O voltage standard |
| `PACKAGE_PIN` | pin name (e.g., A14) | Physical pin location |
| `DRIVE` | 4, 8, 12, 16 (mA) | Output drive strength |
| `SLEW` | SLOW, FAST | Output slew rate |
| `IN_TERM` | NONE, UNTUNED_SPLIT_40/50/60 | Input termination |
| `DIFF_TERM` | TRUE, FALSE | 100-ohm differential termination |
| `KEEPER` | TRUE, FALSE | Weak driver to hold value |
| `PULLTYPE` | PULLUP, PULLDOWN, NONE | Pull resistor |
| `DCI_CASCADE` | set on IOBANK objects | Master/slave bank DCI |
| `INTERNAL_VREF` | voltage value | Internal Vref for IO bank |
| `IODELAY_GROUP` | group name | IDELAY/ODELAY grouping |
| `IOB` | TRUE, FALSE | Pack FF into IOB |
| `IOB_TRI_REG` | TRUE, FALSE | Tristate FF in HDIO IOB (UltraScale+) |

**Example: Complete I/O constraint set**
```tcl
set_property PACKAGE_PIN A14 [get_ports clk]
set_property IOSTANDARD LVCMOS33 [get_ports clk]

set_property PACKAGE_PIN B15 [get_ports {data[0]}]
set_property IOSTANDARD LVCMOS33 [get_ports {data[0]}]
set_property DRIVE 12 [get_ports {data[0]}]
set_property SLEW FAST [get_ports {data[0]}]
```

### Placement Constraints
| Property | Description | Example |
|----------|-------------|---------|
| `LOC` | Place cell at site | `set_property LOC RAMB18_X0Y10 [get_cells u_ctrl0/ram0]` |
| `BEL` | Place cell at BEL within slice | `set_property BEL C5LUT [get_cells u_ctrl0/lut0]` |
| `PBLOCK` | Read-only — cell's Pblock assignment | Use `add_cells_to_pblock` instead |
| `PROHIBIT` | Block site from being used | `set_property PROHIBIT TRUE [get_sites {RAMB18_X0Y*}]` |
| `LUTNM` | Co-place two LUTs (cross-hierarchy) | `set_property LUTNM L0 [get_cells {u_ctrl0/dmux0 u_ctrl0/dmux1}]` |
| `HLUTNM` | Co-place two LUTs (same hierarchy) | Same syntax as LUTNM |
| `IS_LOC_FIXED` | Promote LOC to fixed | `set_property IS_LOC_FIXED TRUE [get_cells reg0]` |
| `IS_BEL_FIXED` | Promote BEL to fixed | `set_property IS_BEL_FIXED TRUE [get_cells reg0]` |
| `LOCK_PINS` | Lock LUT input-to-physical pin mapping | `set_property LOCK_PINS {I0:A5 I1:A6} [get_cells myLUT]` |

**Important:** When assigning both BEL and LOC, assign BEL first, then LOC.

### Routing Constraints
| Property | Description |
|----------|-------------|
| `ROUTE` | Read-only — routing node sequence |
| `IS_ROUTE_FIXED` | Mark entire net route as fixed |
| `FIXED_ROUTE` | Fixed portion of net route |

```tcl
# Fix routing of a net
set_property IS_ROUTE_FIXED TRUE [get_nets netA]
# Or fix specific route path
set_property FIXED_ROUTE { CLBLL_LL_CQ ... IMUX_L11 CLBLL_LL_A4 } [get_nets netA]
```

### Netlist Constraints
| Property | Description |
|----------|-------------|
| `CLOCK_DEDICATED_ROUTE` | FALSE = allow non-dedicated clock routing (last resort) |
| `MARK_DEBUG` | Preserve net for ILA probing |
| `DONT_TOUCH` | Prevent optimization in synth AND impl. Use `reset_property` to remove (setting to 0 does NOT work) |
| `LOCK_PINS` | Fix LUT logical→physical pin mapping |

### Configuration Constraints
```tcl
set_property CONFIG_MODE M_SELECTMAP [current_design]
set_property BITSTREAM.GENERAL.DEBUGBITSTREAM Yes [current_design]
set_property BITSTREAM.GENERAL.CRC Disable [current_design]
```

---

## 5. Constraint Scoping Reference

### Project Mode
```tcl
# Scope to module (all instances of this module)
set_property SCOPED_TO_REF uart_tx_ctl [get_files uart_tx_ctl.xdc]

# Scope to specific instance
set_property SCOPED_TO_CELLS uart_tx_i0/uart_tx_ctl_i0 [get_files uart_tx_ctl.xdc]

# Both (instance within module)
set_property SCOPED_TO_REF uart_tx [get_files uart_tx_ctl.xdc]
set_property SCOPED_TO_CELLS uart_tx_ctl_i0 [get_files uart_tx_ctl.xdc]
```

### Non-Project Mode
```tcl
read_xdc -ref uart_tx_ctl uart_tx_ctl.xdc
read_xdc -cells uart_tx_i0/uart_tx_ctl_i0 uart_tx_ctl.xdc
read_xdc -ref uart_tx -cells uart_tx_ctl_i0 uart_tx_ctl.xdc
```

### Scoped Query Behavior
| Command | In Scoped Context |
|---------|-------------------|
| `get_cells/get_nets/get_pins` | Limited to scoped instance and sub-levels |
| `get_ports` | Returns top-level port if directly connected; otherwise returns hierarchical pin |
| `all_inputs/all_outputs` | NOT available in scoped XDC |
| `all_ffs/all_latches/all_registers/all_rams/all_dsps/all_hsios` | Scoped instance only |
| `get_clocks/all_clocks` | NOT scoped — returns all design clocks |
| `all_fanin/all_fanout` | Traverses scoped design, stops at boundary |

### Scoped Timing Constraint Rules
1. Do NOT define clocks if they will be created at top level
2. Query clocks via: `set blockClock [get_clocks -of_objects [get_ports clkIn]]`
3. Specify I/O delay only if port connects directly to top-level port with IO buffer inside IP
4. Do NOT define timing exceptions between clocks not bound to the IP
5. Do NOT refer to clocks by name (names may vary with top-level context)
6. Do NOT add placement constraints if block may be instantiated multiple times

---

## 6. RPM (Relatively Placed Macro)

### Verilog Syntax
```verilog
(* U_SET = "uset0", RLOC = "X0Y0" *) FD my_reg (.C(clk), .D(d0), .Q(q0));
(* HU_SET = "huset0", RLOC = "X0Y0" *) FD other_reg (.C(clk), .D(d1), .Q(q1));
```

### TCL Macro Commands
```tcl
# Create macro (XDC alternative to RPM)
create_macro my_macro
update_macro my_macro {cell1 X0Y0 cell2 X1Y0 cell3 X0Y1}

# Set origin
set_property RLOC_ORIGIN X10Y20 [get_cells my_macro]
```

### RPM Rules
- U_SET: group cells regardless of hierarchy
- HU_SET: hierarchically qualified set name
- H_SET: implied by hierarchy + RLOC presence
- Must be defined in HDL (NOT XDC)
- Preserve hierarchical boundary of RPM-containing module (use KEEP_HIERARCHY)
