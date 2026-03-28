# Vivado/Vitis FPGA Development Skills

8 skills covering AMD Vivado/Vitis 2025.2 FPGA development (RTL → Bitstream + HLS), based on official UG documentation.

## FPGA Development Flow

```
HLS C/C++ ──→ RTL Design ──→ Synthesis ──→ Constraints ──→ Implementation ──→ Timing Analysis ──→ Programming/Debug
vitis-hls                      synth        constraints       impl              analysis            debug

Simulation (throughout): sim
TCL Automation (throughout): tcl
```

## Skills Overview

| Skill | Documentation | Focus | Core Content |
|-------|--------------|-------|--------------|
| **vitis-hls-synthesis** | UG1399 | HLS Synthesis | Pragma optimization, interface config, dataflow/pipeline, burst optimization, synthesis report analysis |
| **vivado-synth** | UG901 | Synthesis | Strategy selection, attributes (RAM_STYLE/USE_DSP/SHREG), resource inference, FSM encoding, OOC/incremental synthesis |
| **vivado-constraints** | UG903 | Constraints | Clock definition, I/O delay, timing exceptions (false_path/multicycle), CDC, physical constraints, XDC debugging |
| **vivado-impl** | UG904 | Implementation | opt/place/phys_opt/route directives, congestion analysis, incremental implementation, ECO flow, run strategies |
| **vivado-analysis** | UG906 | Report Analysis | report_timing interpretation, slack calculation, QoR scoring (1-5), QoR suggestions, timing closure strategies |
| **vivado-debug** | UG908 | Debug Strategy | ILA/VIO/JTAG-to-AXI selection, mark_debug flow, ILA config, Versal debug architecture, hardware programming |
| **vivado-sim** | UG900 | Simulation | Behavioral/post-synth/post-impl simulation, xsim flow, third-party simulator integration, SAIF/VCD power simulation |
| **vivado-tcl** | UG835+UG892 | TCL Execution | Project/Non-Project mode, command reference, IP Integrator BD, debug core insertion, hardware programming |

## Architecture

Each skill uses a three-layer structure:

```
SKILL.md      → Decision guide (when to use what, strategy tables, decision trees)
REFERENCE.md  → Command syntax (complete parameters, attribute tables, value ranges)
examples/     → Code examples (loaded on demand)
```

- **SKILL.md** auto-loads on skill trigger, focuses on decision knowledge
- **REFERENCE.md** auto-loads on skill trigger, provides precise syntax
- **examples/** not auto-loaded, read via index as needed

## Cross-References Between Skills

```
vitis-hls-synthesis ──→ vivado-impl (implementation), vivado-analysis (timing), vivado-constraints (top-level)
                    ──→ vivado-sim (RTL simulation), vivado-debug (hardware debug), vivado-tcl (automation)

vivado-synth ──→ vivado-tcl (execution)
vivado-constraints ──→ vivado-tcl (execution), vivado-analysis (report interpretation)
vivado-impl ──→ vivado-tcl (execution), vivado-synth (synthesis), vivado-constraints (constraints), vivado-analysis (analysis)
              ──→ vitis-hls-synthesis (HLS-level optimization)
vivado-analysis ──→ vivado-tcl (execution), vivado-constraints (constraint modification), vivado-impl (strategy adjustment)
vivado-debug ──→ vivado-tcl (execution), vivado-impl (strategy), vivado-analysis (timing)
              ──→ vitis-hls-synthesis (HLS debug options)
vivado-sim ──→ vivado-tcl (execution), vitis-hls-synthesis (co-simulation)
vivado-tcl ──→ vivado-debug (debug decisions), vivado-analysis (analysis), vitis-hls-synthesis (IP integration)
```

## Examples Directory

| Skill | Directory | Content |
|-------|-----------|---------|
| vivado-synth | `examples/` | 64 Verilog/SV files — UG901 HDL coding templates (RAM/DSP/ROM/SRL/FSM) |
| vivado-impl | `examples/ug906/` | 3 sets of before/after RTL — report_qor_suggestions optimization examples |
| vitis-hls-synthesis | `examples/` | Design/Feature/Introductory tutorials — Official AMD HLS reference implementations |

## Command Verification

All TCL commands referenced in skills have been verified through Vivado 2025.2 (`-help` test) to ensure they exist and are usable.
