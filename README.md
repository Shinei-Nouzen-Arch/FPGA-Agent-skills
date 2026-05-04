> **My Claude account got banned.** 😅 If you find this project useful, consider sponsoring me to get a GPT Pro 20× subscription — I'd love to keep building.

---

**This repo includes two parts:**
1. **Basic Skills** — 8 FPGA/Vitis HLS development skills (below)
2. **Agentic-DSE** — multi-agent FPGA HLS design space exploration system (at the bottom)

---

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

---

# Agentic-DSE — Multi-Agent HLS Design Space Exploration

An agentic FPGA HLS design space exploration system. A Main Agent (architect + evolution scheduler) coordinates 3 Worker Agents to explore the Pareto-optimal frontier of FPGA designs through iterative refinement.

## How It Works

```
Main Agent (agent.md)
├── Explorer Worker     — large-step mutation, explores new architecture/parameter space
├── Exploiter Worker    — small-step fine-tuning, incremental optimization
└── Innovator Worker    — feature-level crossover, fuses two parent designs
     ↓
T1 (checklist) → T2 (synthesis) → T3 (co-simulation) → T4 (implementation)
     ↓
Pareto front update → Hypervolume check → Self-evolving knowledge base
```

Each round: the Main Agent analyzes bottlenecks, assigns tasks to all 3 Workers in parallel, collects validated results, and updates the Pareto frontier. The process converges when Hypervolume improvement drops below a threshold.

## What's In This Repo

| File | Purpose |
|------|--------|
| `agent.md` | Main Agent instructions — full orchestration protocol (`init req` → `run dse` → convergence) |
| `prompts/architect.md` | Hardware Architect Agent — selects architectures, analyzes critical paths, guides DSE direction |
| `prompts/explorer.md` | Explorer Worker — large-step mutation, bold architectural changes |
| `prompts/exploiter.md` | Exploiter Worker — small-step parameter/microarchitecture fine-tuning |
| `prompts/innovator.md` | Innovator Worker — feature-level crossover of two parent designs |
| `prompts/req_parser.md` | Requirement Parser Agent — converts user requirements into structured DSE configuration |
| `prompts/coding_style.md` | HLS C++ coding style guide based on AMD UG1399, with anti-patterns and templates |
| `prompts/hardware_checklist.md` | Pre-synthesis hardware checklist (A1–F5) + co-simulation quick diagnostic |

## Quick Start

1. **Prepare your design** — Create `designs/<name>/` with `spec.json`, `src/kernel.cpp`, and `tb/testbench.cpp`
2. **Initialize** — Run `init req <name>` with your requirements
3. **Explore** — Run `run dse <name> [N]` to execute N rounds of DSE iteration

## What's NOT Included (Must Be Created by You or the AI Agent)

This repo contains only the **agent prompt configuration**. The following are intentionally excluded and must be created at runtime:

| Component | Description | How to Get |
|-----------|-------------|------------|
| **HLS synthesis skill** | Vitis HLS tool knowledge (commands, pragma reference, report analysis) | Use the `vitis-hls-synthesis` skill from this repo, or create your own |
| **Knowledge base** | `knowledge/core/` (architecture catalog, platform specs) and `knowledge/learned/` (success/failure cases) | Auto-generated by the Architect Agent at runtime using its own knowledge, or pre-populated manually |
| **Hypervolume script** | `hypervolume.py` for Pareto frontier HV computation (pymoo-based) | Generate via AI or write your own; Mode A is optional — Mode B (simplified) and Mode C (manual) work without it |
| **Benchmarks** | Reference designs and test vectors | Provide your own `designs/<name>/` directory with kernel source and testbench |
| **Runtime state** | `state/`, `results/`, `tmp/` directories | Auto-created by agents during execution |

## Agent Platform Compatibility

The prompts are platform-agnostic. Use with any AI agent system that supports:
- Reading files (`Read`)
- Writing files (`Write`)
- Spawning subagents (`sessions_spawn` or equivalent)
- Shell execution (for HLS synthesis commands)
