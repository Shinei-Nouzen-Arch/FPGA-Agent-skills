# Agentic-DSE

FPGA HLS Design Space Exploration system. Main Agent (you) is the architect + evolution scheduler, coordinating 3 Worker Agents to explore the design space.

## Hard Constraints

All output designs must satisfy: **functional correctness (Co-sim pass) + timing closure (WNS ≥ 0) + resource compliance**.

## File Conventions

- `designs/<name>/` — HLS design project (spec + source + testbench)
- `workspace/{role}/` — Worker Agent working directories
- `results/{role}.json` — Worker Agent outputs
- `state/` — runtime state (Main Agent maintains, auto-created)
- `prompts/` — Worker Agent prompt templates
- `knowledge/` — knowledge base (self-evolving at runtime, auto-created)

## Auto-Created Files

The following directories and files are **auto-created by agents when needed**. Users do not need to prepare them manually.

### `state/` directory
- `pareto_front.json` — initialized as `[]`
- `lineage.json` — initialized as `[]`
- `search_directive.json` — written by `init req`
- `architecture_decisions.json` — written by Architect Agent
- `agent_contributions.json` — initialized as `{}`

### `knowledge/` directory
- `knowledge/core/platform_specs.json` — if absent, Architect Agent generates it from the target platform (default: xczu7ev)
- `knowledge/core/architecture_catalog.md` — if absent, Architect Agent generates a seed version using its own knowledge (at minimum: loop_unroll, systolic array, streaming pipeline)
- `knowledge/learned/successful_configs.json` — auto-created at runtime, initialized as `[]`
- `knowledge/learned/failure_cases.json` — auto-created at runtime, initialized as `[]`
- `knowledge/learned/learned_hints.json` — auto-created at runtime, initialized as `{}`

### `tmp/` directory
- `tmp/req_analysis.json` — requirement parsing intermediate result

## Commands

- `init req <design>` — parse user requirements, generate search_directive.json (run before `run dse`)
- `run dse <design> [N]` — execute N rounds of DSE iteration (default: 1)
- `show pareto` — display current Pareto front

## Evolution Algorithm

```
Population   = state/pareto_front.json
Selection    = Main Agent selects parents for each Worker
Mutation     = Explorer (large step) / Exploiter (small step)
Crossover    = Innovator (feature-level fusion of two parents)
Fitness      = Worker's internal T1→T2→T3→T4 validation chain
Elitism      = Pareto non-dominated solutions are retained
```

---

## `init req` Protocol

When the user provides rough requirements, execute the following steps:

### R1: Confirm Design Directory

Check if `designs/<name>/` exists. If not, guide the user to create the minimal structure:

```
designs/<name>/
├── spec.json              ← design constraints and objectives (required)
├── src/
│   └── kernel.cpp         ← original reference implementation
└── tb/
    └── testbench.cpp      ← functional verification testbench
```

**spec.json template**:
```json
{
  "name": "design name",
  "description": "brief description",
  "platform": "xczu7ev-ffvc1156-2-e",
  "constraints": {
    "dsp_max": 1728,
    "lut_max": 230400,
    "bram_max": 312
  },
  "objectives": {
    "priority": ["latency_ns", "bram", "lut", "dsp"],
    "latency_ns_max": null
  },
  "clock_options_ns": [3, 5, 7, 10]
}
```

Once created, read `designs/<name>/spec.json`.

### R2: Launch Requirement Parser Agent

Read `prompts/req_parser.md`.

Construct a complete prompt for the req_parser Agent using:
1. User's raw requirement text
2. spec.json content (platform constraints)
3. **Critical gaps checklist** (inline definition below)

Construct the prompt by concatenating these sections in order:

```
[Section 1: Agent Template]
Content of prompts/req_parser.md

[Section 2: Input Data]
### User Requirement
{user's raw requirement text}

### Design Constraints
{content of designs/<name>/spec.json}

### Platform Specs
{content of knowledge/core/platform_specs.json, if available}

### Critical Gaps Checklist
{the table defined below}
```

The Agent outputs `tmp/req_analysis.json`.

**Requirement parsing schema (inline definition)**:

The following critical gaps must be checked during parsing:

| ID | Gap | Condition | Impact | Default |
|----|-----|-----------|--------|---------|
| `frequency_requirement` | Minimum clock frequency | User did not specify clock range | Determines clock exploration range | Try all clock_options |
| `priority_clarification` | Optimization priority | User says "minimize X" but doesn't rank all | Determines `priority` field ordering | latency > BRAM > LUT > DSP |
| `area_constraint` | Hard area limit | User mentions resource limits but no specific values | Determines `constraints_override` | Use spec.json platform maximums |
| `data_precision` | Data precision | User didn't specify fixed/floating point | Affects critical path and clock choice | ap_fixed<16,8> |

### R3: Launch Architecture Selection Agent

Read `prompts/architect.md`.

Launch the Architect Agent with:
- Algorithm description (extracted from spec.json + req_analysis)
- Platform constraints (spec.json)
- Requirement parsing result (tmp/req_analysis.json)

Architect Agent responsibilities:
- If `knowledge/core/architecture_catalog.md` does not exist, generate a seed version using its own knowledge
- If `knowledge/core/platform_specs.json` does not exist, auto-generate for the target platform
- Analyze algorithm characteristics, produce a candidate architecture list
- Output `state/architecture_decisions.json`

### R4: Clarification Questions

Read `tmp/req_analysis.json` and `state/architecture_decisions.json`.
- If `questions_needed` is non-empty: present questions to the user (max 3 at a time), collect answers
- If `confidence == "low"`: must ask, cannot skip
- Present architecture candidates to the user, confirm if adjustments are needed

### R5: Generate Final Configuration

Based on extracted requirements + architecture selection + user answers:

**Update `designs/<name>/spec.json`**:
- Override constraint fields with non-null values from `constraints_override`
- Write `clock_options_ns` to the file

**Write `state/search_directive.json`**:
```json
{
  "user_requirement": "raw requirement text",
  "priority": ["latency_ns", "bram", "lut", "dsp"],
  "latency_ns_max": null,
  "clock_preference": [5, 7, 10],
  "search_bias": "description of Pareto region to focus on",
  "avoid": null,
  "architecture_candidates": ["systolic_os", "loop_unroll", "streaming_pipeline"],
  "architecture_first_round": {
    "explorer": "systolic_os",
    "exploiter": "loop_unroll",
    "innovator": "streaming_pipeline"
  },
  "clarifications": [
    {"question": "...", "answer": "..."}
  ]
}
```

### R6: Confirm Convergence Criteria

Before starting DSE, present convergence options to the user:

**A) Hypervolume metric**
- Objectives: [latency_ns, LUT, BRAM_36K, DSP, II_cycles] (5D minimize)
- Reference point: [20971520, 230400, 312, 1728, 2097152]
- Stop condition: < 2% HV improvement for 3 consecutive rounds
- Requires: pymoo Python library

**B) Simplified metrics**
- No new Pareto solutions for N consecutive rounds → stop
- Or: best latency_ns improvement < X% for N rounds → stop

**C) Manual control (recommended for new users)**
- Show current Pareto front summary after each round
- User decides whether to continue

Default: C. Record the user's choice in `state/search_directive.json` under the `"convergence_mode"` field.

### R7: Confirm Output

Present a summary (including architecture selection + convergence criteria) to the user. After confirmation, proceed to `run dse`.

---

## Iteration Protocol

### ⚠️ Phase Detection

Before each round, determine which phase you're in:

**Logic**:
1. Read `state/pareto_front.json`
2. If Pareto front is empty → **first round**, follow "first round special handling"
3. If Pareto front is non-empty → **iteration round**, follow normal flow

### Step 1: Read State

- Read `state/pareto_front.json`
- Read `state/lineage.json`
- Read `state/agent_contributions.json`
- Read `state/architecture_decisions.json`
- If `state/search_directive.json` exists: read it and prioritize its `priority` / `search_bias` / `avoid` / `architecture_candidates` in decision-making

### Step 2: Four-Layer Decision (Architecture-Aware)

**Layer 0 — Bottleneck Root-Cause Analysis** (iteration rounds only, skip on first round):

Analyze the previous round's results:

```
1. What was the failure/bottleneck?
   - Timing violation? → What is the critical path? Combinational logic too deep or routing congestion?
   - Resource exceeded? → Which resource? Parallelism too high or storage structure misconfigured?
   - Latency stagnation? → Compute bottleneck or memory bandwidth bottleneck?

2. What level does the root cause belong to?
   - Parameter level (fixable by changing PAR/TILE/clock) → small change → Exploiter fine-tuning
   - Microarchitecture level (fixable by pragma/pipeline/partition changes) → medium change → Explorer modifies code structure
   - Architecture level (needs fundamentally different compute pattern/dataflow/PE structure) → large change → Architect re-evaluation + Explorer switches architecture
```

Concrete conditions for triggering architecture re-evaluation:
| Symptom | Root Cause | Action |
|---------|-----------|--------|
| WNS violation unfixable by clock tuning | Critical path is inherent to the architecture | Large change: switch to shorter-critical-path architecture |
| DSP/LUT maxed out but latency unchanged | Parallelism ceiling of current architecture | Large change: switch to more efficient parallelism mode |
| 3 consecutive rounds with improvement < convergence threshold | Parameter space of current architecture exhausted | Large change: switch to unexplored architecture |
| Multiple agents' cosim repeatedly failing | Architecture mismatch with algorithm pattern | Medium or large change |

**Layer 1 — Architecture Decision**:

Based on Layer 0 analysis:
- **Continue current architecture**: root cause is at parameter/microarchitecture level
- **Microarchitecture modification**: add DATAFLOW, change pipeline depth, change storage mapping
- **Switch architecture**: launch Architect Agent (read `prompts/architect.md`) with bottleneck diagnosis info. Update `state/architecture_decisions.json`.

**Layer 2 — Direction Analysis**: Analyze Pareto front coverage across optimization directions:
- Real latency = `latency_ns = latency_cycles × clock_ns`
- Throughput = 1/II
- Compute density = throughput / (LUT + DSP)

Identify gaps and sparse regions. **clock is a design variable** — different solutions can use different clocks; compare based on latency_ns.

**Layer 3 — Task Assignment**: Assign to each Worker:
- Explorer: **architecture directive** + optimization direction + parent source code
- Exploiter: architecture directive + optimization direction + 1 parent source code
- Innovator: crossover directive + optimization direction + 2 parent source codes

### Step 3: Select Knowledge

- If `knowledge/core/platform_specs.json` exists: read it
- If `knowledge/core/architecture_catalog.md` exists: read it
- If `knowledge/core/learned_hints.json` exists and non-empty: read it
- If `knowledge/learned/successful_configs.json` exists and non-empty: read it
- If `knowledge/learned/failure_cases.json` exists and non-empty: read it
- **If knowledge directory or files do not exist: use the model's own knowledge to assist decisions; do not block the flow**

### Step 4: Launch Worker Agents

**How to launch**: Use `sessions_spawn` or equivalent subagent tool to **launch 3 Worker Agents in parallel**.

Each Worker's prompt is constructed by concatenating the following sections in order:

```
[Section 1: Worker Prompt Template]
Content of prompts/{explorer|exploiter|innovator}.md

[Section 2: Dynamic Data]
### Architecture Directive
{architecture direction / six-element description specified by Main Agent}

### Optimization Direction
{current round's optimization target}

### Parent Source Code
{full kernel.cpp content of the assigned parent + its metrics (clock_ns, wns, etc.)}
  NOTE: On the first round (no parents available), fill this with:
  "This is the first round. No parent exists. Use designs/<name>/src/kernel.cpp as a reference implementation only."

### Platform Constraints
{content of designs/<name>/spec.json}

### Current Pareto Front
{content of state/pareto_front.json; on first round: "[] (empty, this is the first round)"}

### Relevant Knowledge
{relevant knowledge entries selected in Step 3, or "No knowledge entries available yet."}
```

**First round note**: When the Pareto front is empty and no parents exist:
- Explorer and Exploiter: fill `{dynamic: parent source code}` with the reference kernel.cpp + a note that this is a reference, not a parent to modify
- Innovator: fill with BOTH kernel.cpp (as Parent A) + a description of a second candidate architecture from `state/architecture_decisions.json` (as Parent B). The Innovator should attempt to fuse features from these two different architectures, even though neither has been validated yet. If this is impractical, treat it as an independent design that borrows ideas from both candidates.

### Step 5: Read Results, Update Pareto

- Read `results/explorer.json`, `results/exploiter.json`, `results/innovator.json`
- For solutions with status="success", perform Pareto dominance check
- Update `state/pareto_front.json`
- Update `state/lineage.json` (record this round's architecture analysis, assignment strategy, each agent's results)
- Update `state/agent_contributions.json`

**⚠️ Error handling**:
- If all Workers failed: analyze failure causes, reduce constraint scope or simplify architecture, retry next round
- If only 1-2 succeeded: normal, continue. Pareto front may have few points

### Step 6: Convergence Check

Execute based on the convergence mode chosen in R6:

**Mode A (Hypervolume)**:
- Compute Hypervolume of the Pareto front
- A `hypervolume.py` script is NOT included in this repository. You (the Main Agent) must generate one or use an equivalent tool. The script should use pymoo's exact N-dimensional HV algorithm.
- Reference parameters:
  - Objectives: [latency_ns, LUT, BRAM_36K, DSP, II_cycles] (5D minimize)
  - Reference point: [20971520, 230400, 312, 1728, 2097152]
- If pymoo is unavailable, fall back to Mode B or C.
- Stop if < 2% improvement for 3 consecutive rounds

**Mode B (Simplified metrics)**:
- Stop if no new Pareto solutions or best latency improvement < X% for N rounds

**Mode C (Manual)**:
- Present current Pareto front summary to the user
- Ask whether to continue to the next round

### Step 7: Self-Evolution

- Successful solutions → append to `knowledge/learned/successful_configs.json`
- Failed solutions → append to `knowledge/learned/failure_cases.json`
- High-frequency success patterns (appeared 3+ times) → promote to `knowledge/core/learned_hints.json`
- Newly validated architectures (cosim pass + WNS ≥ 0) → append to `knowledge/core/architecture_catalog.md`

---

## First Round Special Handling

When the Pareto front is empty (Step 2 detects first round):

- **No parents available** → use `designs/<name>/src/kernel.cpp` as reference (not required to modify based on it)
- **Architecture assignment**: if `search_directive.json` has `architecture_first_round`, use it. Otherwise, select the first 3 candidates from `state/architecture_decisions.json`
- 3 Agents **each try a different architecture**:
  - Explorer: receives the full six-element description of the first candidate architecture, implements from scratch
  - Exploiter: receives the full six-element description of the second candidate (most reliable/baseline), implements from scratch
  - Innovator: receives both candidate architectures as Parent A and Parent B. Attempts a feature-level fusion across the two architectures. If fusion is impractical (both are unvalidated), treat it as an independent design borrowing ideas from both
- **Optimization directions**: assign throughput / compute density / energy efficiency respectively

---

## Innovator Crossover Specification

The Innovator must perform **feature-level crossover**, not compromise or redesign:

1. **Explicitly label** what was taken from Parent A and what from Parent B
2. Crossover granularity is **specific code structures / pragmas / architecture patterns**, for example:
   - ✅ Take A's "32-way parallel MAC loop structure" + B's "hls::stream inter-module communication"
   - ✅ Take A's "ARRAY_PARTITION complete" + B's "merged AXI ports"
3. Result code must be annotated with comments showing which segments came from A and which from B
4. ❌ **No parameter compromise**: A's PAR=32 + B's PAR=4 → PAR=8 is NOT crossover
5. ❌ **No redesign**: ignoring both parents and writing a new design from scratch is NOT crossover

---

## Round Output Format

After each round, present the following summary to the user:

```
=== DSE Round N Results ===

Pareto Front (X solutions):
| ID | Architecture | latency_ns | LUT | BRAM | DSP | II | clock |
|----|-------------|-----------|-----|------|-----|----|-------|
| 1  | ...         | ...       | ... | ...  | ... | .. | ...   |

This round's contributions:
- Explorer: [success/failed] + brief explanation
- Exploiter: [success/failed] + brief explanation
- Innovator: [success/failed] + brief explanation

[Mode C] Continue to next round? (y/n)
```
