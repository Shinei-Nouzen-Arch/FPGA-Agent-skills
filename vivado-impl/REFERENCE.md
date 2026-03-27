# Vivado Implementation Complete Reference

Full command syntax, property tables, and advanced flows from UG904 (v2025.2).

## 1. Complete Command Syntax

### opt_design
```tcl
opt_design [-retarget] [-propconst] [-sweep] [-bram_power_opt] [-remap]
           [-aggressive_remap] [-resynth_remap] [-resynth_area] [-resynth_seq_area]
           [-directive <arg>] [-muxf_remap] [-hier_fanout_limit <arg>]
           [-bufg_opt] [-mbufg_opt] [-shift_register_opt] [-dsp_register_opt]
           [-srl_remap_modes <arg>] [-control_set_merge] [-control_set_opt]
           [-merge_equivalent_drivers] [-carry_remap] [-debug_log]
           [-property_opt_only] [-quiet] [-verbose]
```

### place_design (7 Series / UltraScale)
```tcl
place_design [-directive <arg>] [-no_timing_driven] [-timing_summary]
             [-unplace] [-post_place_opt] [-no_psip] [-sll_align_opt]
             [-no_bufg_opt] [-ultrathreads] [-quiet] [-verbose]
```

### place_design (Versal)
```tcl
place_design [-directive <arg>] [-subdirective <args>] [-no_timing_driven]
             [-timing_summary] [-unplace] [-no_psip] [-no_noc_opt]
             [-clock_vtree_type <arg>] [-net_delay_weight <arg>]
             [-quiet] [-verbose]
```

### phys_opt_design
```tcl
phys_opt_design [-fanout_opt] [-placement_opt] [-routing_opt]
                [-slr_crossing_opt] [-insert_negative_edge_ffs]
                [-restruct_opt] [-interconnect_retime] [-lut_opt] [-casc_opt]
                [-cell_group_opt] [-critical_cell_opt] [-dsp_register_opt]
                [-bram_register_opt] [-uram_register_opt] [-bram_enable_opt]
                [-shift_register_opt] [-hold_fix] [-aggressive_hold_fix]
                [-retime] [-force_replication_on_nets <args>]
                [-directive <arg>] [-critical_pin_opt] [-clock_opt]
                [-path_groups <args>] [-tns_cleanup] [-sll_reg_hold_fix]
                [-equ_drivers_opt] [-quiet] [-verbose]
```

### route_design
```tcl
route_design [-unroute] [-release_memory] [-nets <args>]
             [-physical_nets] [-pins <arg>]
             [-directive <arg>] [-tns_cleanup]
             [-no_timing_driven] [-preserve]
             [-delay] [-auto_delay] [-max_delay <arg>]
             [-min_delay <arg>] [-timing_summary] [-finalize]
             [-ultrathreads] [-eco]
             [-quiet] [-verbose]
```

### power_opt_design
```tcl
power_opt_design [-quiet] [-verbose]

# Control scope
set_power_opt [-include_cells <args>] [-exclude_cells <args>]
              [-clocks <args>] [-cell_types <args>] [-quiet] [-verbose]
```

---

## 2. opt_design Property-Based Optimization

| Property | Set On | Description |
|----------|--------|-------------|
| `MUXF_REMAP` | MUXF cells | TRUE → convert MUXF to LUTs |
| `CARRY_REMAP` | CARRY cells | Integer → max carry chain length to convert to LUTs |
| `SRL_TO_REG` | SRL cells | TRUE → convert SRL to register chain |
| `REG_TO_SRL` | Register cells | TRUE → convert register chain to SRL |
| `SRL_STAGES_TO_REG_INPUT` | SRL cells | 1 = pull register from SRL input; -1 = push register into SRL input |
| `SRL_STAGES_TO_REG_OUTPUT` | SRL cells | 1 = pull register from SRL output; -1 = push register into SRL output |
| `LUT_REMAP` | LUT cells | TRUE → collapse cascaded LUTs |
| `CONTROL_SET_REMAP` | Register cells | ENABLE/RESET/ALL/NONE → remap control signals to D-input |
| `EQUIVALENT_DRIVER_OPT` | Register cells | MERGE/KEEP → force/prevent equivalent driver merging |
| `CLOCK_BUFFER_TYPE` | Nets | BUFG/BUFGCE/NONE → insert/suppress clock buffer |
| `LUT_DECOMPOSE` | LUT5/LUT6 cells | TRUE → decompose to reduce congestion |

### OPT_MODIFIED Values
| opt_design Option | OPT_MODIFIED Value |
|-------------------|-------------------|
| `-bufg_opt` | BUFG_OPT |
| `-carry_remap` | CARRY_REMAP |
| `-control_set_merge` | CONTROL_SET_MERGE |
| `-control_set_opt` | CONTROL_SET_OPT |
| `-hier_fanout_limit` | HIER_FANOUT_LIMIT |
| `-merge_equivalent_drivers` | MERGE_EQUIVALENT_DRIVERS |
| `-muxf_remap` | MUXF_REMAP |
| `-propconst` | PROPCONST |
| `-remap` | REMAP |
| `-resynth_remap` | REMAP |
| `-resynth_area` | RESYNTH_AREA |
| `-resynth_seq_area` | RESYNTH_AREA |
| `-retarget` | RETARGET |
| `-shift_register_opt` | SHIFT_REGISTER_OPT |
| `-sweep` | SWEEP |

### SRL Remap Modes
```tcl
# Convert small SRLs to registers (depth ≤ N)
opt_design -srl_remap_modes {{max_depth_srl_to_ffs <depth>}}

# Convert large register chains to SRLs (depth ≥ N)
opt_design -srl_remap_modes {{min_depth_ffs_to_srl <depth>}}

# Auto-balance by utilization target (0-100%)
opt_design -srl_remap_modes {{target_ff_util <ff%> target_lutram_util <lutram%>}}
```

---

## 3. PSIP (Physical Synthesis in Placer) Optimizations

| Optimization | Applied to | PHYS_OPT_MODIFIED Value |
|-------------|-----------|------------------------|
| Autopipeline Insertion | Nets | AUTOPIPELINE |
| Block RAM Register Opt | Cells | BRAM_REGISTER_OPT |
| Control Set Optimization | Cells | CONTROL_SET_OPT |
| Critical Cell Optimization | Cells | CRITICAL_CELL_OPT |
| DSP Register Optimization | Cells | DSP_REGISTER_OPT |
| Equivalent Driver Rewire | Nets | EQU_REWIRE_OPT |
| Fanout Optimization | Nets | FANOUT_OPT |
| Neg-Edge Register Insertion | Cells | INSERT_NEGEDGE |
| Placement Optimization | Cells | PLACEMENT_OPT |
| Property-Based Retiming | Cells | RETIMING |
| Shift Register Optimization | Cells | SHIFT_REGISTER_OPT |
| Shift Register to Pipeline | Cells | SHIFT_REGISTER_TO_PIPELINE |
| SLR Crossing Optimization | Cells | SLR_CROSSING_OPT |
| URAM Register Optimization | Cells | URAM_REGISTER_OPT |
| Very High Fanout Optimization | Nets | FANOUT_OPT |

### PSIP-Related Properties
| Property | Description |
|----------|-------------|
| `FORCE_MAX_FANOUT` | Force replication regardless of slack |
| `MAX_FANOUT_MODE` | CLOCK_REGION / SLR / MACRO — replicate by physical region |
| `PSIP_RETIMING_FORWARD` | TRUE → forward retiming in PSIP |
| `PSIP_RETIMING_BACKWARD` | TRUE → backward retiming in PSIP |
| `PHYS_SRL2PIPELINE` | TRUE → enable SRL to pipeline optimization |
| `USER_SLL_REG` | TRUE → guide SLR crossing FF optimization |

---

## 4. phys_opt_design Post-Place vs Post-Route Defaults

| Optimization | post-place valid | post-place default | post-route valid | post-route default |
|-------------|-----------------|-------------------|-----------------|-------------------|
| Critical Cell | Y (US/US+) | Y | Y (US/US+) | N |
| Fanout | Y (US/US+) | Y | N | N/A |
| Very High Fanout | Y (US/US+) | Y | N | N/A |
| Interconnect Retime | Y (Versal) | Y | Y (Versal) | Y |
| Critical Cell Group | Y (Versal) | Y | Y (Versal) | N |
| Clock | Y (Versal) | Y | Y | Y |
| DSP Register | Y | Y | N | N/A |
| BRAM Register | Y | Y | N | N/A |
| URAM Register | Y | Y | N | N/A |
| Shift Register | Y | Y | N | N/A |
| Critical Pin | Y | Y | Y | Y |
| LUT Restructure | Y | Y | Y | N |
| Single LUT | Y (Versal) | Y | Y (Versal) | Y |
| LUT Cascade | Y (Versal) | Y | Y (Versal) | N |
| Placement | Y (US/US+) | Y | Y (US/US+) | Y |
| Routing | N | N/A | Y | Y |
| BRAM Enable | Y (US/US+) | N | N | N/A |
| Hold-Fixing | Y | N | Y | N |
| Neg-Edge FF Insert | Y (US/US+) | N | N | N/A |
| Laguna Hold-Fix | N | N/A | Y (US/US+) | N |
| Forced Net Replication | Y (Versal) | N | N | N/A |
| SLR-Crossing | Y (US/US+) | Y | Y (US/US+) | Y |
| Equ Driver Rewire | Y (Versal) | N | N | N/A |

---

## 5. Incremental Implementation Reference

### read_checkpoint -incremental Full Syntax
```tcl
read_checkpoint -incremental [-directive <arg>] [-auto_incremental]
                [-force_incr] [-fix_objects <cell_objects>]
                [-reuse_objects <cell_objects>]
                [-quiet] [-verbose] <file>
```

### Incremental Directives
| Directive | Timing Target | Reuse Strategy |
|-----------|--------------|----------------|
| `RuntimeOptimized` | Same as reference WNS | Max reuse (default) |
| `TimingClosure` | Rip up failing paths | Aggressive optimization |
| `Quick` | No timer | Fastest, needs WNS > 1.0 ns |

### report_incremental_reuse Sections
1. **Flow Summary** — Synthesis flow, Auto Incremental, Directive, Target WNS, QoR Suggestions
2. **Reuse Summary** — Cells/Nets/Pins/Ports matched%, initial reuse%, current reuse%, fixed%
3. **Reference Checkpoint Info** — DCP location, Vivado version, recorded WNS/WHS
4. **Comparison with Reference Run** — WNS and runtime at each stage
5. **Optimization Comparison** — iphys_opt_design replay reused/not reused counts
6. **Command Comparison** — Side-by-side command lists

### Auto Incremental Script Pattern
```tcl
# After routing, check if checkpoint is good enough to be reference
if {[get_property SLACK [get_timing_path]] > -0.250} {
    file copy -force <postroute>.dcp <reference>.dcp
}
```

---

## 6. Congestion Analysis

### Congestion Level Meaning
| Level | Window Size | Severity |
|-------|------------|----------|
| 1 | 1x1 tiles | Minimal |
| 3 | 4x4 tiles | Low |
| 5 | 32x32 tiles | **Warning** — timing impact expected |
| 8+ | 256x256+ tiles | **Critical** — router early exit |

### Congestion Report Format
```
INFO: [Route 35-449] Initial Estimated Congestion
       | Global Congestion | Long Congestion  | Short Congestion
Direction| Size | % Tiles  | Size | % Tiles  | Size | % Tiles
  NORTH | 32x32|   0.89   | 32x32|   1.00   | 32x32|   0.66
  SOUTH | 16x16|   0.68   | 32x32|   0.75   | 16x16|   0.53
  EAST  |  4x4 |   0.04   |  8x8 |   0.09   |  4x4 |   0.10
  WEST  |  8x8 |   0.18   |  8x8 |   0.09   | 16x16|   0.50
```

### Router Struggle Symptoms
- Excessive runtimes (hours per iteration)
- Large number of overlaps (hundreds/thousands)
- Setup/hold slacks becoming progressively worse
- Congestion level ≥ 5 warning
- `tight_setup_hold_pins.txt` file generated

---

## 7. Interactive Physical Optimization (iphys_opt_design)

### Write Optimizations to Script
```tcl
# Save phys_opt_design changes
write_iphys_opt_tcl [-place] [-quiet] [-verbose] <output_file>
# -place: include placement info for replay after placement
```

### Read and Replay
```tcl
# Replay before placement (retrofit flow)
read_iphys_opt_tcl [-fanout_opt] [-critical_cell_opt] [-replicate_cell]
                   [-placement_opt] [-restruct_opt] [-forward_retime]
                   [-backward_retime] [-dsp_register_opt] [-bram_register_opt]
                   [-uram_register_opt] [-shift_register_opt]
                   [-shift_register_to_pipeline] [-auto_pipeline]
                   [-pipeline_to_shift_register] [-critical_pin_opt]
                   [-equ_drivers_opt] [-include_skipped_optimizations]
                   [-create_bufg] [-insert_negative_edge_ffs] [-hold_fix]
                   [-slr_crossing_opt] [-quiet] [-verbose] [<input>]
```

### Two Flows

**Retrofit Flow (before placement):**
1. Original run: opt_design → place_design → phys_opt_design → write_iphys_opt_tcl
2. Replay run: opt_design → read_iphys_opt_tcl → place_design → phys_opt_design

**Repeat Flow (after placement):**
1. Original run: opt_design → place_design → phys_opt_design → write_iphys_opt_tcl -place
2. Replay run: opt_design → place_design → read_iphys_opt_tcl -place → phys_opt_design

### iphys_opt_design Command
```tcl
iphys_opt_design [-fanout_opt] [-critical_cell_opt] [-replicate_cell]
                 [-reconnect] [-placement_opt] [-forward_retime]
                 [-backward_retime] [-net <arg>] -cluster <args>
                 -place_cell <args> [-dsp_register_opt]
                 [-bram_register_opt] [-uram_register_opt]
                 [-shift_register_opt] [-cell <arg>]
                 [-packing] [-unpacking] [-port <arg>] [-critical_pin_opt]
                 [-restruct_opt] [-equ_drivers_opt] [-skipped_optimization]
                 [-create_bufg] [-insert_negative_edge_ffs] [-hold_fix]
                 [-slr_crossing_opt] [-shift_register_to_pipeline]
                 [-auto_pipeline] [-pipeline_to_shift_register] [-quiet]
                 [-verbose]
```

---

## 8. ECO (Engineering Change Order) Flow

### Basic ECO Pattern
```tcl
# Open routed checkpoint
open_checkpoint post_route.dcp

# Make netlist changes
create_cell -reference LUT2 new_lut
create_net new_net
connect_net -net new_net -objects {new_lut/O existing_cell/I0}

# Place new cells
place_cell new_lut SLICE_X10Y20/A6LUT

# Route with ECO mode
route_design -eco
# Or finalize partial routes
route_design -finalize

# Verify
report_route_status
report_timing_summary
```

### ECO with Incremental Implementation
```tcl
# After ECO changes, use -eco for faster routing
route_design -eco
# For UltraScale+ with register changes
route_design -finalize
```

## QoR Suggestions — RTL Optimization Examples (UG906)

Before/after RTL examples from `report_qor_suggestions` in `examples/ug906/`. Read the specific before/after pair when the user encounters these RQS IDs.

| RQS ID | Files | Problem | Fix |
|--------|-------|---------|-----|
| TIMING-201 | `TIMING-201/before/single_sdp_ram.sv` → `after/single_sdp_ram.sv` | RAM LOW_LATENCY → long clock-to-out | Change to HIGH_PERFORMANCE, enable output register |
| TIMING-202 | `TIMING-202/before/wide_mulitplier.sv` → `after/wide_mulitplier.sv` | Wide multiplier critical path | Add pipeline register stages |
| UTIL-203 | `UTIL-203/before/sp_rom.v` → `after/sp_rom.sv` | Wide/shallow ROM uses excess LUTs | Restructure to narrow/deep ROM for BRAM inference |
