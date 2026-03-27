---
name: vivado-debug
description: Use this skill when the user needs help with Vivado in-system debugging, hardware programming, or debug core configuration. This includes ILA (Integrated Logic Analyzer) configuration and trigger strategies, VIO (Virtual I/O) usage for signal monitoring and control, JTAG-to-AXI Master for AXI transaction generation, mark_debug attribute and debug probing flows (Netlist Insertion, HDL Instantiation), Set Up Debug Wizard, ILA cross-trigger architecture, debug core timing impact and mitigation, Vivado Hardware Manager operation, FPGA/SoC device programming, Versal debugging architecture (AXI4 Debug Hub, CIPS integration), SVF file programming, debug clock requirements, and common debug error troubleshooting. This skill provides debugging strategy and decision-making knowledge — for TCL command execution use vivado-tcl, for implementation strategies use vivado-impl, for timing analysis use vivado-analysis.
---

# Vivado Programming & Debugging Guide

Based on UG908 (v2025.2). This skill helps choose debug strategies, configure debug cores, and troubleshoot hardware debugging. For complete command syntax, see REFERENCE.md. For TCL execution, use vivado-tcl.

## Debug Core Selection

Choose debug cores based on what you need to observe or control:

| Debug Core | Version | Purpose | Key Capability |
|---|---|---|---|
| ILA (Integrated Logic Analyzer) | v6.2 | Trigger on events and capture data at system speeds | Waveform capture, advanced triggers, storage qualification |
| VIO (Virtual Input/Output) | v3.0 | Monitor or control signals at JTAG scan rates | Real-time signal read/write, no waveform capture |
| JTAG-to-AXI Master | v1.2 | Generate AXI transactions to interact with AXI slave cores | Read/write AXI Full and AXI Lite interfaces |
| ILA Cross-Trigger | (ILA feature) | Synchronize triggers between ILA cores or between ILA and processor | Cross-clock-domain trigger coordination |

Decision guide:
```
Need to capture signal waveforms at full speed?
  YES --> ILA
  NO  --> Need to read/write signals in real-time?
            YES --> VIO (low bandwidth, JTAG rate)
            NO  --> Need to read/write AXI slave registers?
                      YES --> JTAG-to-AXI Master
                      NO  --> Need to coordinate triggers across clock domains?
                                YES --> ILA with Cross-Trigger enabled
```

## Debug Probing Flow Selection

Four approaches to add debug cores, each with different trade-offs:

| Scenario | Flow | How It Works | Best For |
|---|---|---|---|
| Tag signals in HDL, use wizard later | mark_debug + Netlist Insertion | Add `mark_debug` attribute in VHDL/Verilog. After synthesis, use Set Up Debug wizard to insert ILA cores. | Flexibility to enable/disable debug without HDL changes |
| Mark signals in synthesized netlist GUI | GUI Mark Debug | Right-click nets in Netlist/Schematic views and select Mark Debug. Use Set Up Debug wizard. | Quick signal selection without modifying source code |
| Automated scripted flow | TCL Automation | Use `set_property` to set mark_debug, then `create_debug_core` / `connect_debug_port` TCL commands. | Repeatable, version-controlled debug insertion |
| Full control in HDL source | HDL Instantiation | Manually instantiate ILA/VIO/JTAG-to-AXI Master IP in HDL and connect to signals. | Per-probe comparator control, cross-trigger ports, VIO cores |

## mark_debug Attribute

### Vivado Synthesis Syntax

**Verilog:**
```verilog
(* mark_debug = "true" *) wire [7:0] char_fifo_dout;
```

**VHDL:**
```vhdl
attribute mark_debug : string;
attribute mark_debug of char_fifo_dout: signal is "true";
```

Valid values: `"TRUE"` or `"FALSE"` (the `"SOFT"` value is not supported by Vivado synthesis).

### config_flows -mark_debug Modes

Control MARK_DEBUG behavior post-synthesis without modifying source files:

| Mode | Synthesis Behavior | Implementation Behavior | Use When |
|---|---|---|---|
| `enable` (default) | Do not optimize MARK_DEBUG nets | Do not optimize MARK_DEBUG nets | Active debugging, need nets preserved |
| `disable` | Freely optimize | Freely optimize | Production build, remove all debug overhead |
| `synthesis_only` | Do not optimize (nets available at start of impl) | Freely optimize during impl | Want debug net availability but allow impl optimization |

### Debug Net Icons in Vivado IDE

```
Hollow green icon  = MARK_DEBUG set, NOT connected to any ILA core
Full green icon    = MARK_DEBUG set, connected to an ILA core
Yellow icon        = No MARK_DEBUG on net, but connected to an ILA core
```

## ILA Configuration Decisions

### Data Depth Selection

The C_DATA_DEPTH property controls how many samples the ILA stores. Larger depth consumes more Block RAM:

| C_DATA_DEPTH | Samples | BRAM Impact | Recommended Use |
|---|---|---|---|
| 1024 (default) | 1K | Low | Initial debug, quick trigger verification |
| 2048 | 2K | Low-Medium | Short protocol sequences |
| 4096 | 4K | Medium | Typical protocol debug |
| 8192 | 8K | Medium-High | Longer event sequences |
| 16384 | 16K | High | Multi-phase protocol analysis |
| 32768 | 32K | High | Rare event capture with large pre/post-trigger window |
| 65536 | 64K | Very High | Extended capture, ensure BRAM budget allows |
| 131072 | 128K | Very High | Maximum capture depth, significant resource cost |

**Rule of thumb:** Start with 1024. Increase only when you need more pre/post-trigger context. Each doubling roughly doubles BRAM usage per probe bit.

### Key ILA Properties Decision Table

| Property | Default | Set to TRUE When | Impact |
|---|---|---|---|
| C_ADV_TRIGGER | false | Need state-machine-based triggers, counter triggers, or range triggers | Adds trigger logic resources |
| C_EN_STRG_QUAL | false | Need basic capture control (filter which samples are stored) | Uses 1 comparator for capture control |
| C_INPUT_PIPE_STAGES | 0 | Timing violations on ILA probe inputs; increase to 1-6 | Adds FF pipeline stages, improves timing at cost of sample latency |
| C_TRIGIN_EN | false | Need cross-trigger input (from another ILA or processor) | Adds TRIG_IN/TRIG_IN_ACK ports |
| C_TRIGOUT_EN | false | Need cross-trigger output (to another ILA or processor) | Adds TRIG_OUT/TRIG_OUT_ACK ports |
| C_MEMORY_TYPE (Versal only) | 0 (BRAM) | High BRAM utilization, want to use UltraRAM instead | 0=BRAM, 1=URAM; URAM can ease BRAM timing |

### Probe Configuration: Data / Trigger / Both

Each probe port can be configured independently:

| Probe Type | Captures Data | Participates in Trigger | BRAM Usage | Use When |
|---|---|---|---|---|
| Data and Trigger | Yes | Yes | Full | Need to both see and trigger on signal (most common) |
| Data Only | Yes | No | Full | Only need to see signal values, not trigger on them |
| Trigger Only | No | Yes | Reduced | Only need to trigger on signal, do not need waveform display |

**Tip:** Configuring wide buses as "Trigger Only" when you do not need their waveform significantly reduces BRAM consumption.

### Match Units (Comparators)

- Range: 1 to 16 comparators per probe (C_ALL_PROBE_SAME_MU_CNT)
- If C_ADV_TRIGGER=false and C_EN_STRG_QUAL=false: can be 1-16
- If C_ADV_TRIGGER=false and C_EN_STRG_QUAL=true: must be 2-16 (1 reserved for capture control)
- If C_ADV_TRIGGER=true and C_EN_STRG_QUAL=false: can be 1-16
- If C_ADV_TRIGGER=true and C_EN_STRG_QUAL=true: must be 2-16
- Maximum comparators allowed per ILA: 1024
- If Capture Control is enabled, you have 1 to 15 comparators (1 reserved)

**Important:** In the netlist insertion flow, all probes share the same number of comparators. Use HDL instantiation flow to set different comparator counts per probe.

## ILA Cross-Trigger

### Architecture

Cross-triggering enables trigger coordination between ILA cores in different clock domains, or between an ILA core and a processor (e.g., Zynq-7000 SoC).

```
  +------------------+                  +------------------+
  |     ILA 1        |                  |     ILA 2        |
  |                  |                  |                  |
  |   trig_in  <-----+------------------+-- trig_out      |
  |   trig_in_ack ---+------------------+-> trig_out_ack  |
  |                  |                  |                  |
  |   trig_out ------+------------------+-> trig_in       |
  |   trig_out_ack <-+------------------+-- trig_in_ack   |
  +------------------+                  +------------------+
```

Requirements:
- Enable C_TRIGIN_EN and/or C_TRIGOUT_EN at core generation time
- Use HDL instantiation method to connect TRIG_IN/TRIG_OUT ports to design nets
- The logic driving `trig_in` must be synchronous to the ILA clock

### Cross-Trigger Timing

| Signal | Latency | Notes |
|---|---|---|
| trig_in --> trig_in_ack | 1 clock cycle | ACK asserted 1 clk after trig_in asserts |
| trig_in (or trigger condition) --> trig_out | 9 clock cycles | TRIG_OUT asserted 9 clks after trigger condition met |
| trig_in_ack / trig_out_ack de-assert | When trigger de-asserts | ACK signals go low only when trigger signals are de-asserted |

**Behavior:** TRIG_OUT remains HIGH until TRIG_OUT_ACK is received. If TRIG_OUT_ACK is tied to LOW, TRIG_OUT remains HIGH until the user re-arms the ILA. Only TRIG_OUT goes LOW if TRIG_OUT_ACK is tied to LOW.

### Cross-Trigger Use Cases

- Trigger ILA in slow clock domain from event detected in fast clock domain
- Trigger ILA capture from a software breakpoint on Zynq/Versal processor
- Chain multiple ILA cores to capture a sequence of events across the design

## VIO Usage Guide

The VIO (Virtual Input/Output) core monitors and controls signals at JTAG scan rates (not system speed).

### Port Types

| Port Direction | Hardware Perspective | Use In Vivado | Typical Use |
|---|---|---|---|
| Input ports | Directly connected to design signals to monitor | Read values in VIO Dashboard (periodically refreshed) | Status indicators, counter values, state machine states |
| Output ports | Drive signals into the design for control | Write values from VIO Dashboard or TCL | Reset control, MUX select, enable signals, load values |

### Typical VIO Use Cases

- **Board bring-up:** Toggle reset signals, enable/disable subsystems without recompiling
- **Status monitoring:** Observe PLL lock, FIFO full/empty, error flags in real time
- **Stimulus injection:** Drive test patterns into data paths at low speed
- **ILA trigger coordination:** Use VIO output to gate an ILA trigger condition
- **Register access substitute:** Read/write control registers when JTAG-to-AXI Master is not available

### VIO Operating Model

```
VIO operates on set/commit and refresh/get model:

  Write flow:  set_property OUTPUT_VALUE <val> [get_hw_probes <probe>]
               commit_hw_vio [get_hw_probes {<probe>}]

  Read flow:   refresh_hw_vio [get_hw_vios {hw_vio_1}]
               get_property INPUT_VALUE [get_hw_probes <probe>]
```

Recommended refresh rate: 500 ms or longer (very small values make Vivado sluggish).

## JTAG-to-AXI Master

### Capabilities

- Supports all memory-mapped AXI Full and AXI-Lite interfaces
- 32-bit or 64-bit data width
- Create and run read/write burst transactions via TCL
- Queued operation: up to 16 read and 16 write transactions back-to-back

### Typical Use Cases

- Read/write peripheral registers without a processor
- Verify AXI slave IP functionality during board bring-up
- Inject data into AXI-connected BRAMs for testing
- Debug AXI interconnect routing issues

### Basic Transaction Flow

```
1. reset_hw_axi [get_hw_axis hw_axi_1]
2. create_hw_axi_txn read_txn [get_hw_axis hw_axi_1] \
     -type READ -address 00000000 -len 4
3. run_hw_axi [get_hw_axi_txns read_txn]
4. report_hw_axi_txn [get_hw_axi_txns read_txn]
```

**Important:** If you reprogram the device, all existing jtag_axi transactions are deleted and must be recreated.

## Versal Debug Architecture

Versal adaptive SoC uses a different debug infrastructure than 7 series/UltraScale/UltraScale+.

### CIPS Requirement

Every Versal design with debug cores requires a Control, Interface, and Processing System (CIPS) IP instance in a block design:

1. Create Block Design in IP Integrator
2. Add CIPS IP to the canvas
3. Generate HDL wrapper
4. Proceed with Netlist Insertion, HDL Instantiation, or IP Integrator debug flow
5. During `opt_design`, AXI4 Debug Hub is auto-inserted and connected

### AXI4 Debug Hub

The AXI4 Debug Hub replaces the BSCAN-based debug hub used in previous architectures. It connects the CIPS AXI4 interface to debug cores via AXI4-Stream.

Supported debug cores on Versal:
- AXI4-Stream ILA (AXIS-ILA) -- includes ILA and System-ILA functionality
- AXI4-Stream VIO (AXIS-VIO)
- PCI Express Link Debugger

### Three Connectivity Methods

| Method | Auto-Insertion | Debug Core Connection | When to Use |
|---|---|---|---|
| Automatic AXI4 Debug Hub insertion and connection | Yes, during `opt_design` | Automatic | Most designs (recommended). Cannot be used with DFX. |
| Manual AXI4 Debug Hub instantiation, automatic debug core connection | User instantiates Debug Hub | Automatic during `opt_design` | Need manual address assignment or using DFX. Vivado replaces user's Debug Hub with correctly configured one. |
| Manual AXI4 Debug Hub instantiation, manual debug core connection | User instantiates Debug Hub | User connects each core's AXI4-Stream master/slave | Full manual control. Required for DFX. Must set exact Number of Debug Cores. |

**Recommended AXI connection:** Use PMC NoC interface for the AXI4 Debug Hub. FPD/LPD interfaces are also possible, but extended address ranges 0x004_0000_0000 (8G) and 0x400_0000_0000 (1T) are not supported.

### BSCAN Fallback

When AXI-based interfaces from the PS/PMC cannot be used (e.g., AXI timeout), BSCAN Fallback provides an alternative communication path.

| Connectivity Option | Description | Suggested Use |
|---|---|---|
| AXI4 | Only AXI4 path enabled | Most hardware debug use cases |
| AXI4, BSCAN Fallback | Both paths available, AXI4 default | System-wide AXI timeout debugging |
| AXI4, BSCAN Fallback (BSCAN default) | Both paths available, BSCAN default | AXI path not accessible at boot |
| AXI4 (Unconnected), BSCAN (Default) | Only BSCAN pathway | Exclusive BSCAN connectivity to Debug Hub |

Steps to enable BSCAN Fallback:
1. Manually instantiate AXI4 Debug Hub (cannot enable on auto-inserted hub)
2. Enable BSCAN port on CIPS/PS Wizard
3. Instantiate BSCAN Switch IP between Debug Hub and Processing System
4. Connect AXI4 interface (if using) to desired PS interface (e.g., NoC)
5. Connect aclk and aresetn ports

## Debug Timing Impact & Mitigation

The ILA core can impact design timing. Follow this decision tree when timing violations appear after adding debug cores:

```
Timing violation after adding debug cores?
|
+-- Violation path is in ILA or AXIS-ILA core probe inputs?
|   |
|   YES --> Increase C_INPUT_PIPE_STAGES (try 1, then up to 6)
|           Still failing? --> Try URAM storage (Versal: C_MEMORY_TYPE=1)
|                              Still failing? --> Try impl strategy:
|                                                 Performance_Explore or
|                                                 Performance_ExtraTimingOpt
|
+-- Violation path is in debug_hub (dbg_hub) core?
|   |
|   YES --> 1. Set C_CLK_INPUT_FREQ_HZ to actual clock frequency
|           2. Set C_ENABLE_CLK_DIVIDER to true
|           3. Re-implement design
|           (This adds MMCM clock divider to achieve ~100 MHz internally)
|
+-- Violation path is elsewhere but worsened by debug?
    |
    YES --> 1. Close timing BEFORE adding debug cores
            2. Choose narrower probe widths
            3. Reduce C_DATA_DEPTH
            4. Use Performance_Explore impl strategy
            5. For Versal: use 100-250 MHz clock for AXI4-Debug Hub
```

Additional timing guidelines:
- Ensure ILA clock is free-running and stable
- Ensure ILA clock is synchronous to the signals being probed
- If clocks are driven from MMCM/PLL, ensure LOCKED signal is high before debug operations
- AMD recommends Debug Hub clock frequency around 100 MHz

## Hardware Debug Six-Step Flow

The steps to debug your design in hardware using an ILA debug core:

```
1. Connect to hardware target and program FPGA/SoC
   --> open_hw_manager
   --> connect_hw_server -url localhost:3121
   --> program_hw_devices [lindex [get_hw_devices] 0]

2. Set up the ILA: configure trigger and capture controls
   --> Add probes in Trigger Setup window
   --> Set compare values, operators, radix
   --> Choose trigger mode: BASIC_ONLY or ADVANCED (state machine)
   --> Configure capture mode: ALWAYS or conditional

3. Arm the ILA trigger
   --> run_hw_ila hw_ila_1

4. View captured data in Waveform window
   --> wait_on_hw_ila hw_ila_1
   --> display_hw_ila_data [upload_hw_ila_data hw_ila_1]

5. Use VIO core to drive control signals and view status
   --> Add probes in VIO Dashboard
   --> Set OUTPUT_VALUE and commit_hw_vio
   --> Read INPUT_VALUE after refresh_hw_vio

6. Use JTAG-to-AXI Master to run AXI transactions
   --> reset_hw_axi, create_hw_axi_txn, run_hw_axi
```

## Debug Clock Requirements

For non-Versal architectures (7 series, UltraScale, UltraScale+):

### Clock Requirements by Debug Phase

| Debugging Phase | JTAG Clock | Debug Hub Clock | Debug Core Clock |
|---|---|---|---|
| Connect to Target | Stable | N/A | N/A |
| Programming | Stable | N/A | N/A |
| Debug Core Discovery | Stable | Stable | N/A |
| Debug Core Measurement | Stable | Stable | Stable |

**Notes:**
- "Stable" = a clock that does not pause/stop during the event
- Debug Core Clock column assumes the debug core clock is different from the Debug Hub clock
- Debug Core Measurement includes any `get` or `set` of properties on the debug core

### The 2.5x JTAG Rule

**For non-Versal architectures:** If your design contains debug cores, ensure that the JTAG clock is 2.5x times slower than the debug hub clock.

```
JTAG_clock_frequency < Debug_Hub_clock_frequency / 2.5
```

Example: If Debug Hub clock is 100 MHz, JTAG must be < 40 MHz.

To lower JTAG frequency:
```tcl
set_property PARAM.FREQUENCY 250000 [get_hw_targets \
  */xilinx_tcf/Digilent/210203327962A]
```

AMD recommends Debug Hub clock frequency around 100 MHz. You can change the Debug Hub clock:
```tcl
connect_debug_port dbg_hub/clk [get_nets <clock net name>]
```

### Versal Clocking

Versal debug cores use AXI-based connectivity and are not subject to the BSCAN clocking guidelines. For Versal, if a timing failure is observed after adding debug cores, use a clock between 100 MHz and 250 MHz for the AXI4-Debug Hub.

## Common Error Troubleshooting

### "debug hub not detected"

**Error message:**
```
INFO: [Labtools 27-1434] Device xxx (JTAG device index = 0) is programmed
with a design that has no supported debug core(s) in it.
WARNING: [Labtools 27-3123] The debug hub core was not detected at User
Scan Chain 1 or 3.
```

**Causes and solutions:**
1. Debug Hub clock is not free-running or is inactive
   - Ensure the clock connected to `dbg_hub` core is free-running and stable
   - If driven from MMCM/PLL, verify LOCKED is high
2. User Scan Chain mismatch
   - Launch hw_server with: `hw_server -e "set xsdb-user-bscan <C_USER_SCAN_CHAIN scan_chain_number>"` to detect debug hub at User Scan Chain 2 or 4
   - Check setting: `get_property C_USER_SCAN_CHAIN [get_debug_cores dbg_hub]`
3. BSCAN_SWITCH_USER_MASK not set correctly
   - Verify in Hardware Device Properties

### "unrecognizable debug core"

**Error message:**
```
CRITICAL WARNING: [Labtools 27-1433] Device xxx is programmed
with a design that has an unrecognizable debug core (slave type = 17) at
user chain = 1, index = 0.
Resolution:
1) Ensure that the clock signal connected to the debug core and/or debug
   hub is clean and free-running.
2) Ensure that the clock connected to the debug core and/or debug hub meets
   all timing constraints.
3) Ensure that the clock connected to debug core and/or debug hub is faster
   than the JTAG clock frequency.
```

**Causes and solutions:**
1. Debug core clock is inactive or unstable
   - Ensure the clock is free-running, stable, and meets timing
2. Debug core clock is slower than JTAG clock
   - Lower JTAG frequency or use a faster debug clock
   - Remember the 2.5x rule: debug hub clock must be > 2.5x JTAG clock
3. Timing violations in debug hub or debug core
   - Enable clock divider on dbg_hub (C_ENABLE_CLK_DIVIDER)
   - Add input pipe stages to ILA (C_INPUT_PIPE_STAGES)

### Debug Bridge IP Conflict

**Error message:**
```
[Chipscope 16-336] Failed to find or create hub core for debug slave
<debug core name>. Insertion of debug hub is not supported when there are
instantiated debug bridge cores in either master mode or switch enabled in
the design.
```

**Solution:** Ensure the design has at least one instance of a Debug Bridge IP in BSCAN-to-Debug Hub mode. Either remove the debug slave core or instantiate a debug bridge master in the region of the debug slave.

## SVF File Programming

Serial Vector Format (SVF) provides offline FPGA/configuration memory programming without a live JTAG connection.

**Note:** SVF programming is not supported on AMD Versal devices.

### SVF File Creation Flow

```
1. create_hw_target my_svf_target    ;# Create offline SVF target
2. open_hw_target                     ;# Open the SVF target
3. create_hw_device -part <part>      ;# Add devices to define JTAG chain
4. set_property PROGRAM.FILE {file.bit} $device
   program_hw_devices $device         ;# Record programming operations
5. write_hw_svf my_output.svf         ;# Write cached operations to SVF file
6. close_hw_target                    ;# Close SVF target
```

**Important:** Create ALL devices in the chain first, then perform programming operations. Interleaving `create_hw_device` and `program_hw_devices` produces incorrect SVF sequences.

### SVF Execution

```tcl
execute_hw_svf my_file.svf
```

- Use `-verbose` option to see JTAG_TCL operations
- **Size limit:** Vivado supports SVF files under 500 MB. For larger files, use a third-party SVF player.
- The XSVF file format is not supported in Vivado IDE.

## Configurable Report Strategies for Debug

### report_debug_core Usage

After inserting debug cores, use `report_debug_core` to verify the debug configuration:

```tcl
# Report all debug cores in the design
report_debug_core

# Key information reported:
#   - Debug core instances and types
#   - Connected probe nets and widths
#   - Clock domain assignments
#   - Core properties (C_DATA_DEPTH, C_ADV_TRIGGER, etc.)
```

### Useful Debug Verification Commands

| Command | Purpose |
|---|---|
| `report_debug_core` | List all debug cores, their probes, and properties |
| `get_debug_cores` | Return list of debug core objects in design |
| `get_debug_ports` | Return list of debug port objects |
| `report_property [get_debug_cores u_ila_0]` | Show all properties of a specific ILA core |
| `get_property C_USER_SCAN_CHAIN [get_debug_cores dbg_hub]` | Check BSCAN user scan chain setting |
| `get_property C_DATA_DEPTH [get_debug_cores u_ila_0]` | Check ILA capture depth |
| `report_hw_targets` | Report all active hardware targets, devices, and properties |

### Post-Implementation Debug Verification

```tcl
# After implementation, verify debug cores are intact
open_run impl_1
report_debug_core
report_utilization -cells [get_cells -hierarchical -filter {IS_DEBUG_CORE}]

# Check DONE status after programming
get_property REGISTER.IR.BIT5_DONE [lindex [get_hw_devices] 0]

# For Versal devices (different register)
get_property REGISTER.JTAG_STATUS.BIT[34]_DONE [lindex [get_hw_devices] 1]
```
