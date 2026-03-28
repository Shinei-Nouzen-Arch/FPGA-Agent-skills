# Vitis HLS Synthesis Complete Reference Manual

## HLS Programming Model

### Task Semantics
**Blocking vs Nonblocking**:
- Blocking: `read()`, `write()` wait until data is available
- Nonblocking: `read_nb()`, `write_nb()` return immediately with status flag

**Control-driven vs Data-driven**:
- Control-driven: Triggered by control signals, suitable for state machines
- Data-driven: Triggered by data availability, suitable for streaming pipelines

### Design Paradigms
**Programmable Logic**: Single control flow, algorithm-intensive applications
**Producer-Consumer**: Main thread + worker threads, parallel processing
**Streaming Data**: FIFO-connected task chains, dataflow processing

## Terminal Commands
### Vitis Environment Setup
```bash
# Set up Vitis environment
source <Vitis_Installation_Directory>/settings64.sh

# Launch Vitis IDE
vitis -w <workspace>

# Launch Vitis HLS standalone mode
vitis-hls
```

### Synthesis Commands
```bash
# Synthesize using configuration file
v++ -c --mode hls --config <config_file.cfg>

# Run individual flow steps
vitis-run --mode hls --csim --config <config_file>
vitis-run --mode hls --csynth --config <config_file>
vitis-run --mode hls --cosim --config <config_file>
vitis-run --mode hls --impl --config <config_file>
vitis-run --mode hls --package --config <config_file>
```

### Project Management
```bash
# Create new project
mkdir -p <project_name>/src
mkdir -p <project_name>/tb

# Clean build artifacts
rm -rf <project_name>/.Xil/
rm -rf <project_name>/build/
```

## Tcl Script Commands
```tcl
# Create/open project
open_project <project_name> -part <part_number>

# Set top-level function
set_top <top_function_name>

# Add source files
add_files src/kernel.cpp
add_files -tb tb/testbench.cpp

# C simulation
csim_design -clean -O

# Synthesis
csynth_design

# Co-simulation
cosim_design -tool xsim -trace_level all

# Export IP
export_design -format ip_catalog -vendor xilinx.com -library hls -version 1.0

# Close project
close_project
```

## Configuration File Format (.cfg)
### General Configuration
```ini
# General Settings
part=xcvu11p-flga2577-1-e
clock=8
clock_uncertainty=15%
flow_target=vitis

# Array partition configuration
config_array_partition=throughput-driven

# Loop unrolling tripcount threshold
config_tripcount_threshold=16

# RTL configuration
config_rtl.reset_level=low
config_rtl.reset_async=1
config_rtl.fsm_encoding=onehot

# Dataflow configuration
config_dataflow -depth=64
config_dataflow -strict=1
```

### HLS Configuration
```ini
[hls]
# Source Files
syn.file=src/kernel.cpp
syn.top=kernel

# Testbench
tb.file=tb/testbench.cpp

# C Simulation Settings
csim.O=true
csim.clean=true
csim.sanitize_address=true
csim.sanitize_undefined=true

# Co-Simulation Settings
cosim.O=true
cosim.trace_level=port
cosim.wave_debug=true
cosim.enable_dataflow_profiling=true
cosim.enable_tasks_profiling=true

# Top-level function specification
syn.top=kernel_top
```

### Interface Configuration
```ini
[hls]
# Clock enable
syn.interface.clock_enable=1

# 64-bit addressing
syn.interface.m_axi_addr64=1

# Alignment byte size
syn.interface.m_axi_alignment_byte_size=64

# Interface latency
syn.interface.m_axi_latency=21

# Maximum bit width
syn.interface.m_axi_max_bitwidth=512

# Automatic port width extension
syn.interface.m_axi_max_widen_bitwidth=512

# Number of outstanding read requests
syn.interface.m_axi_num_read_outstanding=16

# Number of outstanding write requests
syn.interface.m_axi_num_write_outstanding=16

# Maximum read burst length
syn.interface.m_axi_max_read_burst_length=16

# Maximum write burst length
syn.interface.m_axi_max_write_burst_length=16

# Offset mechanism
syn.interface.m_axi_offset=slave

# Mailbox functionality
syn.interface.s_axilite_mailbox=both

# Automatically create maximum ports
syn.interface.m_axi_auto_max_ports=false

# Automatic ID channel
syn.interface.m_axi_auto_id_channel=true

# Buffer implementation
syn.interface.m_axi_buffer_impl=block_ram
```

### Directive Configuration (Alternative to Source Pragmas)
```ini
[hls]
# Array partition
syn.directive.array_partition=dct_2d.in_block type=cyclic factor=8 dim=1

# Function inlining
syn.directive.inline=DES10<SIZE,RATE>::calcRUN recursive

# Pipeline
syn.directive.pipeline=kernel.compute_loop II=1 rewind

# Loop unroll
syn.directive.unroll=kernel.process_loop factor=4

# Dataflow
syn.directive.dataflow=kernel.top

# Operation binding
syn.directive.bind_op=* impl=dsp latency=3
```

### Debug Settings
```ini
[hls]
# Enable debugging
syn.debug.enable=1
syn.debug.directory=debug

# RTL Settings
syn.rtl.reset=state
syn.rtl.reset_async=0
syn.rtl.reset_level=high
syn.rtl.fsm_encoding=gray
syn.rtl.deadlock_detection=hw
```

### Vivado Implementation
```ini
[hls]
vivado.flow=impl
vivado.impl_strategy=Performance_Explore
vivado.optimization_level=2
vivado.report_level=1
```

### Operator Binding
```ini
[hls]
# Implement multipliers using DSP
syn.op=mul impl=dsp

# Implement adders using Fabric with 6 cycle latency
syn.op=add impl=fabric latency=6
```

## Pragma Optimization Instructions
### Conditional Optimization
```cpp
#pragma HLS if (OPT == 1) PIPELINE II=1
#pragma HLS if (OPT == 2) UNROLL factor=16

#pragma HLS if (TripCount > 20) PIPELINE II=1
#pragma HLS if (TripCount <= 20) UNROLL
```

### PIPELINE
```cpp
#pragma HLS PIPELINE II=1
#pragma HLS PIPELINE II=2 rewind
#pragma HLS PIPELINE style=flp  // flp/frp/stp
```

### UNROLL
```cpp
#pragma HLS UNROLL  // Full unroll
#pragma HLS UNROLL factor=4
#pragma HLS UNROLL skip_exit_check
#pragma HLS UNROLL off
```

### LOOP_FLATTEN
```cpp
#pragma HLS LOOP_FLATTEN off
#pragma HLS LOOP_FLATTEN factor=2
```

### LOOP_MERGE
```cpp
#pragma HLS LOOP_MERGE
#pragma HLS LOOP_MERGE off
```

### LOOP_TRIPCOUNT
```cpp
#pragma HLS LOOP_TRIPCOUNT min=16 max=64
#pragma HLS LOOP_TRIPCOUNT min=8 max=128 avg=64
#pragma HLS LOOP_TRIPCOUNT max=1024
```

### ARRAY_PARTITION
```cpp
#pragma HLS ARRAY_PARTITION variable=arr type=block factor=4
#pragma HLS ARRAY_PARTITION variable=arr type=cyclic factor=2 dim=1
#pragma HLS ARRAY_PARTITION variable=arr type=complete dim=0
#pragma HLS ARRAY_PARTITION variable=arr off=true
```

### ARRAY_RESHAPE
```cpp
#pragma HLS ARRAY_RESHAPE variable=arr type=block factor=2 dim=1
#pragma HLS ARRAY_RESHAPE variable=arr type=cyclic factor=2 dim=1
#pragma HLS ARRAY_RESHAPE variable=arr type=complete dim=1
#pragma HLS ARRAY_RESHAPE variable=container_arr object=item
```

### ARRAY_STENCIL
```cpp
#pragma HLS array_stencil variable=src
```

### AGGREGATE
```cpp
#pragma HLS AGGREGATE variable=my_struct compact=none
#pragma HLS AGGREGATE variable=my_struct compact=byte
#pragma HLS AGGREGATE variable=my_struct compact=bit
#pragma HLS AGGREGATE variable=my_struct compact=auto
```

### DISAGGREGATE
```cpp
#pragma HLS DISAGGREGATE variable=in
#pragma HLS DISAGGREGATE variable=out
#pragma HLS DISAGGREGATE variable=array_of_structs
#pragma HLS DISAGGREGATE variable=nested_struct->field
```

### DATAFLOW
```cpp
#pragma HLS DATAFLOW
```

### STREAM
```cpp
// 推荐方式：使用 hls::stream 类（Vitis HLS 2021.2+）
#include "hls_stream.h"
hls::stream<int> fifo("fifo");
#pragma HLS STREAM variable=fifo depth=64

// 或在声明时指定深度
hls::stream<int, 64> fifo_with_depth;

// 传统 pragma 方式（兼容旧版本）
#pragma HLS STREAM variable=fifo type=fifo depth=64
#pragma HLS STREAM variable=buf type=pipo depth=8
#pragma HLS STREAM variable=shared_buf type=shared
#pragma HLS STREAM variable=async_buf type=unsync depth=16
#pragma HLS STREAM variable=arr off=true
```

### INLINE
```cpp
#pragma HLS INLINE
#pragma HLS INLINE recursive
#pragma HLS INLINE off
```

### FUNCTION_INSTANTIATE
```cpp
#pragma HLS FUNCTION_INSTANTIATE variable=func
```

### INTERFACE
```cpp
// AXI4-Stream
#pragma HLS INTERFACE mode=axis port=stream_var
#pragma HLS INTERFACE mode=axis register_mode=both port=stream
#pragma HLS INTERFACE mode=axis register_mode=forward port=in
#pragma HLS INTERFACE mode=axis register_mode=reverse port=in
#pragma HLS INTERFACE mode=axis register_mode=off port=in

// M_AXI burst optimization configuration
#pragma HLS INTERFACE mode=m_axi port=arr offset=slave bundle=gmem0
#pragma HLS INTERFACE mode=m_axi port=arr offset=direct bundle=gmem0
#pragma HLS INTERFACE mode=m_axi port=arr offset=off bundle=gmem0
#pragma HLS INTERFACE mode=m_axi port=arr addr64=1
#pragma HLS INTERFACE mode=m_axi port=arr latency=100
#pragma HLS INTERFACE mode=m_axi port=arr num_read_outstanding=16  // Improve concurrency
#pragma HLS INTERFACE mode=m_axi port=arr num_write_outstanding=16
#pragma HLS INTERFACE mode=m_axi port=arr max_read_burst_length=256  // Increase burst length
#pragma HLS INTERFACE mode=m_axi port=arr max_write_burst_length=256
#pragma HLS INTERFACE mode=m_axi port=arr bundle=gmem0 channel=0
#pragma HLS INTERFACE mode=m_axi port=arr max_widen_bitwidth=512  // Automatic width extension

// S_AXILITE
#pragma HLS INTERFACE mode=s_axilite port=ctrl bundle=control
#pragma HLS INTERFACE mode=s_axilite port=ctrl clock=AXI_clk1
#pragma HLS INTERFACE s_axilite port=return autorestart

// Control protocols
#pragma HLS INTERFACE mode=ap_ctrl_none port=return      // No control signals, free-running
#pragma HLS INTERFACE mode=ap_ctrl_hs port=return        // Handshake control (default)
#pragma HLS INTERFACE mode=ap_ctrl_hs port=return autorestart  // Auto-restart after completion
#pragma HLS INTERFACE mode=ap_ctrl_chain port=return     // Chain control for dataflow

// Simple interfaces
#pragma HLS INTERFACE mode=ap_vld register port=out_data
#pragma HLS INTERFACE mode=ap_hs port=in_data
#pragma HLS INTERFACE mode=ap_fifo port=fifo_in
#pragma HLS INTERFACE mode=ap_memory port=array
#pragma HLS INTERFACE mode=ap_none port=simple_in
```

#### Control Protocol Details
- **ap_ctrl_none**: No control signals, kernel runs continuously. Use for streaming designs.
- **ap_ctrl_hs**: Standard handshake with ap_start, ap_done, ap_idle, ap_ready. Default for most designs.
- **ap_ctrl_chain**: Chain control for dataflow regions, enables task-level pipelining.
- **autorestart**: Kernel automatically restarts after completion without software intervention.

#### M_AXI Burst Optimization Tips
- Increase `max_read_burst_length` and `max_write_burst_length` to 64/128/256 to improve burst transmission efficiency
- Increase `num_read_outstanding` and `num_write_outstanding` to 16/32 to support more concurrent requests
- Use `bundle` parameter to assign different interfaces to independent AXI ports for parallel access
- Use `max_widen_bitwidth` to automatically extend data width and improve bus utilization

### LATENCY
```cpp
#pragma HLS LATENCY min=10 max=100
#pragma HLS LATENCY max=50
```

### PERFORMANCE
```cpp
#pragma HLS PERFORMANCE target_ti=1  // Target initiation interval (cycles)
#pragma HLS PERFORMANCE target_ti=16.7ms  // Supports time units: ms/us/ns
#pragma HLS PERFORMANCE target_tl=1000 // Target latency (cycles)
#pragma HLS PERFORMANCE target_ti=1 loop=compute_loop
```

### ALLOCATION
```cpp
#pragma HLS ALLOCATION function instances=func_name limit=2
#pragma HLS ALLOCATION operation instances=mul limit=256  // Maximum 256 multipliers
#pragma HLS ALLOCATION operation instances=add limit=128  // Maximum 128 adders
```

### BIND_OP
```cpp
#pragma HLS BIND_OP variable=c op=mul impl=dsp latency=3
#pragma HLS BIND_OP variable=temp op=add impl=fabric
#pragma HLS BIND_OP variable=result op=fma impl=dsp
```

### BIND_STORAGE
```cpp
#pragma HLS BIND_STORAGE variable=buffer type=RAM_1P impl=bram
#pragma HLS BIND_STORAGE variable=coeffs type=ROM_1P impl=lutram
#pragma HLS BIND_STORAGE variable=big_array type=RAM_2P impl=uram
```

### ALIAS
```cpp
#pragma HLS ALIAS ports=ptr0,ptr1,ptr2 distance=1024
#pragma HLS ALIAS ports=ptr0,ptr1,ptr2 offset=0,1024,2048
```

### CACHE
```cpp
#pragma HLS CACHE port=read_ptr lines=4 depth=32
#pragma HLS CACHE port=read_ptr lines=4 depth=32 ports=3 l2_lines=16
```

### STABLE
```cpp
#pragma HLS STABLE variable=config_reg
#pragma HLS STABLE variable=config_data off
```

### RESET
```cpp
#pragma HLS RESET variable=my_reg
#pragma HLS RESET variable=state_reg off
```

### EXPRESSION_BALANCE
```cpp
#pragma HLS EXPRESSION_BALANCE
#pragma HLS EXPRESSION_BALANCE off
```

### OCCURRENCE
```cpp
#pragma HLS OCCURRENCE cycle=4
#pragma HLS OCCURRENCE off
```

### DEPENDENCE
```cpp
#pragma HLS DEPENDENCE variable=arr type=inter distance=4
#pragma HLS DEPENDENCE variable=arr type=intra false
```

## Data Types
### Standard C/C++ Types
```cpp
// Signed types
char, short, int, long, long long

// Unsigned types
unsigned char, unsigned short, unsigned int, unsigned long, unsigned long long

// Exact width types (requires #include <stdint.h>)
int8_t, int16_t, int32_t, int64_t
uint8_t, uint16_t, uint32_t, uint64_t

// Floating point types
float, double  // Partial IEEE-754 compatible
```

### Arbitrary Precision Integers
```cpp
#include "ap_int.h"

ap_int<32> signed_32bit;      // Signed 32-bit
ap_uint<16> unsigned_16bit;   // Unsigned 16-bit
ap_int<1024> wide_int;        // Maximum 4096 bits (requires AP_INT_MAX_W definition)

// Override maximum width
#define AP_INT_MAX_W 4096
#include "ap_int.h"
ap_int<4096> very_wide_var;
```

### Arbitrary Precision Fixed-Point
```cpp
#include "ap_fixed.h"

// ap_fixed<W, I, Q, O, N>
// W: Total word length, I: Integer bits (including sign bit)
// Q: Quantization mode, O: Overflow mode, N: Saturation bits
ap_fixed<16, 8> q16_8;  // Default: AP_TRN (truncation), AP_WRAP (wrap-around)

// Specify quantization mode
ap_fixed<16, 8, AP_RND> round_plus_inf;           // Round to +infinity
ap_fixed<16, 8, AP_RND_ZERO> round_zero;          // Round to zero
ap_fixed<16, 8, AP_RND_INF> round_inf;            // Round to infinity
ap_fixed<16, 8, AP_RND_CONV> round_conv;          // Convergent rounding
ap_fixed<16, 8, AP_TRN_ZERO> trunc_zero;          // Truncate to zero

// Specify overflow mode
ap_fixed<16, 8, AP_RND, AP_SAT> sat_mode;         // Saturate
ap_fixed<16, 8, AP_RND, AP_SAT_SYM> sat_sym;      // Symmetric saturate
ap_fixed<16, 8, AP_RND, AP_SAT_ZERO> sat_zero;    // Saturate to zero
ap_fixed<16, 8, AP_RND, AP_WRAP_SM> wrap_sm;      // Sign-magnitude wrap

// Unsigned fixed-point
ap_ufixed<16, 8> unsigned_fixed;
```

### Arbitrary Precision Floating-Point
```cpp
#include "ap_float.h"

// ap_float<W, E> - W: Total bit width, E: Exponent bit width
ap_float<32, 8> fp32;    // Equivalent to float
ap_float<64, 11> fp64;   // Equivalent to double
ap_float<16, 5> fp16;    // Equivalent to half
ap_float<16, 8> bf16;    // Equivalent to bfloat16
ap_float<19, 8> tf32;    // Equivalent to TensorFloat32
```

### HLS Vector (SIMD)
```cpp
#include "hls_vector.h"

hls::vector<float, 4> vec4;  // 4-element float vector
typedef hls::vector<int, 8> t_int8Vec;

void processVec(hls::stream<t_int8Vec> &in, hls::stream<t_int8Vec> &out) {
    #pragma HLS PIPELINE II=1
    t_int8Vec a = in.read();
    t_int8Vec b = in.read();
    t_int8Vec c = a * b;  // SIMD parallel operation
    out.write(c);
}
```

### Burst Maxi Interface Type
For optimizing AXI burst access, automatically handling burst transmission alignment and length:
```cpp
#include "hls_burst_maxi.h"

// Define burst_maxi interface
hls::burst_maxi<int> mem_port;

// Function parameter example
void dut(hls::burst_maxi<int> A, hls::burst_maxi<int> B);
```
- Only valid in C simulation model, design and test bench must be stored in separate files
- Automatically optimizes burst transmission efficiency, no need to manually manage address alignment

## Synthesis Report Analysis
### Key Metrics
#### Timing Report
- **Clock Period**: Target clock period (ns)
- **Clock Uncertainty**: Clock jitter and skew
- **Effective Period**: Clock period - uncertainty
- **WNS (Worst Negative Slack)**: Worst timing margin, should be > 0
- **TNS (Total Negative Slack)**: Total timing margin, should be = 0

#### Performance Report
- **Latency**: Execution delay (number of cycles)
- **II (Initiation Interval)**: Interval between consecutive start of operations
- **Throughput**: Throughput = 1/II

#### Resource Report
- **LUT**: Look-up table usage
- **FF**: Flip-flop usage
- **BRAM**: Block RAM usage
- **URAM**: Ultra RAM usage
- **DSP**: DSP slice usage

### Common Commands
```bash
# View synthesis report
cat <project_dir>/build/kernel/syn/report/kernel_csynth.rpt

# View timing report
cat <project_dir>/build/kernel/syn/report/kernel_timing.rpt

# View resource usage
cat <project_dir>/build/kernel/syn/report/kernel_resource.rpt
```

## Reference Examples
The `examples/` directory contains official AMD Vitis HLS reference implementations organized into three categories: Design Tutorials (end-to-end applications), Feature Tutorials (specific optimization techniques), and Introductory Examples (basic feature demonstrations). When users need coding patterns, implementation examples, or best practices, read the corresponding example files first.

### Directory Structure
```
examples/
├── Design_Tutorials/              # End-to-end application design tutorials
│   └── 02-Beamformer/              # 5G massive MIMO beamformer implementation
│       ├── reference_files/        # Complete synthesis files (source, header, testbench)
│       ├── project.cfg             # HLS synthesis configuration file
│       ├── Makefile                # Build script for synthesis/ simulation
│       └── README.md               # Detailed tutorial documentation
├── Feature_Tutorials/              # Specific HLS feature and optimization tutorials
│   └── 02-Beamformer_Analysis/     # Performance analysis and optimization for beamformer
│       ├── reference_files/        # Optimized implementation files
│       ├── project.py              # Project configuration script
│       └── README.md               # Optimization technique documentation
└── Introductory_Examples/          # Basic feature and optimization examples
    ├── Array/                      # Array optimization techniques (partition, reshape, etc.)
    ├── DSP/                        # DSP kernel implementations (FIR, FFT, filters)
    ├── Interface/                  # Interface protocol examples (AXI, ap_fifo, etc.)
    ├── Migration/                  # Code migration guides from previous HLS versions
    ├── Misc/                       # Miscellaneous common patterns
    ├── Modeling/                   # Algorithmic modeling best practices
    ├── Pipelining/                 # Loop and function pipelining examples
    └── Task_level_Parallelism/     # Dataflow and task-level parallelism examples
```

### Tutorial Descriptions
#### Design Tutorials
| Tutorial | Description | Key Learning Points |
|----------|-------------|----------------------|
| **02-Beamformer** | Complete implementation of a 5G massive MIMO beamforming kernel using QR decomposition | Full HLS design flow, array partitioning, loop optimization, complex arithmetic implementation, interface configuration |

#### Feature Tutorials
| Tutorial | Description | Key Learning Points |
|----------|-------------|----------------------|
| **02-Beamformer_Analysis** | Performance analysis and optimization of the beamformer design | Design space exploration, bottleneck identification, performance vs resource tradeoffs, advanced optimization techniques |

#### Introductory Examples (By Category)
| Category | Description | Common Use Cases |
|----------|-------------|-------------------|
| **Array** | Array optimization techniques including partitioning, reshaping, and memory interface optimization | When working with large arrays, memory bottlenecks, parallel data access |
| **DSP** | Digital signal processing kernel implementations (FIR filters, FFT, matrix operations) | Implementing signal processing algorithms, DSP resource utilization optimization |
| **Interface** | All interface protocol examples (AXI4-Stream, M_AXI, S_AXILITE, ap_fifo, ap_memory) | Configuring design interfaces, integrating with other FPGA components |
| **Pipelining** | Loop and function pipelining examples with II optimization techniques | Achieving high throughput, minimizing initiation interval (II) |
| **Task_level_Parallelism** | Dataflow architecture and task-level parallelism implementation | Building multi-stage pipelines, increasing design throughput via parallel execution |
| **Modeling** | Algorithmic modeling best practices for HLS synthesis | Writing C/C++ code that maps efficiently to hardware |
| **Migration** | Code migration guides between different Vitis HLS versions | Porting existing HLS designs to newer tool versions |

### Key Best Practices from Official Examples
1. **Interface Optimization**: Use `ARRAY_RESHAPE` on input ports to maximize memory bandwidth
2. **Parallel Access**: Use `ARRAY_PARTITION` on internal arrays for full parallel access
3. **Loop Optimization**: Specify `loop_tripcount` for known loop bounds to improve optimization results
4. **Performance Tuning**: Combine `PIPELINE II=1` with partial `UNROLL` for balanced performance and resource usage
5. **Code Organization**: Extract complex operations into helper functions for better code organization and optimization
6. **Configurable Design**: Use compile-time parameters for configurable unroll factors and array dimensions
7. **Project Structure**: Use standard project structure with separate source, testbench, and configuration files for reproducible builds

---

## Unsupported C/C++ Constructs

## Optimization Strategies

### II Optimization
1. Identify bottleneck: Check Schedule Viewer
2. Resolve dependencies: Use DEPENDENCE pragma
3. Increase resources: Use ALLOCATION pragma
4. Adjust pipeline style: Try flp/frp/stp options

### Memory Bandwidth Optimization
1. Use burst_maxi interface for efficient burst access
2. Increase max_read/write_burst_length to 64/128/256
3. Increase num_read/write_outstanding to 16/32
4. Use ARRAY_PARTITION for parallel access

### Resource Balancing
1. DSP vs Fabric: BIND_OP to select implementation
2. BRAM vs URAM: BIND_STORAGE to select memory type
3. Area vs Performance: ALLOCATION to limit resource usage

## Vitis Kernel Flow

### Flow Target Differences
- `flow_target=vitis`: Generate Vitis acceleration kernel (.xo)
- `flow_target=vivado`: Generate Vivado IP Catalog IP

### Vitis Kernel Requirements
- Top function must use `extern "C"` declaration
- Interfaces must use m_axi or axis protocols
- Supports XRT (Xilinx Runtime) management

### Packaging Configuration
```ini
[hls]
syn.file=kernel.cpp
syn.top=kernel_top
flow_target=vitis

package.output_format=xo
package.vendor=xilinx.com
package.library=kernel
package.version=1.0
```

## Unsupported C/C++ Constructs
### System Calls
```cpp
// Not synthesizable
printf(), fprintf(), getc(), time(), sleep()

// Solution: Use __SYNTHESIS__ macro to exclude
#ifndef __SYNTHESIS__
    printf("Debug: %d\n", value);
#endif
```

### Dynamic Memory
```cpp
// Not synthesizable
malloc(), alloc(), free(), new, delete

// Solution: Use fixed-size arrays
#ifdef NO_SYNTH
    int* arr = malloc(64 * sizeof(int));
#else
    int _arr[64];
    int* arr = &_arr[0];
#endif
```

### Recursive Functions
```cpp
// Direct recursion not supported
unsigned foo(unsigned n) {
    if (n == 0) return 1;
    return foo(n-1);  // Error!
}

// Solution: Implement tail recursion using templates
template<int N>
struct fib {
    static int compute() { return fib<N-1>::compute() + fib<N-2>::compute(); }
};
template<>
struct fib<0> { static int compute() { return 0; } };
template<>
struct fib<1> { static int compute() { return 1; } };
```
