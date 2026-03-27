---
name: vivado-analysis
description: Use this skill when the user needs help with Vivado design analysis, timing report interpretation, or timing closure. This includes report_timing interpretation (slack calculation, path analysis, clock skew/uncertainty), report_timing_summary signoff verification, report_qor_assessment (QoR scoring 1-5, assessment categories), report_qor_suggestions (automated optimization suggestions, .rqs workflow), report_design_analysis (timing path characteristics, complexity/Rent analysis, congestion analysis), report_methodology (design rule compliance), report_utilization (resource usage analysis), report_cdc (clock domain crossing checks), report_drc (design rule checks), report_bus_skew, timing closure strategies (setup/hold violation resolution, congestion mitigation), message severity management, and design check waivers. This skill provides analysis and interpretation knowledge — for TCL command execution use vivado-tcl, for constraint modifications use vivado-constraints, for implementation strategy changes use vivado-impl.
---

# Vivado Design Analysis & Timing Closure Guide

Based on UG906 (v2025.2). This skill helps interpret analysis reports and make timing closure decisions. For complete command syntax, see REFERENCE.md. For TCL execution, use vivado-tcl. For constraint changes, use vivado-constraints. For implementation strategies, use vivado-impl.

## Timing Path Fundamentals

Four common path types:
```
1. Input Port → Register       (constrained by set_input_delay)
2. Register → Register         (constrained by clock period)
3. Register → Output Port      (constrained by set_output_delay)
4. Input Port → Output Port    (combinational, set_max_delay)
```

Path structure (three sections):
```
Source Clock Path → Data Path → Destination Clock Path
(clock source      (launch pin    (clock source
 to launch cell)    to capture     to capture
                    cell input)    cell)
```

Slack formula:
- **Max delay (Setup/Recovery):** `slack = data_required_time - data_arrival_time`
- **Min delay (Hold/Removal):** `slack = data_arrival_time - data_required_time`

## Max/Min Delay Analysis Corner Selection

| Analysis Type | Source Clock | Data Path | Destination Clock |
|--------------|-------------|-----------|-------------------|
| Setup/Recovery (max delay) | Slow_max | Slow_max | Slow_min |
| Setup/Recovery (max delay) | Fast_max | Fast_max | Fast_min |
| Hold/Removal (min delay) | Slow_min | Slow_min | Slow_max |
| Hold/Removal (min delay) | Fast_min | Fast_min | Fast_max |

**Key rule:** Delays from different corners are NEVER mixed on same path during slack calculation.

## Report Selection Guide

| Goal | Command | When to Use |
|------|---------|-------------|
| Quick timing signoff | `report_timing_summary` | After implementation, mandatory before bitstream |
| Analyze specific paths | `report_timing -from/-to/-through` | Debugging specific failing paths |
| Overall QoR score (1-5) | `report_qor_assessment` | After routing, assess closure likelihood |
| Auto optimization hints | `report_qor_suggestions` | When stuck on timing closure |
| Path characteristics | `report_design_analysis` | Deep analysis of logic levels, fanout, physical spread |
| Methodology compliance | `report_methodology` | Early in flow to catch methodology violations |
| Resource usage | `report_utilization` | After synthesis or implementation |
| CDC checks | `report_cdc` | After synthesis, verify clock domain crossings |
| Congestion analysis | `report_design_analysis -congestion` | After placement, if timing degraded |
| Design rule checks | `report_drc` | Before bitstream generation |
| Bus skew | `report_bus_skew` | If set_bus_skew constraints exist (NOT in report_timing_summary) |

## Timing Report Header Fields

| Field | Meaning |
|-------|---------|
| Slack | Positive = meets timing. Negative = violation |
| Source | Startpoint cell + launch clock (edge, name, period) |
| Destination | Endpoint cell + capture clock (edge, name, period) |
| Path Group | Clock group containing endpoint (async pins → async_default) |
| Path Type | Max (setup/recovery) or Min (hold/removal); corner (Slow/Fast) |
| Requirement | Clock period (same clock) or smallest positive delta (different clocks) |
| Data Path Delay | Total delay through logic section |
| Logic Levels | Count of each primitive type in data path |
| Clock Path Skew | Destination - Source insertion delay + CPR |
| CPR | Clock Pessimism Removal — shared clock circuitry correction |
| Clock Uncertainty | TSJ + TIJ + DJ + PE + UU (see below) |

### Clock Uncertainty Components
| Component | Source |
|-----------|--------|
| TSJ (Total System Jitter) | Combined system jitter on both clocks |
| TIJ (Total Input Jitter) | From set_input_jitter constraint |
| DJ (Discrete Jitter) | Hardware primitives (MMCM/PLL) |
| PE (Phase Error) | Phase variation between clock signals |
| UU (User Uncertainty) | From set_clock_uncertainty constraint |

### Delay Type in Path Details
| Value | Meaning |
|-------|---------|
| Unplaced | Cell not placed, delay estimated |
| Estimated | Cell placed but not routed |
| Routed | Final routed delay |

## Clock Phase Shift Mode (Device Defaults)

| Device Family | Default Mode | Behavior |
|--------------|-------------|----------|
| 7 Series | WAVEFORM | Modifies clock waveform edges |
| UltraScale | WAVEFORM | Modifies clock waveform edges |
| UltraScale+ | LATENCY | Models as MMCM/PLL insertion delay |
| Versal | LATENCY | Models as MMCM/PLL insertion delay |

**Warning:** Migrating 7 Series/UltraScale designs to UltraScale+ changes phase shift modeling. Review and remove legacy multicycle path constraints used for phase shift.

## QoR Assessment Interpretation (report_qor_assessment)

### Score Meaning
| Score | Interpretation | Action |
|-------|---------------|--------|
| 1 | Design will likely NOT complete implementation | Major redesign needed |
| 2 | Will complete but will NOT meet timing | Significant optimization required |
| 3 | Will likely NOT meet timing | Targeted optimization needed |
| 4 | Will likely meet timing | Minor adjustments may suffice |
| 5 | Will meet timing | Proceed to bitstream |

### Five Assessment Categories
| Category | What It Checks |
|----------|---------------|
| Utilization | Resource usage across device, SLR, Pblock levels |
| Netlist | Logical structure, DONT_TOUCH properties, high fanout nets |
| Clocking | Clock skew on setup and hold paths |
| Congestion | Netlist structures causing routing congestion |
| Timing | WNS/TNS/WHS/THS per clock group, net/LUT budget |

Each shows **OK** or **REVIEW** status. Items with asterisk (*) don't directly affect score but impact closure.

### ML Strategy Availability
Available when ALL conditions met:
- opt_design ran with Explore or Default directive
- phys_opt_design enabled
- Design fully routed
- UltraScale or UltraScale+ device family

### Auto-Termination
Set `MIN_RQA_SCORE` property (1-5) to auto-terminate runs scoring below threshold.

## QoR Suggestions Workflow

```
report_qor_suggestions          ← Generate suggestions
        ↓
write_qor_suggestions file.rqs  ← Export to file
        ↓
read_qor_suggestions file.rqs   ← Import in next run
        ↓
Suggestions auto-apply          ← If AUTOMATIC=Yes
```

### Suggestion Classification
| Dimension | Values |
|-----------|--------|
| Origin | GENERATED (current run) / EXISTING (imported from .rqs) |
| Status | APPLIED / FAILED TO APPLY |
| Stage generated | opt_design / place_design / phys_opt_design / route_design |
| Stage applicable | Where suggestion should be applied |
| Automatic | Yes (auto-apply) / No (manual action needed) |

### Suggestion Categories
Clocking, Congestion, Utilization, Timing, Netlist, XDC, Strategy

## Design Analysis Interpretation (report_design_analysis)

### Timing Path Characteristics — Five Categories
| Category | Fields |
|----------|--------|
| Timing | Path Type, Requirement, Slack, Timing Exception |
| Logic | Start/End Pin Primitives, Pins, Logic Levels, Routes |
| Physical | Arch Boundary Crossings (IO/RAM/DSP/NOC), Pblock restrictions, Bounding Box, Net Fanout/Detour |
| Property | Combined LUT pairs, MARK_DEBUG, DONT_TOUCH, Fixed constraints |
| DFX | DFX Path Type, Boundary Nets, Boundary Fanout |

### Complexity (Rent Exponent) Interpretation
| Rent Exponent | Complexity | Action |
|--------------|-----------|--------|
| < 0.65 | Low to Normal | No action needed |
| 0.65 - 0.85 | High | Review hierarchy, consider floorplanning |
| > 0.85 | Very High | Redesign hierarchy, reduce connectivity |

**Average Fanout:** < 4 normal, 4-5 placement difficulty, > 5 implementation failure risk.

### Congestion Level Interpretation
| Level | Impact | Action |
|-------|--------|--------|
| 3-4 | Minor | Usually acceptable unless timing budget is tight |
| 5+ | Significant QoR impact | Apply congestion mitigation strategies |

## Timing Closure Decision Tree

### Setup Violation Resolution
```
Setup violation detected
  ├─ Check Logic Levels (report_design_analysis)
  │   └─ High logic levels → Pipeline registers / retiming (vivado-synth: -global_retiming)
  ├─ Check Fanout (report_design_analysis)
  │   └─ High fanout → MAX_FANOUT attribute / phys_opt replication (vivado-impl)
  ├─ Check Physical Spread (report_design_analysis -congestion)
  │   └─ Large bounding box → Pblock floorplanning (vivado-constraints)
  ├─ Check Clock Skew
  │   └─ Negative skew → Review clock tree, BUFG placement
  └─ Check Timing Exception
      └─ Missing/wrong constraint → Fix in XDC (vivado-constraints)
```

### Hold Violation Resolution
```
Hold violation detected
  ├─ Check fast-corner delay
  │   └─ Very short data path → Add delay cells (phys_opt_design hold fix)
  ├─ Check Clock Skew
  │   └─ Large positive skew → Review clock tree balance
  └─ Check Inter-SLR paths (SSI devices)
      └─ SLR crossing → SLR-aware placement (vivado-impl)
```

### Congestion Mitigation
```
Congestion Level ≥ 5
  ├─ Check utilization (report_utilization)
  │   └─ > 80% LUT → Reduce design size or use area-optimized synthesis
  ├─ Check high-fanout nets
  │   └─ Replicate drivers (phys_opt_design fanout optimization)
  ├─ Try congestion-focused strategies
  │   └─ vivado-impl: Congestion_* strategies
  └─ Floorplanning
      └─ Spread logic across device (Pblocks, vivado-constraints)
```

## Message Severity Levels

| Severity | Meaning | Action Required |
|----------|---------|-----------------|
| Status | General processing feedback | None |
| Info | Process/design feedback | None |
| Warning | Constraints not applied as intended, sub-optimal results possible | Review |
| Critical Warning | Input/constraints failing best practices, often leads to errors | Fix recommended |
| Error | Problem stopping design flow | Must fix |

**Tip:** Promote warning severity: `set_msg_config -id "Common 17-81" -new_severity "CRITICAL WARNING"`

## Design Check Waiver System

### When to Waive
- Known-safe CDC crossings with external synchronization
- DRC checks not applicable to your design
- Methodology checks overridden by design intent

### Waiver Wildcards
| Keyword | Matches |
|---------|---------|
| `*CELL` | Any cell |
| `*NET` | Any net |
| `*PIN` | Any pin |
| `*PORT` | Any port |
| `*CLOCK` | Any clock |
| `*` | Any string |

Waivers auto-saved in checkpoints. Export with `write_waivers`, import with `read_xdc` or `source`.

**Cannot delete AMD IP waivers.**

## Synthesis Analysis & Closure Techniques

### RTL Optimization: Integer Range Constraints
Explicitly define signal ranges to reduce logic depth:
```verilog
// Before: 32-bit counter, deep comparator logic
reg [31:0] counter;
// After: range-constrained, smaller comparator
reg [9:0] counter;  // if max value < 1024
```

### Deep Memory Decomposition
| Attribute | Purpose | Effect |
|-----------|---------|--------|
| RAM_DECOMP | Control memory decomposition strategy | power vs area tradeoff |
| CASCADE_HEIGHT | Granular cascading depth control | Limits BRAM cascade chain depth |

### RAMB Utilization (Non-Power-of-2 Depth)
When memory depth is not a power of 2, synthesis may over-allocate BRAMs. Manually partition with address decoder for optimal utilization. Check `report_utilization` and synthesis log for memory mapping.

### RAMB Output Register Inference
Multiple logic levels before BRAM output flip-flop prevent DOA register inference → timing degradation. Restructure RTL to allow BRAM output register usage (see vivado-impl examples/ug906/ for before/after).

## Configurable Report Strategies

| Strategy | Stage | Focus |
|----------|-------|-------|
| Vivado Synthesis Default | Synthesis | Utilization only |
| Vivado Implementation Default | Implementation | Standard reports |
| UltraFast Methodology Reports | Implementation | Methodology compliance |
| Performance Explore Reports | Implementation | Timing exploration |
| Timing Closure Reports | Implementation | Detailed timing analysis |
| No Reports | Both | Skip all reports |
