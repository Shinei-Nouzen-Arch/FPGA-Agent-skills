# Hardware Design Checklist

Answer each item during T1 self-check. Every item must have a clear ✓ or ✗ + reason.
All ★ items must be ✓ before proceeding to T2 synthesis. Any ✗ must be fixed in code first.

---

## A. Pipeline Coverage ★

**A1.** Does the innermost compute loop have `#pragma HLS PIPELINE II=1`?
- **How to check**: Scan all loop nests. For each nest, find the innermost loop. Verify PIPELINE pragma is on that loop (not on an outer loop).
- **Common error**: PIPELINE on the outer loop → inner loop executes serially, no throughput improvement.

**A2.** If the outer loop also needs high throughput, have you considered loop flattening or DATAFLOW?
- Outer loop without PIPELINE + inner loop with PIPELINE → each outer iteration waits for the inner to complete.
- If outer throughput also matters: either flatten (perfect nesting) or switch to DATAFLOW.

---

## B. Parallel Access & Storage Partition ★

**B1.** For UNROLLED loops, are the accessed arrays partitioned with `ARRAY_PARTITION`?
- **How to check**: For each `#pragma HLS UNROLL` or `UNROLL factor=P`, find all arrays accessed inside that loop. Verify each has `ARRAY_PARTITION` with factor ≥ P.
- Unroll factor P → P concurrent read/write ports needed per cycle.
- BRAM has only 2 ports → P > 2 without partitioning causes II > 1.

**B2.** Does the partition type match the access pattern?
- **How to check**: Look at the array index expression inside the unrolled loop.
  - Column access (`B[k][j+p]`, j varies) → `cyclic factor=P dim=2`
  - Row access (`A[i][k]`, k varies) → `cyclic factor=P dim=1`
  - Random/irregular access → `complete` (full expansion to registers)

**B3.** Do multiple DATAFLOW functions access the same array?
- **How to check**: List all arrays used inside the `#pragma HLS DATAFLOW` region. If any array is read or written by more than one function → violation.
- Same array accessed by two functions → DATAFLOW compliance fails, tool silently falls back to sequential execution.

---

## C. DATAFLOW Compliance ★ (only if using DATAFLOW)

**C1.** Does every variable (array/stream) in the DATAFLOW region have exactly **one writer and one reader**?
- **How to check**: For each array/stream inside the DATAFLOW function, count how many functions write to it and how many read from it. Both must be exactly 1.
- Violation example: both `load` and `compute` functions read the same `local_A` → violation.

**C2.** Does the inter-function dataflow form a **directed acyclic graph (DAG)**? No feedback loops.
- **How to check**: Draw the producer→consumer graph. If there is any cycle (A→B→A) → violation.

**C3.** Inside each DATAFLOW sub-function, are all reads completed before all writes (sequential access)?
- **How to check**: Within each function, verify the code structure: read phase first, then write phase. No interleaving of reads and writes to the same variable.
- Out-of-order access (e.g., write to the middle, then read from the head) breaks DATAFLOW scheduling.

**C4.** If using `hls::stream`, is FIFO depth sufficient to prevent deadlock?
- **How to check**: Estimate the maximum burst the producer can generate before the consumer catches up. Set `depth` ≥ that number.
- Conservative estimate: `depth` ≥ one tile's worth of data, or the producer's maximum unconsumed output.
- Producer faster than consumer + insufficient depth → producer blocks → deadlock.

---

## D. Resource Estimation (fill in concrete numbers)

**D1.** Estimated DSP = ?
- Each MAC (multiply-accumulate) ≈ 3 DSP48s (Zynq UltraScale+)
- Total DSP ≈ parallel MAC count × 3
- Platform limit: 1728 DSP (xczu7ev)

**D2.** Estimated BRAM = ?
- On-chip array usage = element count × bit width (bits) / 18 Kbits (per BRAM_18K)
- Under DATAFLOW, local arrays become ping-pong buffers: × 2
- Using `hls::stream` instead of arrays → BRAM = 0 (FIFO implemented by registers when depth is small)
- Platform limit: 312 BRAM_36K = 624 BRAM_18K (xczu7ev)

**D3.** Are estimates within constraints (including this round's target constraints)?

---

## E. Timing Estimation

**E1.** What is the deepest combinational logic chain on the critical path? (List the path and estimate its delay)
- DSP multiplication: ~2–3 ns
- Adder tree (log₂P levels): ~0.5–1 ns per level
- BRAM read: ~1–2 ns
- Total must be < chosen `clock_ns`

**E2.** Does the chosen `clock_ns` match the architecture complexity?
- Deep DSP cascade (PAR=32 adder tree = 5 levels) → long critical path → not suitable for clock=5
- Simple pipeline (no large adder tree) → can attempt clock=5

---

## F. Code Compliance

**F1.** No dynamic memory (`new` / `malloc` / `delete` / `free`)?

**F2.** No recursive function calls?

**F3.** All loop bounds are compile-time constants, or annotated with `LOOP_TRIPCOUNT`?

**F4.** `printf` / `cout` / `assert` guarded by `#ifndef __SYNTHESIS__`?

**F5.** No STL containers (`std::vector` / `std::map` / etc.) in the synthesis path?

---

## G. Co-Simulation Quick Diagnostic

Common cosim failures and their fixes:

| Symptom | Likely Cause | Fix |
|---------|-------------|-----|
| m_axi port cosim error | Missing `depth` pragma on m_axi interface | Add `depth=1024` (or appropriate size) |
| Output mismatch at cycle N | Testbench data generation differs from kernel expectation | Check seed, input range, data type widths |
| Cosim hangs (timeout) | DATAFLOW deadlock or FIFO depth too small | Increase stream depth; verify single-producer-single-consumer |
| Type width mismatch | Testbench uses `int` but kernel uses `ap_int<N>` | Match types exactly between testbench and kernel |
| Partial output correct, rest wrong | Pipeline II > 1 causes timing misalignment | Check for II violations; add explicit cycle counting |

---

## Post-T2: Hardware Consistency Check

After synthesis, read actual values from the report and compare against D1/D2 estimates:

| Metric | Estimated | Actual | Ratio | Status |
|--------|-----------|--------|-------|--------|
| DSP    |           |        |       | ✓ / >2× |
| BRAM   |           |        |       | ✓ / >2× |
| LUT    |           |        |       | ✓ / >2× |
| II     | 1         |        |       | ✓ / >1 |
| WNS    | ≥ 0       |        |       | ✓ / <0 |

**Any ratio > 2× or II > 1 or WNS < 0 → must analyze root cause; do NOT proceed to T3**

Common root causes:
- **DSP far exceeds estimate** → unexpected floating-point operation not converted to fixed-point, or multiplication not synthesized to DSP
- **BRAM far exceeds estimate** → DATAFLOW ping-pong buffering not accounted for (local arrays × 2), or array not converted to stream
- **LUT far exceeds estimate** → excessive control logic from array partition, or soft-float fallback when `float` type was used
- **II > 1** → array port conflict (needs ARRAY_PARTITION) / loop dependency (logic must be rewritten) / timing violation (reduce clock or split logic)
- **WNS < 0** → critical path too long for chosen clock; add pipeline stages, reduce parallelism, or switch to shorter-critical-path data type
