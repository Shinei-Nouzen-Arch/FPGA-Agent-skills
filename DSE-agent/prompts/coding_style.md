# HLS C-Model Coding Style Guide

> Based on AMD Vitis HLS UG1399 (2025.2) official documentation, recommended coding styles, and Vitis-HLS-Introductory-Examples.
>
> References:
> - [UG1399 HLS Programmers Guide (2025.2)](https://docs.amd.com/r/en-US/ug1399-vitis-hls/HLS-Programmers-Guide)
> - [UG1399 Examples of Recommended Coding Styles (2025.2)](https://docs.amd.com/r/en-US/ug1399-vitis-hls/Examples-of-Recommended-Coding-Styles)
> - [UG1399 Coding C/C++ Functions (2025.2)](https://docs.amd.com/r/en-US/ug1399-vitis-hls/Coding-C/C-Functions)
> - [UG1399 Array Accesses and Performance (2025.2)](https://docs.amd.com/r/en-US/ug1399-vitis-hls/Array-Accesses-and-Performance)
> - [UG1399 Best Practices for M_AXI Interfaces (2025.2)](https://docs.amd.com/r/en-US/ug1399-vitis-hls/Best-Practices-for-Designing-with-M_AXI-Interfaces)
> - [UG1399 Optimizing Techniques (2025.2)](https://docs.amd.com/r/en-US/ug1399-vitis-hls/Optimizing-Techniques-and-Troubleshooting-Tips)
> - [UG1399 Coding Style for Array to Stream (2025.2)](https://docs.amd.com/r/en-US/ug1399-vitis-hls/Coding-Style-for-Array-to-Stream)
> - [Vitis-HLS-Introductory-Examples (GitHub)](https://github.com/Xilinx/Vitis-HLS-Introductory-Examples)
> - [Vitis-Tutorials 2025.2 branch](https://github.com/Xilinx/Vitis-Tutorials/tree/2025.2/Vitis_HLS)

---

## Core Principle

**C++ code = hardware architecture description.** Every line of code must map to a concrete hardware structure on the FPGA.

---

## 1. AMD Three Paradigms (Three Paradigms for Programming FPGAs)

AMD UG1399 defines three paradigms for FPGA programming. All HLS code should be a composition of these:

| Paradigm | Meaning | Hardware Mapping |
|----------|---------|-----------------|
| **Sequential** | Sequential C code execution | FSM + datapath |
| **Pipelining** | Instruction-level pipeline | Pipelined compute units, one launch per cycle |
| **Dataflow** | Task-level parallelism (producer-consumer) | Multiple independent modules interconnected via FIFOs/PIPOs running in parallel |

**Design order (UG1399 recommended)**:
1. Design the macro architecture first (Dataflow: module partition + interconnect)
2. Optimize the micro structure next (Pipelining: loop pipelining)
3. Tune parameters last (Unroll factor, Partition factor, etc.)

---

## 2. Load-Compute-Store Pattern (AMD-Recommended Kernel Structure)

AMD's recommended top-level kernel structure, dividing code into three stages:

```cpp
void kernel_top(data_t *ddr_in, data_t *ddr_out) {
    #pragma HLS DATAFLOW

    // On-chip cache (BRAM)
    data_t local_in[N];
    data_t local_out[N];

    // Three independent hardware modules, interconnected via BRAM/Stream
    load(ddr_in, local_in);       // DMA read engine
    compute(local_in, local_out); // Arithmetic datapath
    store(local_out, ddr_out);    // DMA write engine
}
```

**Hardware mapping**:
```
[DDR] ─AXI Master─→ [Load Module] ─BRAM/FIFO─→ [Compute Module] ─BRAM/FIFO─→ [Store Module] ─AXI Master─→ [DDR]
```

Using `hls::stream` instead of arrays enables element-level pipelining:
```cpp
void kernel_top(data_t *ddr_in, data_t *ddr_out) {
    #pragma HLS DATAFLOW

    hls::stream<data_t> s_in, s_out;
    #pragma HLS STREAM variable=s_in depth=64
    #pragma HLS STREAM variable=s_out depth=64

    load(ddr_in, s_in);
    compute(s_in, s_out);
    store(s_out, ddr_out);
}
```

> ⚠️ **Anti-pattern**: Putting all logic in a single function with no Load/Compute/Store separation → the tool cannot pipeline memory access and compute, resulting in sequential execution with no throughput improvement.

---

## 3. Data Types → Hardware Bit Width

Use arbitrary precision types for precise hardware resource control (UG1399 Arbitrary Precision Types):

```cpp
#include <ap_int.h>
#include <ap_fixed.h>

// Integer types → precise-width registers/arithmetic units
ap_int<32> sum = 0;            // 32-bit signed
ap_uint<8> index = 0;          // 8-bit unsigned counter
ap_int<64> acc = 0;            // 64-bit accumulator (prevents overflow in multiply)

// Fixed-point types → fixed-point arithmetic units (avoids float DSP overhead)
ap_fixed<16,8,AP_RND,AP_SAT> coeff;  // 16-bit width, 8 integer bits, round, saturate

// Vector types → SIMD parallel operations (2025.2 enhanced)
#include <hls_vector.h>
hls::vector<ap_int<16>, 8> vec8;  // 8-way 16-bit SIMD
```

> ⚠️ **Anti-pattern**: Using `float` or `double` for arithmetic → synthesized as LUT-based soft-float (FP32 fmul+fadd ~8ns critical path on xczu7ev), consuming far more resources than DSP48E2 native integer/fixed-point. Always prefer `ap_fixed` or `ap_int` unless FP32 precision is absolutely required.

---

## 4. Functions → Hardware Modules

Each function becomes an independent hardware module after synthesis (UG1399 Coding C/C++ Functions):

```cpp
// Each function = independent hardware module (with its own FSM and datapath)
void load_data(data_t *mem, data_t local[N]) { ... }   // → DMA read module
void compute(data_t in[N], data_t out[N]) { ... }      // → Arithmetic module
void store_data(data_t local[N], data_t *mem) { ... }   // → DMA write module

// Function inlining → merged into parent module (eliminates call overhead, increases optimization space)
void small_helper(...) {
    #pragma HLS INLINE
}

// Keep hierarchy → synthesized independently, for reuse and timing closure
void large_module(...) {
    #pragma HLS INLINE off
}
```

**AMD recommendation**: Each function should ideally contain a single loop nest for throughput measurability and optimization.

> ⚠️ **Anti-pattern**: Multiple independent loop nests in one function without DATAFLOW → sequential execution. Split into separate functions + `#pragma HLS DATAFLOW` for task-level parallelism.

---

## 5. Arrays → On-Chip Storage

(UG1399 Array Accesses and Performance)

```cpp
// Local array → BRAM (dual-port by default, max 2 concurrent read/writes)
data_t local_buf[1024];
#pragma HLS BIND_STORAGE variable=local_buf type=RAM_2P impl=bram

// Large-capacity array → URAM (UltraRAM, larger density, single-port)
data_t large_buf[65536];
#pragma HLS BIND_STORAGE variable=large_buf type=RAM_S2P impl=uram

// Array partitioning → multi-port storage / register bank
data_t buf[16];
#pragma HLS ARRAY_PARTITION variable=buf type=complete  // → 16 registers

data_t matrix[64][64];
#pragma HLS ARRAY_PARTITION variable=matrix type=cyclic factor=4 dim=2
// → 4 interleaved partitions on dim=2, supports 4-way parallel column access
```

### Storage Selection Decision Tree

| Array Size | Recommended Storage | Reason |
|------------|-------------------|--------|
| < 2 KB | Distributed RAM (automatic) | Zero-latency read, consumes LUTs |
| 2 KB – 64 KB | BRAM | Dual-port, flexible width/depth, low latency |
| > 64 KB | URAM | Large capacity (3.46 MB total on xczu7ev), single-port, built-in pipeline registers |

**BRAM constraints** (AMD UG1399):
- BRAM has at most 2 ports → if a loop requires > 2 concurrent accesses, ARRAY_PARTITION is mandatory
- Complete partition (`complete`) → registers, fastest but area-heavy
- Cyclic/block partition → multiple BRAM blocks, balances bandwidth and area
- Functions/loops must complete all array reads before all writes → affects dataflow scheduling

> ⚠️ **Anti-pattern**: `#pragma HLS UNROLL factor=8` on a loop accessing an array without `ARRAY_PARTITION` → BRAM only has 2 ports, the tool must serialize accesses → II > 1 (performance collapses).

> ⚠️ **Anti-pattern**: Large arrays left as default BRAM without `BIND_STORAGE impl=uram` → BRAM exhaustion. Use URAM for buffers > 64 KB (e.g., large tile buffers, systolic input staging).

---

## 6. Loops → Hardware Control Structures

### 6.1 Pipeline

```cpp
// Pipelined loop → pipelined datapath, accepts new input every II cycles
LOOP_PROCESS:
for (int i = 0; i < N; i++) {
    #pragma HLS PIPELINE II=1
    result[i] = a[i] * b[i] + c[i];  // → multiplier (DSP) + adder
}
// Hardware: N-stage pipeline, 1 result per cycle throughput
```

### 6.2 Unroll

```cpp
// Full unroll → parallel hardware copies
LOOP_PARALLEL:
for (int i = 0; i < 4; i++) {
    #pragma HLS UNROLL
    out[i] = in[i] * weight[i];  // → 4 parallel multipliers
}

// Partial unroll → partial parallelism
LOOP_PARTIAL:
for (int i = 0; i < 16; i++) {
    #pragma HLS UNROLL factor=4
    out[i] = in[i] * weight[i];  // → 4 parallel multipliers, 4 iterations
}
```

### 6.3 Nested Loop Optimization

```cpp
// Perfect nesting + inner loop pipeline = most common efficient pattern
LOOP_ROW:
for (int i = 0; i < M; i++) {
    LOOP_COL:
    for (int j = 0; j < N; j++) {
        #pragma HLS PIPELINE II=1   // inner loop pipelined
        ...
    }
}
// Hardware: outer loop = FSM counter, inner loop = pipelined datapath
// Total latency ≈ M × (N + pipeline_depth)
```

### 6.4 Loop Bounds

```cpp
// Loop bounds must be compile-time determinable or annotated with tripcount
// Variable-bound loops must be annotated → helps the synthesizer allocate resources
LOOP_VAR:
for (int i = 0; i < n; i++) {
    #pragma HLS LOOP_TRIPCOUNT min=1 max=1024 avg=512
    ...
}
```

> ⚠️ **Anti-pattern**: `#pragma HLS PIPELINE` on the outer loop but not the innermost → inner loop executes serially per outer iteration, no throughput improvement. PIPELINE must be on the innermost compute loop.

> ⚠️ **Anti-pattern**: Non-perfectly nested loops (extra statements between inner and outer loops) → the tool cannot flatten them → outer loop FSM stalls inner pipeline. Restructure to perfect nesting or use DATAFLOW.

---

## 7. Stream → FIFO Hardware

(UG1399 Coding Style for Array to Stream)

```cpp
#include <hls_stream.h>

// hls::stream = hardware FIFO
hls::stream<data_t> fifo_a("fifo_a");
#pragma HLS STREAM variable=fifo_a depth=64  // 64-deep FIFO

// Template depth declaration (2025.2 recommended)
hls::stream<data_t, 64> fifo_b;

// Stream read/write = FIFO push/pop
fifo_a.write(data);                 // → FIFO write port
data_t val = fifo_a.read();         // → FIFO read port

// stream_of_blocks → Ping-Pong Buffer (2025.2)
#include <hls_streamofblocks.h>
hls::stream_of_blocks<data_t, BLOCK_SIZE> sob;
// → automatic double buffering: producer writes one block while consumer reads the other
```

**Array-to-Stream conversion** (UG1399 recommended pattern):
```cpp
// Convert array parameters to stream interfaces for element-level pipelining
void func(hls::stream<data_t> &in, hls::stream<data_t> &out) {
    // Element-by-element processing, no need to wait for the whole array
    for (int i = 0; i < N; i++) {
        #pragma HLS PIPELINE II=1
        data_t val = in.read();
        out.write(process(val));
    }
}
```

> ⚠️ **Anti-pattern**: `hls::stream` without `depth` pragma and a fast producer / slow consumer → producer blocks waiting for FIFO space → deadlock. Always set `depth` ≥ the maximum burst the producer can generate before the consumer catches up.

---

## 8. Dataflow → Module-Level Pipeline

(UG1399 Tasks and Dataflow)

```cpp
void top(data_t *in, data_t *out) {
    #pragma HLS DATAFLOW

    hls::stream<data_t> s1, s2;

    // Three modules run in parallel, interconnected via FIFOs
    // Hardware: three independent FSMs + back-pressure FIFOs
    load(in, s1);       // data ready → push to s1
    compute(s1, s2);    // s1 has data → start processing
    store(s2, out);     // s2 has data → start writing
}
```

**Dataflow constraints** (AMD UG1399):
- Inter-function dataflow must be **single-producer, single-consumer**
- No feedback loops between functions (acyclic DAG only)
- Inter-function communication uses `hls::stream` or arrays (arrays auto-converted to PIPO)
- Inside each function, loop reads must be strictly before writes (sequential access)

**hls::task data-driven model (2025.2)**:
```cpp
// For free-running data-driven modules, no external start signal needed
void free_running_module(hls::stream<data_t> &in, hls::stream<data_t> &out) {
    #pragma HLS INTERFACE ap_ctrl_none port=return
    data_t val = in.read();
    out.write(process(val));
}
```

> ⚠️ **Anti-pattern**: Two DATAFLOW functions both reading the same array → DATAFLOW compliance check fails, tool silently falls back to sequential execution. Use `hls::stream` or separate arrays for inter-function communication.

---

## 9. Interfaces → Bus Protocols

(UG1399 Best Practices for M_AXI Interfaces)

### 9.1 AXI Master (DDR/HBM Access)

```cpp
void kernel(data_t *mem_in, data_t *mem_out) {
#pragma HLS INTERFACE m_axi port=mem_in  bundle=gmem0 \
    offset=slave \
    max_read_burst_length=64 \
    num_read_outstanding=16 \
    max_widen_bitwidth=512

#pragma HLS INTERFACE m_axi port=mem_out bundle=gmem1 \
    offset=slave \
    max_write_burst_length=64 \
    num_write_outstanding=16
```

**AMD M_AXI best practices**:
- Different data ports use different `bundle` → independent AXI channels, no bus contention
- `max_read/write_burst_length=64/128/256` → larger burst length for better bandwidth utilization
- `num_read/write_outstanding=16/32` → allow multiple concurrent requests, hide latency
- `max_widen_bitwidth=512` → auto width extension, better bus utilization
- Only sequential access patterns can be synthesized as burst transfers; random access degenerates to single beats

> ⚠️ **Anti-pattern**: Two `m_axi` ports sharing the same `bundle` → bus contention, sequentialized memory access. Always use separate bundles for independent data streams.

### 9.2 AXI Stream (Streaming Data)

```cpp
void kernel(hls::stream<ap_axiu<32,1,1,1>> &axis_in,
            hls::stream<ap_axiu<32,1,1,1>> &axis_out) {
#pragma HLS INTERFACE axis port=axis_in
#pragma HLS INTERFACE axis port=axis_out
```

### 9.3 AXI Lite (Control Registers)

```cpp
#pragma HLS INTERFACE s_axilite port=ctrl_reg
#pragma HLS INTERFACE s_axilite port=return  // start/done signals
```

---

## 10. Tiling → Data Movement Architecture

Within the Load-Compute-Store framework, process large-scale data in tiles:

```cpp
#define TILE_M 8
#define TILE_N 8
#define TILE_K 8

void matmul_tiled(data_t *ddr_A, data_t *ddr_B, data_t *ddr_C) {
    data_t local_A[TILE_M][TILE_K];
    data_t local_B[TILE_K][TILE_N];
    data_t local_C[TILE_M][TILE_N];

    TILE_I:
    for (int ii = 0; ii < M; ii += TILE_M) {
        TILE_J:
        for (int jj = 0; jj < N; jj += TILE_N) {
            // Initialize output tile
            init_tile(local_C);

            TILE_K:
            for (int kk = 0; kk < K; kk += TILE_K) {
                // Load → Compute on on-chip BRAM
                load_tile(ddr_A, local_A, ii, kk);
                load_tile(ddr_B, local_B, kk, jj);
                compute_tile(local_A, local_B, local_C);
            }
            // Store back to DDR
            store_tile(local_C, ddr_C, ii, jj);
        }
    }
}
```

**Hardware mapping**:
```
Outer tile loops → control FSM
load_tile    → AXI burst read + BRAM write
compute_tile → MAC array (pipeline + partition)
store_tile   → BRAM read + AXI burst write
```

---

## 11. Operation Binding → DSP/Fabric Selection

(UG1399 BIND_OP / BIND_STORAGE)

```cpp
// Multiplication → force DSP48 (high performance, limited quantity)
#pragma HLS BIND_OP variable=result op=mul impl=dsp latency=3

// Addition → use fabric logic (save DSPs)
#pragma HLS BIND_OP variable=sum op=add impl=fabric

// Resource limit → max N instances of a given operation
#pragma HLS ALLOCATION operation instances=mul limit=16
```

---

## 12. Prohibited Constructs

(UG1399 Unsupported C/C++ Constructs)

```cpp
// Prohibited: dynamic memory
int *p = new int[n]; // ✗  → use fixed-size arrays
malloc(size);        // ✗

// Prohibited: recursion
int fib(int n) { return fib(n-1) + fib(n-2); } // ✗ → use template unrolling or iteration

// Prohibited: system calls (during synthesis)
printf("debug\n");   // ✗ → guard with #ifndef __SYNTHESIS__

// Prohibited: non-deterministic loop bounds (without tripcount annotation)
while (unknown()) { ... } // ✗ → add LOOP_TRIPCOUNT or convert to for-loop

// Use with caution: floating-point types (heavy resource overhead)
float x = 1.5f;     // → prefer ap_fixed; use ap_float<W,E> only when floating-point is mandatory
```

---

## 13. Naming Conventions

```
Function names → hardware module:    load_tile, compute_core, store_result
Array names    → storage unit:       local_A (BRAM), buf_line (register bank)
Stream names   → FIFO name:          s_input, s_output, fifo_stage1
Loop labels    → FSM state:          LOAD_ROW:, COMP_MAC:, STORE_COL:
Macros         → hardware params:    TILE_SIZE, PARALLEL_FACTOR, FIFO_DEPTH
Type typedefs  → bit-width contract: typedef ap_int<32> data_t; typedef ap_int<64> acc_t;
```

---

## 14. Complete Kernel + Testbench Template

### 14.1 kernel.cpp

```cpp
#include <ap_int.h>
#include <hls_stream.h>

// ── Hardware Parameters ──
#define N 1024
#define TILE 64
#define PAR  4         // Parallelism factor

// ── Type Definitions ──
typedef ap_int<32> data_t;
typedef ap_int<64> acc_t;

// ── Load Module: AXI Burst Read → Stream ──
void load(data_t *ddr_in, hls::stream<data_t> &s_out) {
    LOAD:
    for (int i = 0; i < N; i++) {
        #pragma HLS PIPELINE II=1
        s_out.write(ddr_in[i]);
    }
}

// ── Compute Module: Stream → Arithmetic → Stream ──
void compute(hls::stream<data_t> &s_in, hls::stream<data_t> &s_out) {
    COMPUTE:
    for (int i = 0; i < N; i++) {
        #pragma HLS PIPELINE II=1
        data_t val = s_in.read();
        s_out.write(val * val + val);  // → DSP multiplier + adder
    }
}

// ── Store Module: Stream → AXI Burst Write ──
void store(hls::stream<data_t> &s_in, data_t *ddr_out) {
    STORE:
    for (int i = 0; i < N; i++) {
        #pragma HLS PIPELINE II=1
        ddr_out[i] = s_in.read();
    }
}

// ── Top: Interconnect Three Modules ──
void kernel_top(data_t *ddr_in, data_t *ddr_out) {
    #pragma HLS INTERFACE m_axi port=ddr_in  bundle=gmem0 max_read_burst_length=64
    #pragma HLS INTERFACE m_axi port=ddr_out bundle=gmem1 max_write_burst_length=64
    #pragma HLS INTERFACE s_axilite port=ddr_in
    #pragma HLS INTERFACE s_axilite port=ddr_out
    #pragma HLS INTERFACE s_axilite port=return

    #pragma HLS DATAFLOW

    hls::stream<data_t, 64> s1, s2;

    load(ddr_in, s1);
    compute(s1, s2);
    store(s2, ddr_out);
}
```

### 14.2 testbench.cpp

```cpp
#include <iostream>
#include <cstdlib>
#include <cstring>

// Must match kernel types
typedef ap_int<32> data_t;

// Reference (golden) implementation — runs on CPU
void golden_kernel(data_t *in, data_t *out, int N) {
    for (int i = 0; i < N; i++) {
        out[i] = in[i] * in[i] + in[i];
    }
}

// DUT function declaration
void kernel_top(data_t *ddr_in, data_t *ddr_out);

int main() {
    const int N = 1024;

    // Allocate buffers
    data_t *in  = new data_t[N];
    data_t *out_hw = new data_t[N];
    data_t *out_sw = new data_t[N];

    // Initialize input with deterministic seed
    for (int i = 0; i < N; i++) {
        in[i] = (data_t)(rand() % 100 - 50);  // range: -50..49
    }

    // Run hardware design (DUT)
    kernel_top(in, out_hw);

    // Run golden reference
    golden_kernel(in, out_sw, N);

    // Compare
    int errors = 0;
    for (int i = 0; i < N; i++) {
        if (out_hw[i] != out_sw[i]) {
            std::cerr << "Mismatch at index " << i
                      << ": HW=" << out_hw[i] << " SW=" << out_sw[i] << std::endl;
            errors++;
            if (errors > 10) break;  // limit error output
        }
    }

    if (errors == 0) {
        std::cout << "PASSED: All " << N << " outputs match." << std::endl;
        return 0;
    } else {
        std::cout << "FAILED: " << errors << " mismatches." << std::endl;
        return 1;
    }
}
```

> ⚠️ **Testbench tip**: For co-simulation, the testbench must use the same data types and function signatures as the kernel. Mismatched types (e.g., `int` in testbench vs `ap_int<32>` in kernel) are the #1 cause of cosim failures.
