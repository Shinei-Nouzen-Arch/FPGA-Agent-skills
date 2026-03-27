---
name: vivado-synth
description: Use this skill when the user needs help with Vivado synthesis strategy selection, synthesis attribute configuration, synth_design option tuning, resource inference control (RAM/DSP/SRL/BRAM), hierarchy optimization (flatten_hierarchy, KEEP_HIERARCHY), OOC (out-of-context) synthesis, incremental synthesis, RTL linting, Block-Level synthesis (BLOCK_SYNTH), or any synthesis optimization decision. Trigger when the user mentions synthesis strategies, synthesis directives, synthesis attributes, resource mapping control, FSM encoding, retiming, or asks how to optimize synthesis results for area/timing/power. This skill provides decision-making knowledge — for TCL command execution, use vivado-tcl skill.
---

# Vivado Synthesis Decision Guide

Based on UG901 (v2025.2). This skill helps choose the right synthesis strategies, options, and attributes. For attribute syntax see REFERENCE.md; for HDL coding templates (RAM/DSP/ROM/SRL/FSM/etc.) see `examples/` directory — consult the index table in REFERENCE.md to locate the right file.

## synth_design Complete Options

### Hierarchy Control
| Option | Values | Default | Effect |
|--------|--------|---------|--------|
| `-flatten_hierarchy` | none/full/rebuilt | rebuilt | **none**: preserve RTL hierarchy. **full**: flatten to top only. **rebuilt**: flatten then rebuild similar hierarchy (best QoR + readability) |

### Timing Optimization
| Option | Values | Default | Effect |
|--------|--------|---------|--------|
| `-directive` | See strategy table below | Default | Selects preconfigured optimization strategy |
| `-global_retiming` | auto/on/off | auto | Register balancing across combinational logic. Auto = on for Versal, off for others |
| `-no_timing_driven` | flag | — | Disables timing-driven synthesis (faster, worse timing) |

### Resource Control
| Option | Values | Default | Effect |
|--------|--------|---------|--------|
| `-max_bram` | integer | -1 (max) | Limit Block RAM usage |
| `-max_uram` | integer | -1 (max) | Limit UltraRAM usage (UltraScale+) |
| `-max_dsp` | integer | -1 (max) | Limit DSP block usage |
| `-shreg_min_size` | integer | 3 | Min chain length for SRL inference |
| `-srl_style` | register/rl/srl_reg/reg_srl/reg_srl_reg | — | Global SRL implementation style |
| `-cascade_dsp` | auto/tree/force | auto | DSP adder chain implementation |
| `-max_bram_cascade_height` | integer | -1 | Max BRAM cascade depth |
| `-max_uram_cascade_height` | integer | -1 | Max UltraRAM cascade depth |
| `-no_srlextract` | flag | — | Disable all SRL extraction |
| `-no_lc` | flag | — | Disable LUT combining |

### FSM & Logic
| Option | Values | Default | Effect |
|--------|--------|---------|--------|
| `-fsm_extraction` | auto/one_hot/sequential/gray/johnson/user_encoding/off | auto | FSM encoding strategy |
| `-resource_sharing` | auto/on/off | auto | Arithmetic operator sharing |
| `-keep_equivalent_registers` | flag | off | Prevent merging of equivalent registers |
| `-control_set_opt_threshold` | auto/integer/0 | auto | Fanout threshold for control set optimization. 0 = disable |

### Clock & IO
| Option | Values | Default | Effect |
|--------|--------|---------|--------|
| `-bufg` | integer | 12 | Max global clock buffers to infer |
| `-gated_clock_conversion` | off/on/auto | off | Convert gated clocks to enables |

### Design Entry
| Option | Values | Default | Effect |
|--------|--------|---------|--------|
| `-top` | module name | — | Top module |
| `-part` | part number | — | Target FPGA part |
| `-constrset` | fileset name | — | Constraint fileset to use |
| `-include_dirs` | paths | — | Verilog include search dirs |
| `-generic` | name=value | — | Override Verilog parameters / VHDL generics |
| `-verilog_define` | macro[=text] | — | Define Verilog macros |
| `-mode` | default/out_of_context | default | OOC mode: no IO buffers |

### Special Modes
| Option | Values | Default | Effect |
|--------|--------|---------|--------|
| `-rtl` | flag | — | Elaborate only, don't synthesize |
| `-lint` | flag | — | Run RTL Linter only (see section below) |
| `-incremental_mode` | default/quick/off | default | Incremental synthesis mode |
| `-sfcu` | flag | — | Single-file compilation unit mode |

### Debug & Performance
| Option | Values | Default | Effect |
|--------|--------|---------|--------|
| `-debug_log` | flag | — | Extra debug info in log |
| `-assert` | flag | — | Enable VHDL assertions |

### Multi-Threading
```tcl
set_param general.maxThreads 8  ;# 1-8 threads for synthesis
```

---

## Strategy Decision Table

**Use `-directive <strategy>` to select:**

| Scenario | Strategy | Key Effect |
|----------|----------|------------|
| Default / first try | `default` | Balanced optimization |
| Fast iteration / debug | `runtimeoptimized` | Fewer optimizations, faster runtime |
| Resource-constrained | `AreaOptimized_high` | Force ternary adders, includes AreaMapLargeShiftRegToBRAM + AreaMultThresholdDSP |
| Resource-constrained (moderate) | `AreaOptimized_medium` | Area-optimized MUX, ternary adders, lower multiplier→DSP threshold |
| Timing-critical | `PerformanceOptimized` | Logic level reduction at expense of area |
| Routing congestion | `AlternateRoutability` | Less MUXF/CARRY usage for better routability |
| Minimum area | `LogicCompaction` | Compact CARRY+LUT into fewer SLICEs (negative timing impact) |
| DSP-heavy | `AreaMultThresholdDSP` | Lower threshold for multiplier→DSP inference |
| Carry-chain issues | `FewerCarryChains` | Higher threshold to use LUTs instead of carry chains |
| Large shift registers | `AreaMapLargeShiftRegToBRAM` | Implement large shift regs in Block RAM |

**Note:** Directive values are case-sensitive in v2025.2. Use exactly as shown above.

---

## Synthesis Attributes Quick Reference (by scenario)

### Preventing Optimization
| Attribute | Values | Where | When to Use |
|-----------|--------|-------|-------------|
| `DONT_TOUCH` | TRUE/FALSE | RTL+XDC | Prevent optimization AND preserve through P&R. Strongest protection |
| `KEEP` | TRUE/FALSE | RTL only | Prevent signal absorption into LUTs. Synthesis-only, NOT forwarded to P&R |
| `KEEP_HIERARCHY` | TRUE/SOFT/FALSE | RTL+XDC | **SOFT** (recommended): allow constant propagation. **TRUE**: block all cross-boundary optimization |

### Resource Inference Control
| Attribute | Values | Where | When to Use |
|-----------|--------|-------|-------------|
| `RAM_STYLE` | block/distributed/registers/ultra/mixed/auto | RTL+XDC | Force specific RAM implementation |
| `ROM_STYLE` | block/distributed/ultra | RTL+XDC | Force specific ROM implementation |
| `USE_DSP` | yes/no/logic/simd | RTL+XDC | Force/prevent DSP block usage. `logic`=XOR→DSP, `simd`=SIMD mode |
| `SHREG_EXTRACT` | yes/no | RTL+XDC | Enable/disable SRL inference |
| `SRL_STYLE` | register/srl/srl_reg/reg_srl/reg_srl_reg/block | RTL+XDC | Specific SRL implementation |
| `RAM_DECOMP` | power/area | RTL+XDC | RAM split: `power` = address decode (saves power), `area` = smallest |
| `CASCADE_HEIGHT` | integer (0=disable) | RTL+XDC | BRAM/URAM cascade chain length (UltraScale+ only) |
| `RW_ADDR_COLLISION` | auto/yes/no | RTL only | Read-write collision handling |

### Timing Optimization
| Attribute | Values | Where | When to Use |
|-----------|--------|-------|-------------|
| `RETIMING_FORWARD` | integer (0=off) | RTL+XDC | Move register forward through logic |
| `RETIMING_BACKWARD` | integer (0=off) | RTL+XDC | Move register backward through logic |
| `CRITICAL_SIG_OPT` | true/false | RTL+XDC | Shannon decomposition on critical feedback loops. Trades area for timing |
| `MAX_FANOUT` | integer (-1=no limit) | RTL+XDC | Fanout limit → triggers register replication. Best on local signals only |

### Debug
| Attribute | Values | Where | When to Use |
|-----------|--------|-------|-------------|
| `MARK_DEBUG` | TRUE/FALSE | RTL+XDC | Mark nets for ILA debug probing |
| `ASYNC_REG` | TRUE/FALSE | RTL+XDC | Mark CDC synchronizer registers |

### FSM
| Attribute | Values | Where | When to Use |
|-----------|--------|-------|-------------|
| `FSM_ENCODING` | one_hot/sequential/gray/johnson/user_encoding/none | RTL+XDC | Override FSM encoding |
| `FSM_SAFE_STATE` | auto_safe_state/reset_state/power_on_state/default_state | RTL+XDC | Add invalid state recovery logic |

### IO & Clock
| Attribute | Values | Where | When to Use |
|-----------|--------|-------|-------------|
| `IOB` | TRUE/FALSE | RTL only | Pack register into IOB |
| `IO_BUFFER_TYPE` | NONE | RTL only | Disable automatic IO buffer insertion |
| `CLOCK_BUFFER_TYPE` | BUFG/BUFH/BUFIO/BUFMR/BUFR/none | RTL+XDC | Specify clock buffer type |
| `GATED_CLOCK` | yes | RTL+XDC | Mark signal as clock for gated clock conversion |

### Enable & Reset
| Attribute | Values | Where | When to Use |
|-----------|--------|-------|-------------|
| `DIRECT_ENABLE` | yes | RTL+XDC | Force signal to CE pin of register |
| `DIRECT_RESET` | yes | RTL+XDC | Force signal to reset pin of register |
| `EXTRACT_ENABLE` | yes/no | RTL+XDC | Control enable extraction to CE pin |
| `EXTRACT_RESET` | yes/no | RTL+XDC | Control reset extraction (sync reset only) |

### DSP Folding
| Attribute | Values | Where | When to Use |
|-----------|--------|-------|-------------|
| `DSP_FOLDING` | yes/no | RTL only | Fold two MAC structures into one DSP |
| `DSP_FOLDING_FASTCLOCK` | yes/no | RTL only | Designate fast clock port for DSP folding |

### Verilog Case Control
| Attribute | Values | Where | When to Use |
|-----------|--------|-------|-------------|
| `FULL_CASE` | (presence) | RTL only | All case values covered — suppress latch inference |
| `PARALLEL_CASE` | (presence) | RTL only | Build as parallel if-elsif, no priority |

### Other
| Attribute | Values | Where | When to Use |
|-----------|--------|-------|-------------|
| `BLACK_BOX` | (presence) | RTL only | Force module to black box |
| `TRANSLATE_OFF/ON` | comment-based | RTL only | Exclude code from synthesis |

---

## Block-Level Synthesis (BLOCK_SYNTH)

Apply per-instance synthesis settings via XDC:
```tcl
set_property BLOCK_SYNTH.<option> <value> [get_cells <instance>]
```

| Option | Type | Values | Description |
|--------|------|--------|-------------|
| STRATEGY | STRING | DEFAULT/AREA_OPTIMIZED/ALTERNATE_ROUTABILITY/PERFORMANCE_OPTIMIZED | Per-instance strategy |
| RETIMING | INT | 0/1 | Enable/disable retiming |
| ADDER_THRESHOLD | INT | 4-128 | Adder size → CARRY chain threshold |
| COMPARATOR_THRESHOLD | INT | 4-128 | Comparator size → CARRY chain threshold |
| SHREG_MIN_SIZE | INT | 3-32 | SRL inference threshold |
| FSM_EXTRACTION | STRING | OFF/ONE_HOT/SEQUENTIAL/GRAY/JOHNSON/AUTO | FSM encoding |
| LUT_COMBINING | INT | 0/1 | Enable/disable LUT combining |
| CONTROL_SET_THRESHOLD | INT | 0-128 | Control set optimization threshold |
| MAX_LUT_INPUT | INT | 4-6 | 4=no LUT5/6, 5=no LUT6, 6=all |
| MUXF_MAPPING | INT | 0/1 | Enable/disable MUXF7/F8/F9 |
| KEEP_EQUIVALENT_REGISTER | INT | 0/1 | Merge or retain equivalent registers |
| PRESERVE_BOUNDARY | INT | any | Mark hierarchy as changing (for incremental) |
| LOGIC_COMPACTION | INT | 1 | Compact CARRY+LUT into fewer SLICEs |
| SRL_STYLE | STRING | REGISTER/SRL/SRL_REG/REG_SRL/REG_SRL_REG | SRL implementation |

**Note:** BLOCK_SYNTH hardens the instance hierarchy. Use only where needed.

---

## OOC (Out-of-Context) Synthesis

**When to use:** Large IP modules, 3rd-party netlists, modules that rarely change.

```tcl
# Non-Project Mode
synth_design -top <module> -part <part> -mode out_of_context

# Project Mode: right-click module → Set As Out-of-Context for Synthesis
```

**Key rules:**
- OOC modules are synthesized independently, treated as black boxes in top-level synthesis
- No IO buffers are created
- Incremental synthesis is NOT supported for OOC runs
- Do NOT use OOC if module has AMD IP in lower levels
- Do NOT use OOC if module ports use user-defined types

---

## Incremental Synthesis

Detects RTL changes and only re-synthesizes modified sections. Reduces runtime and QoR fluctuation.

```tcl
# Project Mode: Settings → Synthesis → Incremental synthesis
# Non-Project Mode:
synth_design -top <top> -part <part> -incremental_mode default
```

| Mode | Effect |
|------|--------|
| `default` | Full incremental (detect + selective re-synthesis) |
| `quick` | Faster, less thorough incremental |
| `off` | Full re-synthesis |

**Note:** Not supported for OOC runs.

---

## RTL Linter

Pre-synthesis code quality check:
```tcl
synth_design -lint -top <top> -part <part>
# For OOC runs:
synth_design -lint -srcset [get_property SRCSET [get_runs my_IP_core_synth_1]]
```

**Key rules detected:** latch inference (INFER-1), combinational loops (INFER-4), arithmetic overflow (ASSIGN-1), mixed clock edges (CLOCK-1), mixed async resets (RESET-1), unconnected ports (ASSIGN-12). See REFERENCE.md for complete rule table.

**Waivers:**
```tcl
create_waiver -type LINT -id ASSIGN-1 -rtl_hierarchy x/y
write_waivers -type LINT -file waivers.tcl
```

---

## Synthesis-Stage Constraints

Only these constraints are used during synthesis:

| Type | Commands |
|------|----------|
| Timing | `create_clock`, `create_generated_clock`, `set_input_delay`, `set_output_delay`, `set_false_path`, `set_multicycle_path`, `set_max_delay`, `set_clock_groups`, `set_clock_latency`, `set_disable_timing` |
| Object Access | `all_clocks`, `all_inputs`, `all_outputs`, `get_cells`, `get_ports`, `get_clocks`, `get_nets`, `get_pins` |

**Important:** Timing analysis on synthesized design uses estimated routing delays — only post-route timing is accurate.

---

## Attribute Propagation Rules

- Placing attribute on a **hierarchy** affects only its boundary, NOT signals inside (except: `DSP_FOLDING`, `RAM_STYLE`, `ROM_STYLE`, `SHREG_EXTRACT`, `USE_DSP` — these DO affect internal signals)
- If same attribute is set in both RTL and XDC with different values → **XDC wins**
- `KEEP` and `DONT_TOUCH` must be set in RTL (XDC does not support them — the objects would already be optimized away)
