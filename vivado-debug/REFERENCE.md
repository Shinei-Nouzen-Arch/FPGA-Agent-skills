# Vivado Programming & Debugging Complete Reference

Full command syntax, property tables, and debug flows from UG908 (v2025.2).

## 1. Debug Core Creation & Connection

```tcl
# Create an ILA debug core (post-synthesis netlist insertion)
create_debug_core <name> <type>
# type: ila (only supported type for netlist insertion)
# Example:
create_debug_core u_ila_0 ila

# Create a debug port on an existing core
create_debug_port <core>/<port> <type>
# type: clk, probe, trig_in, trig_out, trig_in_ack, trig_out_ack
# Example:
create_debug_port u_ila_0 probe    ;# Creates next sequential probe (probe1, probe2, ...)

# Set port width
set_property port_width <N> [get_debug_ports <core>/<port>]
# Example:
set_property port_width 8 [get_debug_ports u_ila_0/probe0]

# Connect debug port to design nets
connect_debug_port <core>/<port> [get_nets [list <nets>]]
# Example:
connect_debug_port u_ila_0/probe0 [get_nets [list {data[0]} {data[1]} {data[2]} {data[3]}]]

# Finalize debug core insertion into the netlist
implement_debug_core

# Report current debug core configuration
report_debug_core

# Write probe mapping file for Hardware Manager
write_debug_probes <filename>.ltx
# Example:
write_debug_probes design.ltx

# Mark nets for debug (in synthesized design)
set_property mark_debug true [get_nets <net_pattern>]
# Example:
set_property mark_debug true [get_nets {counter[*]}]

# Save constraints after debug core changes
save_constraints
```

## 2. ILA Debug Core Properties Complete Table

| Property | Description | Values | Default |
|----------|-------------|--------|---------|
| C_DATA_DEPTH | Maximum number of data samples stored by the ILA core. Increasing this value consumes more block RAM and can adversely affect design performance. | 1024, 2048, 4096, 8192, 16384, 32768, 65536, 131072 | 1024 |
| C_TRIGIN_EN | Enables the TRIG_IN and TRIG_IN_ACK ports of the ILA core. Must use advanced netlist change commands to connect these ports to design nets. | true, false | false |
| C_TRIGOUT_EN | Enables the TRIG_OUT and TRIG_OUT_ACK ports of the ILA core. Must use advanced netlist change commands to connect these ports to design nets. | true, false | false |
| C_ADV_TRIGGER | Enables the advanced trigger mode of the ILA core, providing state-machine-based triggering. | true, false | false |
| C_MEMORY_TYPE | Selects the memory primitive (Versal Only). Targeting UltraRAM can be useful for designs with high Block RAM utilization. | 0 (BRAM), 1 (URAM) | 0 (BRAM) |
| C_INPUT_PIPE_STAGES | Enables extra levels of pipe stages (flip-flop registers) on the PROBE inputs. Improves timing by allowing Vivado tools to place the ILA core away from critical sections. | 0, 1, 2, 3, 4, 5, 6 | 0 |
| C_EN_STRG_QUAL | Enables the basic capture control mode of the ILA core, allowing selective data storage based on a qualifier condition. | true, false | false |
| C_ALL_PROBE_SAME_MU | Enables all PROBE inputs to have the same number of comparators (match units). This property must always be set to true. | true, false (not recommended) | true |
| C_ALL_PROBE_SAME_MU_CNT | Number of comparators (match units) per PROBE input. Required count depends on C_ADV_TRIGGER and C_EN_STRG_QUAL settings. | 1-16 | 1 |

### Match Unit Count Rules

| C_ADV_TRIGGER | C_EN_STRG_QUAL | Allowed C_ALL_PROBE_SAME_MU_CNT |
|---------------|----------------|----------------------------------|
| false | false | 1 through 16 |
| false | true | 2 through 16 |
| true | false | 1 through 16 |
| true | true | 2 through 16 |

## 3. Hardware Manager Commands

### hw_manager Commands
```tcl
open_hw_manager                              ;# Open the Hardware Manager
close_hw_manager                             ;# Close the Hardware Manager
```

### hw_server Commands
```tcl
connect_hw_server -url <host>:<port>         ;# Connect to a hardware server
# Example:
connect_hw_server -url localhost:3121        ;# Default port is 3121

current_hw_server                            ;# Get or set the current hardware server
get_hw_servers                               ;# Get list of hardware server names
disconnect_hw_server <server>                ;# Close connection to a hardware server
# Example:
disconnect_hw_server localhost

refresh_hw_server                            ;# Refresh a connection to a hardware server
```

### hw_target Commands
```tcl
get_hw_targets                               ;# Get list of hardware targets
# Example with filter:
get_hw_targets */xilinx_tcf/Digilent/210203339395A

current_hw_target [get_hw_targets <pattern>] ;# Set the current hardware target
open_hw_target                               ;# Open connection to current hardware target
close_hw_target                              ;# Close current hardware target
# Example with specific target:
close_hw_target {localhost/xilinx_tcf/Digilent/210203339395A}

refresh_hw_target                            ;# Refresh a hardware target

# Set JTAG clock frequency on target
set_property PARAM.FREQUENCY <freq_hz> [get_hw_targets <pattern>]
# Example:
set_property PARAM.FREQUENCY 250000 [get_hw_targets */xilinx_tcf/Digilent/*]
```

### hw_device Commands
```tcl
get_hw_devices                               ;# Get list of hardware devices
# Example:
get_hw_devices xc7k325t_1

current_hw_device [get_hw_devices <name>]    ;# Set current hardware device

# Program the device
program_hw_devices [get_hw_devices <name>]
# Example:
program_hw_devices [lindex [get_hw_devices] 0]

# Refresh device status (re-reads registers, DONE pin, etc.)
refresh_hw_device [get_hw_devices <name>]
# With options:
refresh_hw_device -update_hw_probes false [lindex [get_hw_devices] 0]

# Boot a Versal device after programming
boot_hw_device [get_hw_devices <name>]
# Example:
boot_hw_device [get_hw_devices xcvm1802_1]
```

### Hardware Manager Tcl Object Hierarchy

| Tcl Object | Description |
|------------|-------------|
| hw_server | Object referring to hardware server. Can have one or more hw_target objects. |
| hw_target | Object referring to JTAG cable or board. Can have one or more hw_device objects. |
| hw_device | Object referring to a device in the JTAG chain (FPGA or adaptive SoC). |
| hw_ila | Object referring to an ILA core in the device. Has one hw_ila_data and one or more hw_probe objects. |
| hw_ila_data | Object referring to data uploaded from an ILA debug core. |
| hw_probe | Object referring to the probe input of an ILA debug core. |
| hw_vio | Object referring to a VIO core in the device. |

## 4. ILA Hardware Operations

```tcl
# Get list of ILA cores on the current device
get_hw_ilas
# Example with filter:
get_hw_ilas -of_objects [get_hw_devices xc7k325t_0] -filter {CELL_NAME=~"u_ila_0"}

# Get/set the current ILA
current_hw_ila [get_hw_ilas hw_ila_1]

# Reset ILA control properties to default values
reset_hw_ila [get_hw_ilas hw_ila_1]

# Set up trigger condition
set_property CONTROL.TRIGGER_POSITION 512 [get_hw_ilas hw_ila_1]
set_property COMPARE_VALUE.0 eq4'b0000 [get_hw_probes counter]

# Arm the ILA trigger
run_hw_ila [get_hw_ilas hw_ila_1]
# Arm with trigger settings file (Trigger at Startup)
run_hw_ila -file ila_trig.tas [get_hw_ilas hw_ila_1]

# Wait until all data has been captured
wait_on_hw_ila [get_hw_ilas hw_ila_1]
# With timeout (milliseconds):
wait_on_hw_ila -timeout 5000 [get_hw_ilas hw_ila_1]

# Upload captured data from ILA core
current_hw_ila_data [upload_hw_ila_data hw_ila_1]
upload_hw_ila_data [get_hw_ilas hw_ila_1]

# Display ILA data in waveform viewer
display_hw_ila_data

# Write captured ILA data to file
write_hw_ila_data <filename> [current_hw_ila_data]
# Example:
write_hw_ila_data my_capture.ila [current_hw_ila_data]

# Read previously saved ILA data from file
read_hw_ila_data <filename>
# Example:
read_hw_ila_data my_capture.ila

# List data samples for a probe
list_hw_samples [get_hw_probes counter -of_objects [get_hw_ilas hw_ila_1]]

# Apply trigger settings to implemented design (Trigger at Startup)
apply_hw_ila_trigger ila_trig.tas
```

### ILA Trigger and Capture Tcl Script Example
```tcl
# Connect and program
connect_hw_server -url localhost:3121
current_hw_target [get_hw_targets */xilinx_tcf/Digilent/12345]
open_hw_target
current_hw_device [lindex [get_hw_devices] 0]
set_property PROGRAM.FILE {C:/design.bit} [lindex [get_hw_devices] 0]
set_property PROBES.FILE {C:/design.ltx} [lindex [get_hw_devices] 0]
program_hw_devices [lindex [get_hw_devices] 0]
refresh_hw_device [lindex [get_hw_devices] 0]

# Set up ILA core trigger position and probe compare values
set_property CONTROL.TRIGGER_POSITION 512 [get_hw_ilas hw_ila_1]
set_property COMPARE_VALUE.0 eq4'b0000 [get_hw_probes counter]

# Arm the ILA trigger and wait for it to finish capturing data
run_hw_ila hw_ila_1
wait_on_hw_ila hw_ila_1

# Upload the captured ILA data, display it, and write it to a file
current_hw_ila_data [upload_hw_ila_data hw_ila_1]
display_hw_ila_data [current_hw_ila_data]
write_hw_ila_data my_hw_ila_data [current_hw_ila_data]
```

## 5. VIO Hardware Operations

```tcl
# Get list of VIO cores
get_hw_vios
# Example:
get_hw_vios -of_objects [get_hw_devices xc7k325t_0]

# Refresh VIO core (read input values from hardware)
refresh_hw_vio [get_hw_vios {hw_vio_1}]
# Refresh and update output values from hardware:
refresh_hw_vio -update_output_values 1 [get_hw_vios {hw_vio_1}]

# Read a VIO input probe value
get_property INPUT_VALUE [get_hw_probes BUTTON_IBUF]
# Example with -of_objects:
get_property INPUT_VALUE [get_hw_probes vio_probe_in -of_objects [get_hw_vios hw_vio_1]]

# Set a VIO output probe value
set_property OUTPUT_VALUE 1 [get_hw_probes vio_probe_out -of_objects [get_hw_vios hw_vio_1]]
# Example with binary value:
set_property OUTPUT_VALUE 11111 [get_hw_probes vio_slice5_fb_2]

# Commit output values to VIO hardware
commit_hw_vio [get_hw_vios {hw_vio_1}]
# Commit specific probe only:
commit_hw_vio [get_hw_probes {vio_slice5_fb_2}]

# Reset VIO input activity indicators
reset_hw_vio_activity [get_hw_vios {hw_vio_1}]

# Reset VIO output probes to initial values
reset_hw_vio_outputs [get_hw_vios {hw_vio_1}]

# Set VIO refresh rate (milliseconds, 0 = stop auto-refresh)
set_property CORE_REFRESH_RATE_MS 1000 [get_hw_vios hw_vio_1]

# Set input/output probe radix
set_property INPUT_VALUE_RADIX HEX [get_hw_probes BUTTON_IBUF]
set_property OUTPUT_VALUE_RADIX HEX [get_hw_probes vio_slice5_fb_2]

# Set activity persistence
set_property ACTIVITY_PERSISTENCE LONG [get_hw_probes BUTTON_IBUF]
```

### VIO Core Status Values

| VIO Status | Description | Required User Action |
|------------|-------------|---------------------|
| OK - Outputs Reset | Outputs are in sync with Vivado IDE and in initial/reset state | None |
| OK | Outputs are in sync with Vivado IDE, not in initial state | None |
| Outputs out-of-sync | Outputs are not in sync with Vivado IDE | commit_hw_vio or refresh_hw_vio -update_output_values 1 |

## 6. JTAG-to-AXI Master Operations

```tcl
# Get list of JTAG-to-AXI cores
get_hw_axis
# Example:
get_hw_axis hw_axi_1

# Reset the JTAG-to-AXI Master core (required before first use)
reset_hw_axi [get_hw_axis hw_axi_1]

# Create a read transaction
create_hw_axi_txn <name> [get_hw_axis hw_axi_1] -type READ -address <addr> -len <burst_len>
# Example: 4-word burst read from address 0
create_hw_axi_txn read_txn [get_hw_axis hw_axi_1] -type READ -address 00000000 -len 4

# Create a write transaction
create_hw_axi_txn <name> [get_hw_axis hw_axi_1] -type WRITE -address <addr> -len <burst_len> -data <data>
# Example: 4-word burst write to address 0
create_hw_axi_txn write_txn [get_hw_axis hw_axi_1] -type WRITE \
    -address 00000000 -len 4 -data {11111111_22222222_33333333_44444444}
# Note: -data direction is LSB to the left (address 0) and MSB to the right (address 3)

# Run a transaction
run_hw_axi [get_hw_axi_txns <name>]
# Example:
run_hw_axi [get_hw_axi_txns read_txn]
# Queued mode (up to 16 read and 16 write transactions, back-to-back):
run_hw_axi -queue [get_hw_axi_txns write_txn]

# Report transaction results
report_hw_axi_txn [get_hw_axi_txns read_txn]

# Get transaction data property
get_property DATA [get_hw_axi_txns read_txn]

# Delete a transaction
delete_hw_axi_txn [get_hw_axi_txns read_txn]

# Refresh AXI object status
refresh_hw_axi [get_hw_axis hw_axi_1]
```

### AXI Transaction Properties

| Property | Type | Description |
|----------|------|-------------|
| CMD.ADDR | string | Start address (hex) |
| CMD.BURST | enum | Burst type: INCR (default) |
| CMD.CACHE | int | Cache setting (default: 3) |
| CMD.ID | int | Transaction ID (default: 0) |
| CMD.LEN | int | Burst length in words |
| CMD.SIZE | enum | Data size: 32 or 64 bits |
| DATA | string | Read/write data (hex) |
| TYPE | enum | READ or WRITE |

### Note
Versal adaptive SoC devices do not support the JTAG-to-AXI Master. Use the built-in CIPS AXI Master with the Debug Packet Controller (DPC) instead.

## 7. JTAG Low-Level Operations

```tcl
# Run TCK clock cycles
runtest_hw_jtag -tck <count> [get_hw_devices <name>]
# Example: Run 100 TCK cycles
runtest_hw_jtag -tck 100 [get_hw_devices xc7k325t_0]

# Scan data register (DR)
scan_dr_hw_jtag <length> -tdi <data> [get_hw_devices <name>]
# Example:
scan_dr_hw_jtag 32 -tdi 00000000 [get_hw_devices xc7k325t_0]

# Scan instruction register (IR)
scan_ir_hw_jtag <length> -tdi <data> [get_hw_devices <name>]
# Example:
scan_ir_hw_jtag 6 -tdi 09 [get_hw_devices xc7k325t_0]

# Move JTAG state machine to specified state
run_state_hw_jtag <state> [get_hw_devices <name>]
# States: RESET, IDLE, DRSELECT, DRCAPTURE, DRSHIFT, DREXIT1, DRPAUSE,
#         DREXIT2, DRUPDATE, IRSELECT, IRCAPTURE, IRSHIFT, IREXIT1,
#         IRPAUSE, IREXIT2, IRUPDATE
```

## 8. mark_debug Syntax Reference

### Vivado Synthesis - Verilog
```verilog
(* mark_debug = "true" *) wire [7:0] char_fifo_dout;
(* mark_debug = "true" *) reg [3:0] counter;
```

### Vivado Synthesis - VHDL
```vhdl
attribute mark_debug : string;
attribute mark_debug of char_fifo_dout : signal is "true";
```

### Synplify - VHDL
```vhdl
attribute syn_keep : boolean;
attribute mark_debug : string;
attribute syn_keep of char_fifo_dout : signal is true;
attribute mark_debug of char_fifo_dout : signal is "true";
```

### Synplify - Verilog
```verilog
(* syn_keep = "true", mark_debug = "true" *) wire [7:0] char_fifo_dout;
```

### Synplify - SDC
```
define_attribute {n:char_fifo_din[*]} {mark_debug} {"true"}
define_attribute {n:char_fifo_din[*]} {syn_keep} {"true"}
```

### Precision - VHDL
```vhdl
attribute mark_debug : string;
attribute mark_debug of char_fifo_dout : signal is "true";
```

### Precision - Verilog
```verilog
(* mark_debug = "true" *) wire [7:0] char_fifo_dout;
```

### Tcl (Post-Synthesis)
```tcl
# Mark nets in an open synthesized design
set_property mark_debug true [get_nets {counter[*]}]
set_property mark_debug true [get_nets data_valid]
```

## 9. config_flows -mark_debug Options

```tcl
# Default: Do not optimize MARK_DEBUG nets (preserve through synth and impl)
config_flows -mark_debug enable

# Allow both synthesis and implementation to freely optimize MARK_DEBUG nets
config_flows -mark_debug disable

# Synthesis preserves MARK_DEBUG nets, but implementation can optimize them
config_flows -mark_debug synthesis_only
```

| Option | Synthesis Behavior | Implementation Behavior |
|--------|-------------------|------------------------|
| enable (default) | Preserves MARK_DEBUG nets | Preserves MARK_DEBUG nets |
| disable | May optimize MARK_DEBUG nets | May optimize MARK_DEBUG nets |
| synthesis_only | Preserves MARK_DEBUG nets | May optimize MARK_DEBUG nets |

## 10. Netlist Insertion Tcl Flow (Set Up Debug Wizard Equivalent)

```tcl
# 1. Open synthesized design
open_run synth_1

# 2. Mark nets for debug
set_property mark_debug true [get_nets {data[*]}]
set_property mark_debug true [get_nets enable]

# 3. Create ILA core
create_debug_core u_ila_0 ila

# 4. Set ILA core properties
set_property C_DATA_DEPTH 1024 [get_debug_cores u_ila_0]
set_property C_TRIGIN_EN false [get_debug_cores u_ila_0]
set_property C_TRIGOUT_EN false [get_debug_cores u_ila_0]
set_property C_ADV_TRIGGER false [get_debug_cores u_ila_0]
set_property C_INPUT_PIPE_STAGES 0 [get_debug_cores u_ila_0]
set_property C_EN_STRG_QUAL false [get_debug_cores u_ila_0]
set_property ALL_PROBE_SAME_MU true [get_debug_cores u_ila_0]
set_property ALL_PROBE_SAME_MU_CNT 1 [get_debug_cores u_ila_0]

# 5. Connect clock port (auto-created as u_ila_0/clk)
set_property port_width 1 [get_debug_ports u_ila_0/clk]
connect_debug_port u_ila_0/clk [get_nets [list clk]]

# 6. Connect probe0 (auto-created with the core)
set_property port_width 8 [get_debug_ports u_ila_0/probe0]
connect_debug_port u_ila_0/probe0 [get_nets [list \
    {data[0]} {data[1]} {data[2]} {data[3]} \
    {data[4]} {data[5]} {data[6]} {data[7]}]]

# 7. Create additional probes as needed
create_debug_port u_ila_0 probe
set_property port_width 1 [get_debug_ports u_ila_0/probe1]
connect_debug_port u_ila_0/probe1 [get_nets [list enable]]

# 8. Save constraints and implement
save_constraints
implement_debug_core

# 9. Write probe file for Hardware Manager
write_debug_probes design.ltx
```

## 11. Hardware Programming Flow

### 7 Series / UltraScale / UltraScale+ FPGA
```tcl
open_hw_manager
connect_hw_server -url localhost:3121
current_hw_target [get_hw_targets */xilinx_tcf/Digilent/*]
open_hw_target

# Set programming files
set_property PROGRAM.FILE {design.bit} [get_hw_devices xc7k325t_0]
set_property PROBES.FILE {design.ltx} [get_hw_devices xc7k325t_0]

# Program the device
program_hw_devices [get_hw_devices xc7k325t_0]

# Verify DONE status
get_property REGISTER.IR.BIT5_DONE [lindex [get_hw_devices] 0]

# Refresh to detect debug cores
refresh_hw_device [get_hw_devices xc7k325t_0]
```

### Versal Adaptive SoC
```tcl
open_hw_manager
connect_hw_server -url localhost:3121
open_hw_target

# Program with PDI file
set_property PROGRAM.FILE {design.pdi} [get_hw_devices xcvm1802_1]
program_hw_devices [get_hw_devices xcvm1802_1]

# Boot the Versal device
boot_hw_device [get_hw_devices xcvm1802_1]

# Verify DONE status (Versal uses different register)
get_property REGISTER.JTAG_STATUS.BIT[34]_DONE [lindex [get_hw_devices] 1]
```

### Auto-Connect Shortcut
```tcl
open_hw_manager
connect_hw_server
open_hw_target
```

## 12. SVF Programming Commands

### Creating and Writing SVF Files
```tcl
# 1. Open Hardware Manager and connect
open_hw_manager
connect_hw_server

# 2. Create SVF target
create_hw_target my_svf_target
open_hw_target [get_hw_targets *my_svf_target]

# 3. Create device chain FIRST (all devices before any programming)
set device0 [create_hw_device -part xcku9p]
set device1 [create_hw_device -part xcvu095]

# 4. Program devices
set_property PROGRAM.FILE {my_xcku9p.bit} $device0
program_hw_devices $device0
set_property PROGRAM.FILE {my_xcvu095.bit} $device1
program_hw_devices $device1

# 5. Write SVF file
write_hw_svf my_output.svf

# 6. Close target
close_hw_target
```

### Adding User-Defined Devices to SVF Chain
```tcl
open_hw_target [current_hw_target]
create_hw_device -idcode 01234567 -irlength 8 -mask ffffffff -part my_part
```

### Executing SVF Files
```tcl
# Execute on a live hardware target (must be open)
execute_hw_svf my_file.svf
# Verbose mode:
execute_hw_svf -verbose my_file.svf
```

### SVF Target Management
```tcl
# List SVF targets
get_hw_targets -filter {IS_SVF}

# Check if a target is SVF
get_property IS_SVF [get_hw_targets -regexp .*my_svf_target]

# Delete SVF target (also deletes all associated devices)
delete_hw_target [get_hw_targets -regexp .*my_svf_target]

# Report target details
report_hw_targets
```

Note: SVF programming is not supported on AMD Versal devices.

## 13. Debug Hub Clock Configuration

### Non-Versal: Reduce Debug Hub Clock Frequency
```tcl
# Set the input clock frequency (in Hz) on the debug hub
set_property C_CLK_INPUT_FREQ_HZ 200000000 [get_debug_cores dbg_hub]

# Enable the internal clock divider (inserts MMCM to achieve ~100 MHz)
set_property C_ENABLE_CLK_DIVIDER true [get_debug_cores dbg_hub]

# Note: Run these AFTER synthesis but BEFORE implementation
# AMD recommends debug hub clock frequency around 100 MHz or less
```

### Change Debug Hub Clock Source
```tcl
# Connect a different clock net to the debug hub
connect_debug_port dbg_hub/clk [get_nets <clock_net_name>]
# Note: Run after synthesis, before implementation
```

### Hardware Target JTAG Frequency
```tcl
# Set JTAG clock frequency (in Hz) for connecting to target
set_property PARAM.FREQUENCY 250000 [get_hw_targets */xilinx_tcf/Digilent/*]

# Default frequencies:
#   Digilent cables:  15 MHz (15000000)
#   USB cables:        6 MHz (6000000)
```

### Determine User Scan Chain Setting
```tcl
# In the implemented design:
get_property C_USER_SCAN_CHAIN [get_debug_cores dbg_hub]

# Change BSCAN user mask in Hardware Manager (bit mask value):
# Set on the hw_device BSCAN_SWITCH_USER_MASK property

# Or specify at hw_server startup:
# hw_server -e 'set bscan-switch-user-mask <user-bit-mask>'

# For older designs (pre-2016.3), detect debug hub at User Scan Chain 2 or 4:
# hw_server -e 'set xsdb-user-bscan <C_USER_SCAN_CHAIN scan_chain_number>'
```

## 14. Debug Clock Requirements Table

| Debugging Phase | JTAG Clock | Debug Hub Clock | Debug Core Clock |
|-----------------|-----------|-----------------|------------------|
| Connect to Target | Stable (does not pause/stop) | N/A | N/A |
| Programming | Stable | N/A | N/A |
| Debug Core Discovery | Stable | Stable | N/A |
| Debug Core Measurement | Stable | Stable | Stable |

### Critical Rules
- JTAG clock must be at least 2.5x slower than the debug hub clock.
- Debug Hub Clock must be free running and stable, driven from a properly constrained clock driver.
- If clocks are driven from MMCM/PLL, ensure the LOCKED signal is high prior to any debug core measurements.
- Debug Core Clock is assumed to be different from the Debug Hub Clock.
- A Debug Core Measurement phase includes any step that does a `get` or `set` of properties on the debug core.
- Versal Debug Cores use AXI-based connectivity and are not subject to BSCAN clocking guidelines.

## 15. Common Error Messages

| Error | Cause | Solution |
|-------|-------|----------|
| "The debug hub core was not detected at User Scan Chain 1 or 3" | Debug hub clock not running, unstable, or wrong scan chain | 1) Ensure free-running clock on dbg_hub core is active. 2) Manually launch hw_server with `-e 'set xsdb-user-bscan <C_USER_SCAN_CHAIN scan_chain_number>'` to detect at User Scan Chain 2 or 4. 3) Check: `get_property C_USER_SCAN_CHAIN [get_debug_cores dbg_hub]` |
| "unrecognizable debug core (slave type = 17)" | Debug core clock is inactive, unstable, or timing not met | 1) Ensure clock signal connected to the debug core and/or debug hub is clean and free-running. 2) Ensure clock meets all timing constraints. 3) Ensure clock connected to debug core/hub is faster than the JTAG clock frequency. |
| "incorrect bitstream assignment" | Bitstream generated for a different FPGA or adaptive SoC | Specify the correct bitstream file matching the target device part number. |
| "[Chipscope 16-336] Failed to find or create hub core for debug slave" | Debug Bridge IP present in design prevents debug hub insertion | Ensure design has at least one Debug Bridge IP in BSCAN-to-Debug Hub mode, or remove debug bridge cores in master/switch mode. |

### Troubleshooting Checklist
```tcl
# 1. Verify device is programmed (check DONE pin)
get_property REGISTER.IR.BIT5_DONE [lindex [get_hw_devices] 0]

# 2. Lower JTAG frequency if connection fails
set_property PARAM.FREQUENCY 250000 [get_hw_targets */xilinx_tcf/Digilent/*]

# 3. Verify debug hub scan chain
get_property C_USER_SCAN_CHAIN [get_debug_cores dbg_hub]

# 4. Check BSCAN switch user mask on programmed device
get_property BSCAN_SWITCH_USER_MASK [get_hw_devices xc7k325t_0]

# 5. For >32 devices in JTAG chain, start hw_server with:
# hw_server -e 'set max-jtag-devices 64'

# 6. For IR lengths > 64 bits:
# hw_server -e 'set max-ir-length 93'
```

### hw_server Command Line Options

| Option | Description | Default |
|--------|-------------|---------|
| `-e 'set max-jtag-devices <N>'` | Max devices in JTAG chain | 32 |
| `-e 'set max-ir-length <N>'` | Max instruction register length | 64 |
| `-e 'set xsdb-user-bscan <N>'` | User scan chain number for debug hub | 1 |
| `-e 'set bscan-switch-user-mask <mask>'` | BSCAN user switch bit mask | Auto-detect |
| `--init=<script.txt>` | Initialization script file | None |

### SmartLynq Default Ports

| Default Port | Description | config.ini Setting |
|-------------|-------------|-------------------|
| 80 | TCF over HTTP | `set http-port <port>` |
| 3121 | TCF | `set tcf-port <port>` |
| 10200 | Low level JTAG access over XVC | `set xvc-port <port>` |
| 3000-3005 | GNU Debugger ports (Arm/MicroBlaze) | `set gdb-port <base port>` |
