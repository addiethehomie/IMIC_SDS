# IMIC_SDE Usage Guide

**Intel MIC Software Development Emulator - Usage Instructions**

This guide provides comprehensive usage instructions for the IMIC_SDE emulator, supporting both Knights Corner (KNC) and Knights Landing (KNL) architectures.

## Overview
IMIC_SDE (Intel MIC Software Development Emulator) provides a complete environment for developing and testing KNC (Knights Corner) applications without requiring physical KNC hardware. This emulator specifically targets the Intel Xeon Phi 5110P architecture with 60 cores at 1.053 GHz and 8GB of memory.

## Repository Information
This guide is for the IMIC_SDE repository which contains:
- Complete KNC emulation environment
- Xeon Phi 5110P specific simulation
- Ring bus interconnect modeling
- Performance monitoring and debugging tools
- Full KNC instruction set support

## Quick Start

### Basic Usage
```bash
# KNC emulation (default)
./imic_sde.exe my_knc_program

# KNL emulation 
./imic_sde.exe --arch knl my_knl_program

# With debugging and performance monitoring
./imic_sde.exe --arch knl --debug --performance my_program
```

### Architecture Selection
```bash
# Knights Corner (60 cores, 8GB)
./imic_sde.exe --arch knc [options] binary

# Knights Landing (72 cores, 16GB)  
./imic_sde.exe --arch knl [options] binary
```

## Command Line Options

| Option | Short | Description |
|---------|--------|-------------|
| --help | -h | Show help message |
| --debug | -d | Enable interactive debugging |
| --performance | -p | Enable performance monitoring |
| --ring-bus | -r | Enable ring bus simulation |
| --cores <num> | -c | Number of cores to simulate (1-60) |
| --memory <size> | -m | Memory size in MB (max 6144) |
| --config <file> | -f | Configuration file |

## Debugging Mode

When debugging is enabled, IMIC_SDE provides an interactive command-line debugger:

### Debug Commands
- `continue` or `c` - Continue execution
- `step` or `s` - Single step execution
- `step_over` or `so` - Step over function calls
- `step_out` or `so` - Step out of current function
- `break <address>` - Set breakpoint at address
- `watch <address> <size>` - Set memory watchpoint
- `registers` or `r` - Show current registers
- `memory <address> <size>` - Display memory contents
- `disassemble <address> <count>` - Disassemble instructions
- `info` - Show debugging information
- `quit` or `q` - Exit debugger

### Debug Examples
```bash
# Set breakpoint at main function
(gdb) break 0x401000

# Continue execution
(gdb) continue

# Step through instructions
(gdb) step
(gdb) step

# View registers
(gdb) registers

# Examine memory
(gdb) memory 0x600000 64

# Disassemble current location
(gdb) disassemble 0x401000 10
```

## Performance Monitoring

When performance monitoring is enabled, IMIC_SDE tracks:

### Metrics Collected
- **Instructions Retired**: Total instructions executed
- **Vector Instructions**: Count of SIMD/vector operations
- **Memory Accesses**: Total memory read/write operations
- **Cache Performance**: L1/L2 hit rates and miss counts
- **Ring Bus Activity**: Inter-tile communication statistics
- **IPC**: Instructions per cycle
- **Branch Prediction**: Branch accuracy metrics

### Performance Report Example
```
=== Performance Statistics ===
Instructions retired: 1,234,567
Vector instructions: 456,789
Memory accesses: 234,567
L1 hits: 198,234
L1 misses: 36,333
L2 hits: 32,456
L2 misses: 3,889
Ring bus transactions: 12,345
Total cycles: 987,654
IPC: 1.25
```

## Ring Bus Simulation

The ring bus simulator models KNC's bidirectional ring interconnect:

### Features
- **Cycle-accurate timing**: Models actual KNC network latency
- **Contention modeling**: Simulates network congestion
- **Bidirectional routing**: Optimizes message paths
- **Bandwidth limits**: Enforces realistic bandwidth constraints

### Ring Bus Statistics
```bash
=== Ring Bus Performance Statistics ===
Total messages: 45,678
Total bytes: 2,345,678
Average message size: 51 bytes
Average latency: 12 cycles
Maximum contention delay: 8 cycles
Simulation time: 123,456 cycles
```

## KNC Binary Development

### Compiling KNC Code
Use the provided GCC-KNC cross-compiler:

```bash
# Compile for KNC target
k1om-mpss-linux-gcc -O3 -march=knc -o my_program my_program.c

# Compile with AVX-512 intrinsics
k1om-mpss-linux-gcc -O3 -mavx512f -o vector_program vector_program.c
```

### KNC Vector Intrinsics
```c
#include <immintrin.h>

// 512-bit vector operations
__m512d a = _mm512_load_pd(data);
__m512d b = _mm512_load_pd(data + 8);
__m512d result = _mm512_add_pd(a, b);

// KNC-specific operations
__m512i mask = _mm512_cmpgt_epi_mask(a, b);
__m512d gathered = _mm512_mask_i32gather_pd(indexes, mask, data);
```

### Offloading Support
```c
#include <offload.h>

#pragma offload target(mic)
// KNC code runs here
{
    // Parallel computation on KNC
    #pragma omp parallel for
    for (int i = 0; i < N; i++) {
        result[i] = compute(data[i]);
    }
}
```

## Configuration Files

### Configuration Format
```ini
[general]
cores = 60
memory = 6144
enable_debugging = false
enable_performance = true
enable_ring_bus = true

[ring_bus]
bandwidth_mbps = 51200
latency_cycles = 2
buffer_size = 1024
enable_contention = true

[performance]
enable_pcm = true
sampling_interval = 1000
export_csv = true
export_filename = performance.csv
```

## Integration with Development Tools

### GDB Integration
IMIC_SDE can be used with GDB for remote debugging:
```bash
# Start IMIC_SDE with GDB server
./imic_sde --gdb-server 1234 my_program

# Connect with GDB
gdb my_program
(gdb) target remote localhost:1234
```

### Intel VTune Integration
Performance data can be exported for VTune analysis:
```bash
./imic_sde --performance --export-vtune my_program
# Generates vtune_data.csv for import
```

## Troubleshooting

### Common Issues

#### Binary Loading Errors
```
Error: Invalid ELF magic number
```
**Solution**: Ensure binary is compiled for KNC (k1om) architecture

#### Memory Access Violations
```
Error: Memory access violation at 0x...
```
**Solution**: Check array bounds and pointer arithmetic

#### Performance Issues
```
Warning: Low cache hit rate
```
**Solution**: Optimize memory access patterns, consider data layout

### Debug Mode Tips
1. **Start with breakpoints** at main function entry
2. **Use step execution** to trace program flow
3. **Monitor registers** to track variable values
4. **Set watchpoints** for memory locations of interest
5. **Use disassembly** to understand instruction flow

### Performance Optimization Tips
1. **Vectorize operations** - Use 512-bit SIMD instructions
2. **Optimize memory layout** - Consider cache line boundaries
3. **Minimize ring bus traffic** - Group related computations
4. **Use appropriate data types** - Match KNC vector width
5. **Profile regularly** - Identify performance bottlenecks

## Examples

### Simple Vector Addition
```c
#include <immintrin.h>
#include <stdio.h>

#define N 1024

int main() {
    double a[N] __attribute__((aligned(64)));
    double b[N] __attribute__((aligned(64)));
    double c[N] __attribute__((aligned(64)));
    
    // Initialize arrays
    for (int i = 0; i < N; i++) {
        a[i] = i * 1.5;
        b[i] = i * 2.5;
    }
    
    // Vectorized addition
    for (int i = 0; i < N; i += 8) {
        __m512d va = _mm512_load_pd(&a[i]);
        __m512d vb = _mm512_load_pd(&b[i]);
        __m512d vc = _mm512_add_pd(va, vb);
        _mm512_store_pd(&c[i], vc);
    }
    
    printf("Vector addition completed\n");
    return 0;
}
```

Compile and run:
```bash
k1om-mpss-linux-gcc -O3 -mavx512f -o vector_add vector_add.c
./imic_sde --performance --ring-bus vector_add
```

This provides a complete development environment for KNC applications with full debugging, performance analysis, and ring bus simulation capabilities.
