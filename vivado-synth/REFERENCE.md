# Vivado Synthesis Complete Reference

Full attribute syntax examples and parameter tables from UG901 (v2025.2).

## 1. All Synthesis Attributes — Verilog Syntax Examples

### Prevention Attributes

```verilog
// DONT_TOUCH — prevent optimization, forwarded to P&R
(* dont_touch = "yes" *) wire sig1;
(* dont_touch = "yes" *) module example_dt_ver (clk, In1, In2, out1);
(* dont_touch = "yes" *) example_dt_ver U0 (.clk(clk), .in1(a), .in2(b), .out1(c));

// KEEP — prevent signal absorption (synthesis only, NOT forwarded to P&R)
(* keep = "true" *) wire sig1;

// KEEP_HIERARCHY — prevent cross-boundary optimization
(* keep_hierarchy = "yes" *) module bottom (in1, in2, in3, in4, out1, out2);
// On instance:
(* keep_hierarchy = "yes" *) bottom u0 (.in1(in1), .in2(in2), .out1(temp1));
// XDC:
set_property keep_hierarchy yes [get_cells u0]
// RECOMMENDED: KEEP_HIERARCHY=SOFT over TRUE (allows constant propagation)
```

### Resource Inference

```verilog
// RAM_STYLE — block / distributed / registers / ultra / mixed / auto
(* ram_style = "block" *) reg [DATA_W-1:0] myram [DEPTH-1:0];
(* ram_style = "distributed" *) reg [7:0] small_mem [31:0];
// XDC: set_property ram_style block [get_cells myram]

// ROM_STYLE — block / distributed / ultra
(* rom_style = "distributed" *) reg [DATA_W-1:0] myrom [DEPTH-1:0];

// USE_DSP — yes / no / logic / simd
(* use_dsp = "yes" *) module test(clk, in1, in2, out1);
(* use_dsp = "no" *) reg [31:0] result;  // keep in fabric
// XDC: set_property use_dsp no [get_cells result]

// SHREG_EXTRACT — yes / no
(* shreg_extract = "no" *) reg [16:0] my_srl;

// SRL_STYLE — register / srl / srl_reg / reg_srl / reg_srl_reg / block
(* srl_style = "register" *) reg [16:0] my_srl;  // force registers, no SRL
(* srl_style = "srl_reg" *) reg [16:0] pipeline;  // SRL + one output register
// XDC: set_property srl_style register [get_cells my_shifter_reg*]

// RAM_DECOMP — power / area
(* ram_decomp = "power" *) reg [DATA_W-1:0] myram [DEPTH-1:0];
// XDC: set_property ram_decomp power [get_cells myram]

// CASCADE_HEIGHT — integer (0 or 1 = disable cascading)
(* cascade_height = 4 *) reg [31:0] ram [(2**15)-1:0];
// UltraScale+ and Versal only

// RW_ADDR_COLLISION — auto / yes / no (RTL only)
(* rw_addr_collision = "yes" *) reg [3:0] my_ram [1023:0];
```

### Timing Optimization

```verilog
// RETIMING_BACKWARD / RETIMING_FORWARD — integer (0 = off)
(* retiming_backward = 1 *) reg my_sig;
(* retiming_forward = 1 *) reg my_sig;
// XDC: set_property retiming_backward 1 [get_cells my_sig]
// Note: DONT_TOUCH/MARK_DEBUG, timing exceptions, user-instantiated cells block retiming

// CRITICAL_SIG_OPT — Shannon decomposition on critical paths
(* CRITICAL_SIG_OPT = "true" *) reg [3:0] signal_name;
// XDC: set_property CRITICAL_SIG_OPT 1 [get_cells <registers>]
// Warning: trades area for timing due to logic replication

// MAX_FANOUT — integer (-1 = no replication)
(* max_fanout = 50 *) reg sig1;
// XDC: set_property MAX_FANOUT <value> [get_cells in1_int_reg]
// RECOMMENDED: only use on LOCAL signals inside hierarchies, not global high-fanout
```

### Debug

```verilog
// MARK_DEBUG
(* MARK_DEBUG = "TRUE" *) wire debug_wire;
// XDC: set_property MARK_DEBUG true [get_nets -of [get_pins hier1/hier2/<flop_name>/Q]]

// ASYNC_REG — mark CDC synchronizer chain
(* ASYNC_REG = "TRUE" *) reg [2:0] sync_regs;
```

### FSM

```verilog
// FSM_ENCODING — one_hot / sequential / gray / johnson / user_encoding / none
(* fsm_encoding = "one_hot" *) reg [7:0] my_state;

// FSM_SAFE_STATE — auto_safe_state / reset_state / power_on_state / default_state
(* fsm_safe_state = "reset_state" *) reg [7:0] my_state;
```

### IO & Clock

```verilog
// IOB — pack register into IO buffer (RTL only)
(* IOB = "true" *) reg sig1;

// IO_BUFFER_TYPE — disable automatic IO buffers (RTL only)
(* io_buffer_type = "none" *) input in1;

// CLOCK_BUFFER_TYPE — BUFG / BUFH / BUFIO / BUFMR / BUFR / none
(* clock_buffer_type = "none" *) input clk1;
// XDC: set_property CLOCK_BUFFER_TYPE BUFG [get_ports clk]

// GATED_CLOCK
(* gated_clock = "yes" *) input clk;
// XDC: set_property GATED_CLOCK yes [get_ports clk]
```

### Enable & Reset

```verilog
// DIRECT_ENABLE — force signal to register CE pin
(* direct_enable = "yes" *) input ena3;
// XDC: set_property direct_enable yes [get_nets -of [get_ports ena3]]

// DIRECT_RESET — force signal to register reset pin
(* direct_reset = "yes" *) input rst3;
// XDC: set_property direct_reset yes [get_nets -of [get_ports rst3]]

// EXTRACT_ENABLE — control enable extraction (yes/no)
(* extract_enable = "yes" *) reg my_reg;
// XDC: set_property EXTRACT_ENABLE yes [get_cells my_reg]

// EXTRACT_RESET — control sync reset extraction (yes/no, sync only)
(* extract_reset = "yes" *) reg my_reg;
// XDC: set_property EXTRACT_RESET yes [get_cells my_reg]
```

### DSP Folding

```verilog
// DSP_FOLDING — fold two MACs into one DSP (RTL only, on module)
(* dsp_folding = "yes" *) module top .....

// DSP_FOLDING_FASTCLOCK — designate fast clock port (RTL only)
(* dsp_folding_fastclock = "yes" *) input clk_fast;
```

### Verilog Case

```verilog
// FULL_CASE — assert all values covered (RTL only)
(* full_case *)
case (select)
  3'b100 : sig = val1;
  3'b010 : sig = val2;
  3'b001 : sig = val3;
endcase

// PARALLEL_CASE — build as parallel, no priority (RTL only)
(* parallel_case *) case (select) ...
```

### Other

```verilog
// BLACK_BOX — force module to black box (RTL only)
(* black_box *) module test(in1, in2, clk, out1);

// TRANSLATE_OFF / ON — exclude code from synthesis
// synthesis translate_off
// ... simulation-only code ...
// synthesis translate_on
```

---

## 2. RTL Linter Rules

| Rule ID | Name | Description |
|---------|------|-------------|
| ASSIGN-1 | Arithmetic overflow | Target not large enough for result precision |
| ASSIGN-2 | Mixed signs | Operands have different signs |
| ASSIGN-3 | Shifter overflow | Shift value larger than result size |
| ASSIGN-5 | Signal bits not set | One or more bits never assigned |
| ASSIGN-6 | Signal bits not used | One or more bits never read |
| ASSIGN-7 | Multiple assignments | Array bits assigned multiple times (multi-driver risk) |
| ASSIGN-8 | Array size comparison | Arrays of different dimensions directly compared |
| ASSIGN-9 | IO bits not set | IO port bits not driven |
| ASSIGN-10 | IO bits not used | IO port bits not read |
| ASSIGN-11 | Mixed blocking/non-blocking | Same signal uses both assignment types |
| ASSIGN-12 | Unconnected inputs | Module instance has unconnected pins |
| QOR-1 | Non-mergeable operators | Two consecutive arithmetic operators cannot be merged |
| INFER-1 | Inferred latch | Latch inferred instead of register (often unintended) |
| INFER-2 | Full case statement | Case covers all conditions or has default |
| INFER-3 | Case equality (===) | Case equality auto-converted to logical equality (==) |
| INFER-4 | Combinational loop | Signal assigned through combinational feedback loop |
| CLOCK-1 | Both clock edges | Module uses both posedge and negedge of same clock |
| RESET-1 | Mixed async resets | Always block has more than one type of async reset |
| RESET-2 | Missing async resets | Async reset in sensitivity list but reset logic not specified |
| RESET-3 | Sync reset drives enable | Register without sync reset has enable driven by sync reset |

---

## 3. Block-Level Synthesis Settings (BLOCK_SYNTH)

### Syntax
```tcl
set_property BLOCK_SYNTH.<option> <value> [get_cells <instance_name>]
```

### Complete Options Table

| Option | Type | Values | Description |
|--------|------|--------|-------------|
| STRATEGY | STRING | DEFAULT, AREA_OPTIMIZED, ALTERNATE_ROUTABILITY, PERFORMANCE_OPTIMIZED | Predefined strategy for instance |
| RETIMING | INT | 0/1 | 0=disable, 1=enable retiming |
| ADDER_THRESHOLD | INT | 4-128 | Adder operand size threshold for CARRY chain inference. Higher=more LUTs, lower=more CARRYs |
| COMPARATOR_THRESHOLD | INT | 4-128 | Comparator size threshold for CARRY chain inference |
| SHREG_MIN_SIZE | INT | 3-32 | Min register chain length before SRL inference. Higher=more registers, lower=more SRLs |
| FSM_EXTRACTION | STRING | OFF/ONE_HOT/SEQUENTIAL/GRAY/JOHNSON/AUTO | FSM encoding for this instance |
| LUT_COMBINING | INT | 0/1 | 0=disable, 1=enable LUT combining |
| CONTROL_SET_THRESHOLD | INT | 0-128 | Fanout threshold for control set optimization |
| MAX_LUT_INPUT | INT | 4-6 | 4=no LUT5/LUT6, 5=no LUT6, 6=all LUTs allowed |
| MUXF_MAPPING | INT | 0/1 | 0=disable MUXF7/F8/F9, 1=enable |
| KEEP_EQUIVALENT_REGISTER | INT | 0/1 | 0=merge equivalent registers, 1=retain them |
| PRESERVE_BOUNDARY | INT | any | Mark hierarchy for incremental synthesis (value doesn't matter) |
| LOGIC_COMPACTION | INT | 1 | Arrange CARRY+LUT into fewer SLICEs (negative timing impact) |
| SRL_STYLE | STRING | REGISTER/SRL/SRL_REG/REG_SRL/REG_SRL_REG | SRL implementation style |

### Example: Multiple BLOCK_SYNTH properties
```tcl
set_property BLOCK_SYNTH.STRATEGY {ALTERNATE_ROUTABILITY} [get_cells mod_inst]
set_property BLOCK_SYNTH.KEEP_EQUIVALENT_REGISTER 1 [get_cells mod_inst]
set_property BLOCK_SYNTH.FSM_EXTRACTION {OFF} [get_cells mod_inst]

# Nested: different settings at sub-levels
set_property BLOCK_SYNTH.MAX_LUT_INPUT 6 [get_cells fftEngine/newlevel]
```

**Important notes:**
- Set on **instance names**, not module/entity names
- BLOCK_SYNTH hardens the instance hierarchy (keeps it intact)
- Can be nested at multiple hierarchy levels
- Only set where needed — unnecessary use reduces QoR

---

## 4. Attribute Placement Summary

| Attribute | RTL | XDC | Notes |
|-----------|-----|-----|-------|
| ASYNC_REG | Yes | Yes | |
| BLACK_BOX | Yes | No | Presence-only in Verilog |
| CASCADE_HEIGHT | Yes | Yes | UltraScale+/Versal only |
| CLOCK_BUFFER_TYPE | Yes | Yes | |
| CRITICAL_SIG_OPT | Yes | Yes | |
| DIRECT_ENABLE | Yes | Yes | XDC: use get_nets |
| DIRECT_RESET | Yes | Yes | XDC: use get_nets |
| DONT_TOUCH | Yes | Yes | MUST set in RTL if signal may be optimized before XDC is read |
| DSP_FOLDING | Yes | No | On module only |
| DSP_FOLDING_FASTCLOCK | Yes | No | On port/pin only |
| EXTRACT_ENABLE | Yes | Yes | |
| EXTRACT_RESET | Yes | Yes | Sync reset only |
| FSM_ENCODING | Yes | Yes | |
| FSM_SAFE_STATE | Yes | Yes | |
| FULL_CASE | Yes | No | On case statement |
| GATED_CLOCK | Yes | Yes | |
| IOB | Yes | No | |
| IO_BUFFER_TYPE | Yes | No | |
| KEEP | Yes | No | Synthesis only, NOT forwarded to P&R |
| KEEP_HIERARCHY | Yes | Yes | XDC: on instance only |
| MARK_DEBUG | Yes | Yes | |
| MAX_FANOUT | Yes | Yes | |
| PARALLEL_CASE | Yes | No | On case statement |
| RAM_DECOMP | Yes | Yes | |
| RAM_STYLE | Yes | Yes | |
| RETIMING_BACKWARD | Yes | Yes | |
| RETIMING_FORWARD | Yes | Yes | |
| ROM_STYLE | Yes | Yes | |
| RW_ADDR_COLLISION | Yes | No | |
| SHREG_EXTRACT | Yes | Yes | |
| SRL_STYLE | Yes | Yes | |
| TRANSLATE_OFF/ON | Yes | No | Comment-based |
| USE_DSP | Yes | Yes | |

**Key rule:** If same attribute in RTL and XDC with different values → XDC takes precedence.

## HDL Coding Examples (UG901)

Canonical Verilog templates in `examples/` directory. Read the specific file when the user needs a coding pattern.

| Category | Files | Description |
|----------|-------|-------------|
| RAM — Single Port | `rams_sp_rf.v`, `rams_sp_wf.v`, `rams_sp_nc.v`, `rams_sp_rf_rst.v` | Read-first / Write-first / No-change / With reset |
| RAM — Single Port ROM | `rams_sp_rom.v`, `rams_sp_rom_1.v` | ROM inferred from SP RAM |
| RAM — Simple Dual Port | `simple_dual_one_clock.v`, `simple_dual_two_clocks.v` | SDP with single/dual clock |
| RAM — True Dual Port | `rams_tdp_rf_rf.v` | TDP read-first both ports |
| RAM — Pipeline | `rams_pipeline.v` | Output pipeline register for timing |
| RAM — Init File | `rams_init_file.v`, `rams_init_file.data` | RAM with $readmemb init |
| RAM — Distributed | `rams_dist.v` | LUTRAM inference |
| RAM — 3D / Struct (SV) | `rams_sp_3d.sv`, `rams_sdp_3d.sv`, `rams_tdp_3d.sv`, `rams_sp_struct.sv`, `rams_sdp_struct.sv`, `rams_tdp_struct.sv` | Multi-dimensional / struct-based RAM |
| RAM — Asymmetric | `asym_ram_sdp_read_wider.v`, `asym_ram_sdp_write_wider.v`, `asym_ram_tdp_read_first.v`, `asym_ram_tdp_write_first.v` | Different port widths |
| RAM — Byte Write | `bytewrite_ram_1b.v`, `bytewrite_tdp_ram_nc.v`, `bytewrite_tdp_ram_rf.v`, `bytewrite_tdp_ram_wf.v`, `bytewrite_tdp_ram_readfirst2.v` | Byte-enable writes |
| UltraRAM | `xilinx_ultraram_single_port_no_change.v`, `xilinx_ultraram_single_port_read_first.v`, `xilinx_ultraram_single_port_write_first.v` | UltraScale+ URAM inference |
| Shift Registers | `shift_registers_0.v`, `shift_registers_1.v`, `dynamic_shift_registers_1.v` | Static / dynamic SRL inference |
| DSP / Arithmetic | `mult_unsigned.v`, `macc.v`, `cmacc.v`, `cmult.v`, `presubmult.v`, `dynpreaddmultadd.v`, `squarediffmult.v`, `squarediffmacc.v` | Multipliers, MAC, complex mult, pre-add |
| DSP — Rounding | `convergentRoundingEven.v`, `convergentRoundingOdd.v` | Convergent (banker's) rounding |
| FIR Filter | `sfir_even_symmetric_systolic_top.v`, `sfir_shifter.v` | Symmetric systolic FIR |
| Control Logic | `latches.v`, `registers_1.v`, `tristates_1.v`, `tristates_2.v`, `fsm_1.v` | Latches, regs, tri-state, FSM |
| Language Constructs | `functions_1.v`, `functions_constant.v`, `tasks_1.v`, `parameter_1.v`, `parameter_generate_for_1.v`, `procedure_package_1.v` | Verilog constructs |
| Black Box / Special | `black_box_1.v`, `top.v`, `xor_top.v`, `finish_ignored_1.v` | Black box, top-level wrapper |
