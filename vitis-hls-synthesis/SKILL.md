---
name: vitis-hls-synthesis
description: Vitis HLS synthesis assistant for C/C++ to RTL conversion. Covers kernel development, pragma optimization, interface configuration, and synthesis report analysis. For post-synthesis implementation use vivado-impl, for timing analysis use vivado-analysis, for hardware debug use vivado-debug.
---

# Vitis HLS Synthesis Development Assistant

This skill helps you efficiently perform FPGA high-level synthesis development in the Vitis HLS environment. For complete syntax reference, see [REFERENCE.md](./REFERENCE.md).

## When to Use This Skill
- Need to write or optimize C/C++ HLS kernel code
- Need to add pragma optimization instructions (pipeline, unroll, dataflow, array_partition, etc.)
- Need to configure synthesis constraint files (.cfg)
- Need to run Vitis HLS synthesis commands
- Need to analyze synthesis reports (timing, resource, II value)
- Need to configure interface protocols (AXI4-Stream, M_AXI, s_axilite, etc.)
- Need to select appropriate data types (arbitrary precision types, HLS-specific types)
- Need to optimize DDR access and HBM bandwidth

## Workflow
### 1. Project Initialization
Confirm project structure and target device, create base directories:
```bash
mkdir -p <project>/src <project>/tb
```

### 2. Code Development
Write HLS kernel code, add pragma optimization instructions as needed, select appropriate data types and interface protocols.

### 3. Configuration File
Create `.cfg` configuration file, specify device, clock, interface, optimization and other parameters.

### 4. Run Flow
Execute synthesis flow:
1. C simulation to verify functional correctness
2. C synthesis to generate RTL
3. C/RTL co-simulation to verify hardware functionality
4. Export IP or Vitis kernel

### 5. Report Analysis
Check synthesis reports to confirm timing (WNS>0, TNS=0), II, and resource usage meet design requirements.

## Quick Reference
### Core Commands
| Task | Command |
|------|---------|
| Environment Setup | `source <Vitis_dir>/settings64.sh` |
| Synthesis | `v++ -c --mode hls --config <config.cfg>` |
| C Simulation | `vitis-run --mode hls --csim --config <config.cfg>` |
| Synthesis | `vitis-run --mode hls --csynth --config <config.cfg>` |
| Co-Simulation | `vitis-run --mode hls --cosim --config <config.cfg>` |
| Export IP | `vitis-run --mode hls --package --config <config.cfg>` |

### Common Pragmas
| Optimization Type | Instruction Example |
|-------------------|----------------------|
| Pipeline | `#pragma HLS PIPELINE II=1` |
| Loop Unroll | `#pragma HLS UNROLL factor=4` |
| Array Partition | `#pragma HLS ARRAY_PARTITION variable=arr type=cyclic factor=2` |
| Dataflow | `#pragma HLS DATAFLOW` |
| Interface Configuration | `#pragma HLS INTERFACE mode=m_axi port=mem bundle=gmem` |

### Core Configuration Parameters
| Parameter | Description |
|-----------|--------------|
| `part=<part_number>` | Target device model |
| `clock=<ns>` | Clock period (ns) |
| `flow_target=<vivado/vitis>` | Target output type |
| `syn.top=<function_name>` | Top-level function name |
| `syn.file=<source_file>` | Source file path |

For complete syntax and parameter reference, see [REFERENCE.md](./REFERENCE.md). Reference implementation examples are available in the `examples/` directory - read these files first when users need coding patterns or best practices.

## Common Scenario Examples
### 1. Pipeline Optimization Kernel
Add pipeline optimization to loops to achieve II=1:
```cpp
void kernel(float A[32][32], float B[32][32], float C[32][32]) {
    #pragma HLS INTERFACE m_axi port=A bundle=gmem
    #pragma HLS INTERFACE m_axi port=B bundle=gmem
    #pragma HLS INTERFACE m_axi port=C bundle=gmem

    for (int i = 0; i < 32; i++) {
        for (int j = 0; j < 32; j++) {
            #pragma HLS PIPELINE II=1
            float sum = 0;
            for (int k = 0; k < 32; k++) {
                sum += A[i][k] * B[k][j];
            }
            C[i][j] = sum;
        }
    }
}
```

### 2. Array Partition Optimization
Partition arrays to improve parallel access:
```cpp
#pragma HLS ARRAY_PARTITION variable=input_data type=cyclic factor=4 dim=1
#pragma HLS ARRAY_PARTITION variable=weights type=complete dim=1
```

### 3. Dataflow Task Parallelism
Implement multi-stage task parallelism:
```cpp
void top(hls::stream<int> &in, hls::stream<int> &out) {
    #pragma HLS DATAFLOW
    hls::stream<int> fifo1, fifo2;
    #pragma HLS STREAM variable=fifo1 depth=16
    #pragma HLS STREAM variable=fifo2 depth=16

    read_image(in, fifo1);
    process_image(fifo1, fifo2);
    write_image(fifo2, out);
}
```

### 4. AXI4-Stream Interface
Implement AXI4-Stream input and output:
```cpp
#include "hls_stream.h"

void axis_process(hls::stream<ap_int<32>> &in, hls::stream<ap_int<32>> &out) {
    #pragma HLS INTERFACE axis port=in
    #pragma HLS INTERFACE axis port=out
    #pragma HLS PIPELINE II=1

    ap_int<32> data = in.read();
    // Data processing
    out.write(data);
}
```

### 5. Fixed-Point Conversion
Convert floating-point to fixed-point for performance optimization:
```cpp
#include "ap_fixed.h"
// ap_fixed<total_width, integer_bits, quantization_mode, overflow_mode>
typedef ap_fixed<16, 8, AP_RND, AP_SAT> fixed_t;
fixed_t value = 1.5f; // Automatic quantization
```

## Related Skills

### Post-Synthesis Flow
- **vivado-impl**: Place and route optimization for HLS-generated IP
- **vivado-analysis**: Timing report analysis and closure strategies
- **vivado-constraints**: Add top-level timing and physical constraints

### Simulation and Debug
- **vivado-sim**: RTL functional and timing simulation
- **vivado-debug**: ILA/VIO hardware debug configuration

### Automation
- **vivado-tcl**: HLS IP integration and project automation scripts
