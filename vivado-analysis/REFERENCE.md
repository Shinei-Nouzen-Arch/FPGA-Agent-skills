# Vivado Design Analysis Complete Reference

Full command syntax and reference tables from UG906 (v2025.2).

## 1. report_timing Complete Syntax

```tcl
report_timing [-from <args>] [-rise_from <args>] [-fall_from <args>]
              [-to <args>] [-rise_to <args>] [-fall_to <args>]
              [-through <args>] [-rise_through <args>] [-fall_through <args>]
              [-delay_type <arg>] [-setup] [-hold]
              [-max_paths <arg>] [-nworst <arg>] [-unique_pins]
              [-sort_by <arg>] [-input_pins] [-no_header]
              [-path_type <arg>] [-slack_lesser_than <arg>]
              [-slack_greater_than <arg>]
              [-group <args>] [-no_report_unconstrained]
              [-of_objects <args>] [-cell <args>]
              [-significant_digits <arg>]
              [-column_style <arg>]
              [-file <arg>] [-append] [-name <arg>]
              [-rpx <arg>] [-return_string]
              [-routed_nets] [-no_detour]
              [-quiet] [-verbose]
```

### Common Patterns
```tcl
# Quick: 10 worst setup paths
report_timing -max_paths 10 -sort_by slack

# Specific path
report_timing -from [get_cells src_reg] -to [get_cells dst_reg]

# Hold analysis
report_timing -delay_type min -max_paths 10

# Through specific net
report_timing -through [get_nets critical_net]

# Only paths with negative slack
report_timing -slack_lesser_than 0 -max_paths 100

# Both setup and hold
report_timing -delay_type min_max -max_paths 10

# Group-specific
report_timing -group clk_100 -max_paths 20

# Detailed path with input pins
report_timing -input_pins -max_paths 1
```

## 2. report_timing_summary Syntax

```tcl
report_timing_summary [-delay_type <arg>] [-no_detailed_paths]
                      [-setup] [-hold] [-max_paths <arg>]
                      [-nworst <arg>] [-significant_digits <arg>]
                      [-path_type <arg>] [-no_header]
                      [-check_timing_verbose]
                      [-report_unconstrained]
                      [-datasheet]
                      [-cells <args>] [-input_pins]
                      [-routable_nets] [-unique_pins]
                      [-slack_lesser_than <arg>]
                      [-file <arg>] [-append] [-name <arg>]
                      [-rpx <arg>] [-return_string]
                      [-quiet] [-verbose]
```

**IMPORTANT:** Does NOT include bus skew constraints. Run `report_bus_skew` separately.

### Timing Summary Report Sections
| Section | Contents |
|---------|----------|
| General Information | Design name, device, speed grade, Vivado version, Tcl options used |
| Timer Settings | Multi-corner config, pessimism removal, flight delays, preset/clear arcs |
| Design Timing Summary | WNS/TNS (setup), WHS/THS (hold), WPWS/TPWS (pulse width) |
| Clock Summary | All clocks: name, period, waveform, frequency |
| Methodology Summary | Violations from most recent report_methodology run |
| Check Timing | Missing/incomplete constraints (no_clock, no_input_delay, etc.) |
| Intra-Clock Paths | Worst slack per clock domain (same source and destination clock) |
| Inter-Clock Paths | Worst slack between different clock domains |
| Other Path Groups | User-defined path groups |
| User Ignored Paths | Paths excluded by false_path or similar |
| Unconstrained Paths | Paths with no timing requirement |

### Design Timing Summary Metrics
| Metric | Description |
|--------|-------------|
| WNS (Worst Negative Slack) | Worst slack across all max delay (setup) paths |
| TNS (Total Negative Slack) | Sum of all WNS violations per endpoint |
| WHS (Worst Hold Slack) | Worst slack across all min delay (hold) paths |
| THS (Total Hold Slack) | Sum of all WHS violations per endpoint |
| WPWS (Worst Pulse Width Slack) | Worst slack for pulse width checks |
| TPWS (Total Pulse Width Slack) | Sum of all WPWS violations per pin |

### Check Timing Categories
| Check | Description |
|-------|-------------|
| `pulse_width_clock` | Clock pins with only pulse width check, no setup/hold |
| `no_input_delay` | Non-clock input ports missing input delay constraints |
| `no_clock` | Clock pins not reached by a defined timing clock |
| `constant_clock` | Clock signals connected to GND/VCC/static data |
| `unconstrained_internal_endpoints` | Path endpoints with no timing requirement |
| `no_output_delay` | Non-clock output ports without output delay constraint |
| `multiple_clock` | Clock pins reached by more than one timing clock |
| `generated_clocks` | Generated clocks with master source not in same clock tree |
| `loops` | Combinational loops (Vivado auto-breaks) |
| `partial_input_delay` | Input ports with only min or max input delay |
| `partial_output_delay` | Output ports with only min or max output delay |
| `latch_loops` | Loops through latches affecting time borrowing |

## 3. report_qor_assessment Syntax

```tcl
report_qor_assessment [-max_paths <arg>]
                      [-full_assessment_details]
                      [-csv_output_dir <arg>]
                      [-exclude_methodology_checks]
                      [-file <arg>] [-append] [-name <arg>]
                      [-rpx <arg>] [-return_string]
                      [-quiet] [-verbose]
```

### Auto-Termination Property
```tcl
# Stop runs scoring below threshold
set_property MIN_RQA_SCORE 3 [current_run]

# For non-project mode, manually check:
set rqa_score [get_qor_assessment]
```

## 4. report_qor_suggestions Syntax

```tcl
report_qor_suggestions [-max_paths <arg>]
                       [-max_strategies <arg>]
                       [-report_all_suggestions]
                       [-of_objects <args>]
                       [-cells <args>]
                       [-csv_output_dir <arg>]
                       [-file <arg>] [-append] [-name <arg>]
                       [-rpx <arg>] [-return_string]
                       [-quiet] [-verbose]
```

### Suggestions Workflow Commands
```tcl
# Generate and view suggestions
report_qor_suggestions -name qor_suggestions_1

# Export suggestions to file
write_qor_suggestions suggestions.rqs
write_qor_suggestions -strategy_dir ./ml_strategies suggestions.rqs
write_qor_suggestions -disable_dont_touch suggestions.rqs

# Import suggestions in next run
read_qor_suggestions suggestions.rqs
```

### Suggestion Categories
| Category | Evaluates |
|----------|-----------|
| Clocking | Clock skew issues |
| Congestion | Routing congestion structures |
| Utilization | Resource usage levels |
| Timing | Critical timing paths |
| Netlist | DONT_TOUCH, high fanout nets |
| XDC | Constraint issues |
| Strategy | ML-based implementation run strategies |

### Suggestion Attributes
| Attribute | Description |
|-----------|-------------|
| GENERATED_AT | Design stage where suggestion was created (e.g. opt_design) |
| APPLICABLE_FOR | Stage where suggestion must be applied |
| SOURCE | RQS file or current_run |
| AUTOMATIC | Yes/No — whether Vivado auto-applies |
| SUGGESTION_SCOPE | GLOBALSCOPE or OOC top module |

## 5. report_design_analysis Complete Syntax

```tcl
report_design_analysis [-name <arg>]
                       [-file <arg>] [-append]
                       [-return_string]
                       # Timing options:
                       [-timing] [-max_paths <arg>]
                       [-setup] [-hold] [-delay_type <arg>]
                       [-logic_level_distribution]
                       [-logic_level_dist_paths <arg>]
                       [-logic_levels <arg>]
                       [-max_level <arg>] [-min_level <arg>]
                       [-end_point_clock <arg>]
                       [-routed_vs_estimated]
                       [-extend_analysis]
                       [-of_timing_paths <args>]
                       [-return_timing_paths]
                       [-csv <arg>]
                       # Complexity options:
                       [-complexity]
                       [-hierarchical_depth <arg>]
                       [-rent_greater_than <arg>]
                       [-instances_greater_than <arg>]
                       [-instances_lesser_than <arg>]
                       [-av_fanout_greater_than <arg>]
                       [-bounding_boxes]
                       # Congestion options:
                       [-congestion]
                       [-min_congestion_level <arg>]
                       # QoR summary:
                       [-qor_summary]
                       [-json <arg>]
                       # General:
                       [-cells <args>]
                       [-disable_flight_delays]
                       [-quiet] [-verbose]
```

### Common Patterns
```tcl
# Full timing analysis (10 worst paths per group)
report_design_analysis -timing -max_paths 10

# Logic level distribution
report_design_analysis -timing -logic_level_distribution

# Complexity analysis
report_design_analysis -complexity -hierarchical_depth 3

# Congestion analysis (level 5+)
report_design_analysis -congestion -min_congestion_level 5

# Combined timing + congestion
report_design_analysis -timing -congestion -max_paths 20

# QoR summary as JSON
report_design_analysis -qor_summary -json qor_summary.json

# CSV export for external tools
report_design_analysis -timing -csv timing_analysis.csv

# Extend analysis: show worst path to startpoint and from endpoint
report_design_analysis -timing -extend_analysis -max_paths 10
```

### Complexity Field Defaults
| Option | Default Value |
|--------|--------------|
| `-hierarchical_depth` | 2 |
| `-rent_greater_than` | 0.6 |
| `-instances_greater_than` | 15,000 |
| `-instances_lesser_than` | 100,000 |
| `-av_fanout_greater_than` | 3.0 |

## 6. report_methodology Syntax

```tcl
report_methodology [-name <arg>]
                   [-cells <args>]
                   [-checks <args>]
                   [-file <arg>] [-append]
                   [-return_string]
                   [-rpx <arg>]
                   [-quiet] [-verbose]
```

Three check stages: RTL lint → Netlist-based → Implementation

## 7. report_utilization Syntax

```tcl
report_utilization [-name <arg>]
                   [-cells <args>]
                   [-exclude_cells <args>]
                   [-pblocks <args>]
                   [-exclude_child_pblocks]
                   [-exclude_non_assigned]
                   [-slr]
                   [-packthru]
                   [-hierarchical]
                   [-hierarchical_depth <arg>]
                   [-hierarchical_percentage]
                   [-hierarchical_min_primitive_count <arg>]
                   [-spreadsheet_table <args>]
                   [-spreadsheet_depth <arg>]
                   [-file <arg>] [-append]
                   [-return_string]
                   [-quiet] [-verbose]
```

### Output Sections (UltraScale/UltraScale+)
| Section | Contents |
|---------|----------|
| Netlist Logic | LUT, MuxFx, Register, LUT as memory, LUT-FF pairs, Carry |
| CLB Distribution | Slice details, LUT combining (O5/O6), control sets |
| BLOCKRAM | BlockRAM, UltraRAM, FIFO |
| ARITHMETIC | DSP resources |
| I/O Resources | IOB, ISERDES, OSERDES |
| Clocking Resources | BUFG, MMCM, PLL |
| Device-Specific | STARTUPE2, XADC, etc. |
| Primitive Type Count | Sorted by usage |
| SLR Crossing Utilization | Cross-SLR resources |

### SLR Utilization Tables (with -slr option)
| Table | Contents |
|-------|----------|
| SLR Connectivity | Cross-SLR connections (TX/RX per direction) |
| SLR Connectivity Matrix | Directional crossing counts between SLR pairs |
| SLR CLB Logic and Dedicated Block Utilization | Per-SLR resource usage |
| SLR IO Utilization | Per-SLR I/O usage |

### Pblock Utilization Columns (with -pblocks option)
| Column | Description |
|--------|-------------|
| Parent | Resources assigned only to the parent Pblock |
| Child | Resources assigned only to child Pblocks |
| Non-Assigned | Resources in the Pblock area but not assigned to parent or children |
| Used | Total resources used in the Pblock area |
| Fixed | Resources fixed by LOC constraints |
| Available | Total resources in the Pblock area |
| Util% | Used / Available |

## 8. report_cdc Syntax

```tcl
report_cdc [-name <arg>]
           [-from <args>] [-to <args>]
           [-cells <args>]
           [-severity <args>]
           [-details]
           [-summary]
           [-file <arg>] [-append]
           [-return_string]
           [-quiet] [-verbose]
```

## 9. report_bus_skew Syntax

```tcl
report_bus_skew [-name <arg>]
                [-max_paths <arg>]
                [-significant_digits <arg>]
                [-file <arg>] [-append]
                [-return_string]
                [-rpx <arg>]
                [-quiet] [-verbose]
```

## 10. report_drc Syntax

```tcl
report_drc [-name <arg>]
           [-checks <args>] [-ruledecks <args>]
           [-file <arg>] [-append]
           [-return_string]
           [-rpx <arg>]
           [-quiet] [-verbose]
```

## 11. create_waiver Complete Syntax

```tcl
# From violation objects
create_waiver -of_objects <violations> -description <string> [-user <name>]

# From command line
create_waiver -id <check_ID> -type <CDC|DRC|Methodology>
              -objects <objects> -string <strings>
              -description <string> [-user <name>]
```

### Waiver Wildcards
| Keyword | Matches |
|---------|---------|
| `*CELL` | Any cell |
| `*NET` | Any net |
| `*PIN` | Any pin |
| `*PORT` | Any port |
| `*SITE` | Any site |
| `*TITLE` | Any title |
| `*BEL` | Any BEL |
| `*PKGBANK` | Any package bank |
| `*CLKREGION` | Any clock region |
| `*CLOCK` | Any clock |
| `*PBLOCK` | Any Pblock |
| `*` | Any string |

### Waiver Management
```tcl
get_waivers [-type <CDC|DRC|Methodology>] [-filter <expr>]
delete_waivers [<waivers>]
write_waivers <filename>
# Import: read_xdc <filename> or source <filename>
```

## 12. Dataflow Analysis Commands

```tcl
# Create simplified netlist for dataflow analysis
create_dataflow_design -min_bus_width <value>   ;# default: 16

# Extract dataflow paths
get_dataflow_paths [-from <cells>] [-to <cells>] [-through <cells>]
                   [-max_depth <integer>]    ;# default: 10
                   [-max_paths <integer>]    ;# default: 100
                   [-min_width <integer>]    ;# default: 1
```

## 13. Message Configuration

```tcl
# Change message severity
set_msg_config -id <msg_id> -new_severity <severity>

# Example: Promote to critical warning
set_msg_config -id "Common 17-81" -new_severity "CRITICAL WARNING"

# Suppress message
set_msg_config -id <msg_id> -suppress
```

## 14. Clock Phase Shift Device Defaults

| Device Family | Default PHASESHIFT_MODE | Phase Shift Modeling |
|--------------|------------------------|---------------------|
| 7 Series | WAVEFORM | Clock waveform edge modification |
| UltraScale | WAVEFORM | Clock waveform edge modification |
| UltraScale+ | LATENCY | MMCM/PLL insertion delay |
| Versal | LATENCY | MMCM/PLL insertion delay |

```tcl
# Override phase shift mode
set_property PHASESHIFT_MODE {WAVEFORM|LATENCY} [get_cells mmcm_inst]
```

## 15. QoR Assessment Score Reference

| Score | Meaning | Typical Action |
|-------|---------|---------------|
| 1 | Design will likely NOT complete implementation | Major RTL redesign, reduce utilization |
| 2 | Design will complete but NOT meet timing | Significant optimization, strategy changes |
| 3 | Design will likely NOT meet timing | Targeted fixes, try alternative strategies |
| 4 | Design will likely meet timing | Minor constraint/strategy adjustments |
| 5 | Design will meet timing | Proceed to bitstream |

### Assessment Categories
| Category | Checks | Threshold Behavior |
|----------|--------|--------------------|
| Utilization | Device/SLR/Pblock resource levels | REVIEW if exceeded |
| Netlist | DONT_TOUCH, high fanout > 10K | REVIEW if present |
| Clocking | Setup/hold clock skew | REVIEW if excessive |
| Congestion | Routing congestion structures | REVIEW if level >= 5 |
| Timing | WNS/TNS/WHS/THS, net/LUT budget | REVIEW if negative |

### ML Strategy Availability Requirements
- opt_design directive: Default or Explore
- All implementation directives: consistently Default or Explore (no mixing)
- phys_opt_design: must be enabled
- Design must be fully routed
- Target device: UltraScale or UltraScale+ only

## 16. CDC Rules Priority (Partial)

| Rule | Description | Severity | Safety |
|------|-------------|----------|--------|
| CDC-18 | Synchronized with HARD_SYNC | Info | Safe |
| CDC-13 | 1-bit CDC on non-FD primitive | Critical | Unsafe |
| CDC-14 | Multi-bit CDC on non-FD primitive | Critical | Unsafe |
| CDC-17 | MUX Hold Type | Warning | Safe |
| CDC-16 | MUX Type | Warning | Safe |

## 17. Predefined Report Strategies

| Strategy Name | Stage | Reports Included |
|--------------|-------|-----------------|
| Vivado Synthesis Default Reports | Synthesis | Utilization only |
| Vivado Implementation Default Reports | Implementation | Standard set |
| UltraFast Design Methodology Reports | Implementation | Methodology compliance |
| Performance Explore Reports | Implementation | Timing exploration |
| Timing Closure Reports | Implementation | Detailed timing analysis |
| No Reports | Both | None |

## 18. Timer Settings Reference

| Setting | Description | Default |
|---------|-------------|---------|
| Enable Multi-Corner Analysis | Timing analysis for all configured corners | Yes |
| Enable Pessimism Removal | Remove clock skew at common node | Yes (always keep enabled) |
| Enable Input Delay Default Clock | Apply null input delay to unconstrained input ports | No |
| Enable Preset / Clear Arcs | Timing through async control pins | No |
| Disable Flight Delays | Exclude package delays from I/O timing | No |

### Multi-Corner Delay Types
| Value | Description |
|-------|-------------|
| `none` | Disable timing for that corner |
| `max` | Setup and recovery checks (max delay) |
| `min` | Hold and removal checks (min delay) |
| `min_max` | Both min and max delays (recommended) |

### Interconnect Delay Model (set_delay_model)
| Mode | Description | Used For |
|------|-------------|----------|
| Estimated | Based on ideal placement, driver characteristics, fanout | Post-synthesis |
| Actual | Real hardware delay from routed nets | Post-implementation |
| None | All net delays set to zero | Logic-only analysis |

## 19. report_slr_crossing Syntax

```tcl
report_slr_crossing [-cells <args>]
                    [-fanout_greater_than <arg>]
                    [-max_nets <arg>]
                    [-file <arg>] [-append]
                    [-return_string]
                    [-quiet] [-verbose]
```

### Common Patterns
```tcl
# Nets with fanout > 100, limited to 20 nets
report_slr_crossing -fanout_greater_than 100 -max_nets 20

# Scoped to specific cells
report_slr_crossing -cells [get_cells [list cellA cellB]]
```
