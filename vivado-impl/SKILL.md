---
name: vivado-impl
description: Use this skill when the user needs help with Vivado implementation strategy selection and optimization. This includes opt_design (logic optimization directives and options), place_design (placement directives, congestion analysis, PSIP physical synthesis), phys_opt_design (physical optimization - fanout/placement/routing/SLR crossing/register optimization, hold fixing), route_design (routing directives, congestion resolution, pre-routing critical nets), power_opt_design (clock gating, BRAM power optimization), incremental implementation (read_checkpoint -incremental, auto_incremental, reuse analysis), ECO flow, implementation run strategies (Performance/Congestion/Area strategies), or any question about choosing between implementation directives or resolving timing/congestion issues during implementation. This skill provides decision-making knowledge for the implementation phase — for TCL command execution use vivado-tcl, for synthesis use vivado-synth, for constraints use vivado-constraints, for timing report interpretation and analysis use vivado-analysis.
---

# Vivado Implementation Decision Guide

Based on UG904 (v2025.2). For command syntax and property tables see REFERENCE.md; for report_qor_suggestions RTL optimization examples (UG906 before/after) see `examples/ug906/` directory.

## Implementation Flow Overview

```
opt_design          ← Logic optimization (REQUIRED)
  ↓
power_opt_design    ← Clock gating power opt (OPTIONAL, not Versal)
  ↓
place_design        ← Placement (REQUIRED)
  ↓
power_opt_design    ← Post-place power opt (OPTIONAL)
  ↓
phys_opt_design     ← Post-place physical opt (OPTIONAL, recommended)
  ↓
route_design        ← Routing (REQUIRED)
  ↓
phys_opt_design     ← Post-route physical opt (OPTIONAL)
  ↓
write_bitstream     ← Bitstream (all except Versal)
write_device_image  ← Device image (Versal only)
```

**Key rule:** All implementation commands are **re-entrant** — they can be run repeatedly on the same design. Each run optimizes the results of the previous run.

---

## opt_design — Logic Optimization

### Directive Decision Table

| Scenario | Directive | Effect |
|----------|-----------|--------|
| Default / first try | `Default` | Default optimization phases |
| Deep exploration | `Explore` | Multiple passes of optimization |
| Area reduction (comb) | `ExploreArea` | Multiple passes, emphasis on reducing combinational logic |
| Area reduction (comb+seq) | `ExploreSequentialArea` | Reduces both combinational and sequential logic |
| Explore + LUT remap | `ExploreWithRemap` | Explore + Remap optimization |
| Fast iteration | `RuntimeOptimized` | Minimal optimization passes |
| QoR-suggested | `RQS` | Uses report_qor_suggestion strategy |

### Available Optimizations (18 phases)

| Phase | Option | Default | Description |
|-------|--------|---------|-------------|
| 1 | `-retarget` | ON | Retarget primitives across device families |
| 2 | `-propconst` | ON | Constant propagation |
| 3 | `-sweep` | ON | Remove loadless cells, tie-off, retarget dual-port→single-port RAM |
| 4 | `-muxf_remap` | off | Remap MUXF7/F8/F9 to LUT3 for routability |
| 5 | `-carry_remap` | off | Remap short CARRY chains to LUTs |
| 6 | `-control_set_merge` | off | Merge equivalent control set drivers |
| 7 | `-merge_equivalent_drivers` | off | Merge all equivalent drivers (not just control) |
| 8 | `-bufg_opt` | ON | Insert BUFG on high-fanout clock/non-clock nets |
| 9 | `-shift_register_opt` | ON | SRL fanout opt + SRL↔register transforms |
| 10 | `-mbufg_opt` | off | Replace parallel BUFGCEs with MBUFG (Versal) |
| 11 | `-dsp_register_opt` | off | Optimize DSP pipeline registers |
| 12 | (auto) | ON | Control Set Reduction (CONTROL_SET_REMAP property) |
| 13 | `-hier_fanout_limit <N>` | off | Module-based fanout replication |
| 13 | `-control_set_opt` | off | Control Set Optimization (auto-selected candidates) |
| 14 | `-remap` | off | Combine cascaded LUTs to reduce logic levels |
| 15 | `-resynth_remap` | off | Timing-driven re-synthesis + remap |
| 16 | `-resynth_area` | off | Re-synthesis for area (reduce LUTs) |
| 17 | `-resynth_seq_area` | off | Re-synthesis for area (comb + sequential) |
| 18 | `-bram_power_opt` | ON | Block RAM power optimization (WRITE_MODE) |

**IMPORTANT:** Specifying individual options disables ALL default options. To run defaults + extras: `opt_design -retarget -propconst -sweep -bufg_opt -shift_register_opt -bram_power_opt -remap`

---

## place_design — Placement

### Directive Decision Table

| Scenario | Directive | Designs Benefited |
|----------|-----------|-------------------|
| Default | `Default` | All |
| Deep exploration | `Explore` | All (higher effort detail placement) |
| Aggressive exploration | `AggressiveExplore` | Timing-critical designs |
| RAM/DSP dense | `WLDrivenBlockPlacement` | Many BRAM/DSP blocks |
| RAM/DSP dense | `EarlyBlockPlacement` | RAM/DSP as placement anchors |
| Timing meets post-place but fails post-route | `ExtraNetDelay_high` | Long-distance nets, high fanout |
| Timing meets post-place but fails post-route | `ExtraNetDelay_low` | Same, lower pessimism |
| Congestion | `AltSpreadLogic_high` | High connectivity → congestion |
| Congestion (moderate) | `AltSpreadLogic_medium/low` | Moderate congestion |
| SSI congestion | `SSI_SpreadLogic_high/low` | SSI devices |
| SSI SLR balancing | `SSI_SpreadSLLs` | Balance SLL connections across SLRs |
| SSI SLR balancing | `SSI_BalanceSLLs` | Balance SLLs between SLRs |
| SSI SLR balancing | `SSI_BalanceSLRs` | Balance cell count between SLRs |
| SSI high utilization | `SSI_HighUtilSLRs` | Pack logic closer in each SLR |
| Extra post-place opt | `ExtraPostPlacementOpt` | All |
| Alternate timing | `ExtraTimingOpt` | Alternative timing-driven algorithms |
| ML-predicted best | `Auto_1` | Highest confidence ML prediction |
| ML-predicted 2nd | `Auto_2` | Second best ML prediction |
| ML-predicted 3rd | `Auto_3` | Third best ML prediction |
| Fast iteration | `RuntimeOptimized` | Trade QoR for speed |
| Fastest | `Quick` | Non-timing-driven, minimum legal placement |
| QoR-suggested | `RQS` | Uses report_qor_suggestion |

### Key Options
| Option | Effect |
|--------|--------|
| `-post_place_opt` | Extra timing optimization after placement |
| `-timing_summary` | Force STA-based timing summary (more accurate, slower) |
| `-no_timing_driven` | Wirelength-only placement (fastest) |
| `-no_psip` | Disable Physical Synthesis in Placer |
| `-no_bufg_opt` | Disable BUFG insertion during placement |
| `-sll_align_opt` | Align SLL registers for SSI multi-die parts |
| `-ultrathreads` | Parallel placement across SLRs (UltraScale+ SSI) |
| `-unplace` | Remove all non-fixed placements |

---

## phys_opt_design — Physical Optimization

### Two Modes
- **Post-place:** More aggressive, based on placement timing estimates
- **Post-route:** More conservative, uses actual routed delays, auto-updates routing

**TIP:** Post-route phys_opt is most effective on designs with few failing paths (WNS > -0.200 ns). Designs with > 200 failing endpoints see little improvement.

### Directive Decision Table

| Scenario | Directive | Effect |
|----------|-----------|--------|
| Default | `Default` | Default optimizations |
| Deep exploration | `Explore` | Multi-pass + SLR crossing + critical path final phase |
| Explore + hold fix | `ExploreWithHoldFix` | Explore + hold violation fixing |
| Explore + aggressive hold | `ExploreWithAggressiveHoldFix` | Explore + aggressive hold fixing |
| Most aggressive | `AggressiveExplore` | Allows WNS degradation in SLR crossing opt |
| Alternative replication | `AlternateReplication` | Different critical cell replication algorithm |
| Fanout-focused | `AggressiveFanoutOpt` | Aggressive fanout optimization |
| Add retiming | `AddRetime` | Default flow + register retiming |
| Aggressive + retiming | `AlternateFlowWithRetiming` | Aggressive replication + DSP/BRAM opt + retiming |
| Fast | `RuntimeOptimized` | Fewest iterations |

### Key Individual Options

**Setup optimization (post-place defaults):**
| Option | Description |
|--------|-------------|
| `-fanout_opt` | Replicate high-fanout net drivers (default post-place) |
| `-critical_cell_opt` | Replicate cells in failing paths (default post-place) |
| `-placement_opt` | Re-place critical path cells (default both modes) |
| `-dsp_register_opt` | Move registers in/out of DSP cells |
| `-bram_register_opt` | Move registers in/out of BRAM cells |
| `-uram_register_opt` | Move registers in/out of UltraRAM cells |
| `-shift_register_opt` | Extract SRL end stages to improve timing |
| `-restruct_opt` | Swap LUT connections to reduce logic levels |
| `-lut_opt` | Single LUT movement/replication |
| `-clock_opt` | Useful clock skew optimization |

**Routing optimization (post-route defaults):**
| Option | Description |
|--------|-------------|
| `-routing_opt` | Re-route critical nets/pins (default post-route) |
| `-slr_crossing_opt` | Optimize inter-SLR paths (default both modes) |
| `-critical_pin_opt` | Remap LUT pins to faster physical pins |

**Hold fixing:**
| Option | Description |
|--------|-------------|
| `-hold_fix` | Fix hold violations above threshold |
| `-aggressive_hold_fix` | Fix more hold violations |
| `-sll_reg_hold_fix` | SLL register hold fix (UltraScale+) |
| `-insert_negative_edge_ffs` | Insert neg-edge FFs to split hold paths |

**Other:**
| Option | Description |
|--------|-------------|
| `-retime` | Register retiming (Versal) |
| `-interconnect_retime` | Interconnect retiming by FF movement (Versal) |
| `-force_replication_on_nets <nets>` | Force driver replication on specific nets |
| `-equ_drivers_opt` | Rewire loads to equivalent drivers |
| `-casc_opt` | LUT cascade optimization (Versal) |
| `-cell_group_opt` | Critical fanin cone group opt (Versal) |
| `-bram_enable_opt` | Reverse BRAM power opt on timing-critical enable paths |
| `-path_groups <args>` | Limit optimization to specific path groups |
| `-tns_cleanup` | Allow slack degradation if WNS maintained (with -slr_crossing_opt) |

---

## route_design — Routing

### Directive Decision Table

| Scenario | Directive | Effect |
|----------|-----------|--------|
| Default | `Default` | Default routing |
| Explore alternatives | `Explore` | Explore different critical path routes (signoff timing) |
| Aggressive exploration | `AggressiveExplore` | More aggressive thresholds |
| No timing relaxation | `NoTimingRelaxation` | Never relax timing goals |
| More iterations | `MoreGlobalIterations` | Detailed timing analysis all stages |
| Emphasize delay | `HigherDelayCost` | Trade compile time for delay optimization |
| Fast | `RuntimeOptimized` | Fewest iterations |
| Congestion | `AlternateCLBRouting` | Alternate CLB routing algorithms |
| Fastest | `Quick` | Non-timing-driven, minimum legal routing |
| QoR-suggested | `RQS` | Uses report_qor_suggestion |

### Key Options
| Option | Effect |
|--------|--------|
| `-tns_cleanup` | Focus on WNS, fix non-critical failing paths. Use before post-route phys_opt |
| `-preserve` | Preserve existing routes, route remaining. For pre-routing critical nets |
| `-nets <net_objects>` | Route only specified nets |
| `-pins <pin_objects>` | Route only specified pins |
| `-delay` | Route individual nets with smallest delay |
| `-auto_delay` | Route with timing-constraint-driven budgets (use with -nets/-pins) |
| `-max_delay <ps>` / `-min_delay <ps>` | Target delay for pin routing (use with -pins) |
| `-unroute` | Remove routing (entire design or specific nets/pins) |
| `-timing_summary` | Force STA timing summary (more accurate) |
| `-finalize` | Complete partially routed connections (ECO flow) |
| `-eco` | Incremental ECO routing (faster after small changes) |
| `-ultrathreads` | Parallel routing (faster, slight variation between runs) |
| `-no_timing_driven` | Disable timing-driven routing (feasibility test only) |

### Pre-routing Critical Nets Pattern
```tcl
# Route top 10 critical nets first with minimum delay
set preRoutes [get_nets -of [get_timing_paths -max_paths 10]]
route_design -nets [get_nets $preRoutes] -delay
# Then route rest preserving critical routes
route_design -preserve
```

---

## power_opt_design — Power Optimization

```tcl
# Basic usage (optimize entire design)
power_opt_design

# Control scope
set_power_opt -include_cells [get_cells inst_A]
set_power_opt -exclude_cells [get_cells inst_B]
set_power_opt -cell_types {BRAM}
set_power_opt -clocks {clk1}
```

**Note:** Not supported for Versal. BRAM power opt is skipped if already done by opt_design.

---

## Incremental Implementation

### Setup
```tcl
# Non-Project Mode
read_checkpoint -incremental <reference_routed.dcp>
# Automatic incremental (recommended)
read_checkpoint -incremental -auto_incremental <reference.dcp>
# Force incremental even if criteria not met
read_checkpoint -incremental -force_incr <reference.dcp>
# Fix specific objects
read_checkpoint -incremental <ref.dcp> -fix_objects [all_rams]
```

### Incremental Directives
```tcl
read_checkpoint -incremental -directive <directive> <ref.dcp>
```
| Directive | Effect |
|-----------|--------|
| `RuntimeOptimized` | Reuse max, target same WNS as reference (default) |
| `TimingClosure` | Rip up failing paths, try harder to close timing |
| `Quick` | No timer, fastest, needs WNS > 1.0 ns |

### Auto Incremental Criteria
- Cell matching ≥ 94%
- Net matching ≥ 90%
- Reference WNS ≥ -0.250 ns

### Analysis
```tcl
report_incremental_reuse -file incr_reuse.rpt
```

**Note:** Not supported for Versal.

---

## Congestion Analysis & Resolution

### Congestion Levels
- **Level 5** (32x32 tiles) — warning threshold, expect timing impact
- **Level 8+** — router early exit, design likely unroutable

### Diagnosis
```tcl
report_design_analysis -congestion -file congestion.rpt
report_route_status -file route_status.rpt
```

### Resolution Decision Tree
1. **Synthesis:** Use `AlternateRoutability` directive, reduce MUXF/CARRY usage
2. **opt_design:** Use `-muxf_remap`, `-carry_remap`, `LUT_DECOMPOSE` property
3. **place_design:** Use `AltSpreadLogic_high` or `SSI_SpreadLogic_high`
4. **route_design:** Use `AlternateCLBRouting`
5. **RTL changes:** Reduce fanout, pipeline long paths, reduce resource utilization

---

## Predefined Implementation Strategies (Project Mode)

### Performance-focused
`Performance_Auto_1/2/3`, `Performance_Explore`, `Performance_ExplorePostRoutePhysOpt`, `Performance_ExploreWithRemap`, `Performance_WLBlockPlacement`, `Performance_WLBlockPlacementFanoutOpt`, `Performance_EarlyBlockPlacement`, `Performance_NetDelay_high/low`, `Performance_Retiming`, `Performance_ExtraTimingOpt`, `Performance_RefinePlacement`, `Performance_SpreadSLLs`, `Performance_BalanceSLLs`, `Performance_BalanceSLRs`, `Performance_HighUtilSLRs`

### Congestion-focused
`Congestion_SpreadLogic_high/medium/low`, `Congestion_SSI_SpreadLogic_high/low`

### Area-focused
`Area_Explore`, `Area_ExploreSequential`, `Area_ExploreWithRemap`
