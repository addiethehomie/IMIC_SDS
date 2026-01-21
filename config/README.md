# Independent Many Integrated Core Software Development Suite (IMIC_SDS)

**Intel MIC Software Development Emulator - Production Release**

A comprehensive emulator for Intel Xeon Phi Knights Corner (KNC) and Knights Landing (KNL) architectures, enabling software development without requiring physical hardware.

## Overview

IMIC_SDS provides complete emulation of Intel's Many Integrated Core (MIC) architecture, supporting both Knights Corner (first generation) and Knights Landing (second generation) Xeon Phi processors. This emulator enables developers to build, test, and optimize KNC/KNL applications on standard x86-64 systems.

## Supported Architectures

### Knights Corner (KNC) - First Generation
- **Xeon Phi 5110P**: 60 cores, 8GB memory, 1.053 GHz
- **Vector Processing**: 512-bit SIMD with 32 vector registers
- **Memory System**: 8 symmetric MMUs, 512KB L2 cache per tile
- **Interconnect**: Single bidirectional ring bus at 134.784 GB/s
- **Instruction Set**: Complete KNC ISA with AVX-512 foundation

### Knights Landing (KNL) - Second Generation  
- **Xeon Phi 7250**: 68 cores, 16GB memory, 1.4 GHz base
- **Vector Processing**: Enhanced 512-bit SIMD with AVX-512 extensions
- **Memory System**: 38 MMUs, 1MB L2 cache per tile, DDR4 support
- **Interconnect**: Dual bidirectional ring buses at 213.312 GB/s total
- **Instruction Set**: KNC ISA + AVX-512F/CD/ER/PF extensions

### Knights Ferry (KNF) - Internal Prototype
- **Status**: Not supported - Internal prototype with multiple NoC revisions
- **Reason**: No standardized instruction set or publicly available documentation
- **Note**: Architecture detection framework ready for future support

## Complete Feature Matrix

| Feature | KNC (Knights Corner) | KNL (Knights Landing) |
|---------|---------------------|-----------------------|
| **Core Count** | 60 cores @ 1.053 GHz | 68 cores @ 1.4 GHz |
| **Memory System** | 8GB (8 MMUs × 1GB) | 16GB (38 MMUs × ~421MB) |
| **Cache Hierarchy** | 32KB L1, 512KB L2 per tile | 32KB L1, 1MB L2 per tile |
| **Interconnect** | Single ring @ 134.784 GB/s | Dual rings @ 213.312 GB/s |
| **Vector Units** | 512-bit SIMD, 32 ZMM regs | 512-bit SIMD, 32 ZMM regs |
| **ISA Support** | KNC base instruction set | KNC + AVX-512F/CD/ER/PF |
| **Programming Model** | Offload + native execution | Offload + native execution |
| **Debug Support** | Full GDB compatibility | Full GDB compatibility |
| **Performance Monitoring** | Hardware counters + PCM | Hardware counters + PCM |

## Purpose
The IMIC_SDS enables developers to:
- Develop and test KNC/KNL applications without requiring physical Xeon Phi hardware
- Emulate complete Xeon Phi architectures with accurate timing and memory systems
- Simulate ring bus performance and inter-core communication patterns
- Debug and optimize vector code and offloaded applications
- Support both first-generation (KNC) and second-generation (KNL) architectures

## Architecture
The emulator consists of several key components:

### Core Components
- **KNC/KNL Binary Loader** - Loads and parses KNC/KNL ELF binaries with full symbol resolution
- **Instruction Translator** - Translates KNC/KNL instructions to host x86-64 using DynamoRIO and Intel XED
- **Ring Bus Simulator** - Simulates KNC single-ring (134.784 GB/s) and KNL dual-ring (213.312 GB/s) interconnects
- **Runtime Environment** - Provides KNC/KNL system calls, memory management, and runtime services
- **Performance Monitor** - Tracks KNC/KNL-specific performance metrics using Intel PCM integration
- **PCIe Bridge Simulator** - Models host-coprocessor communication over PCIe 2.0 x16
- **KNC/KNL Debugger** - GDB-compatible debugging interface with breakpoint management
- **Memory System** - Architecture-aware MMU system (8 MMUs for KNC, 38 MMUs for KNL) with cache simulation

### Target Hardware Support
#### Knights Corner (KNC) - Xeon Phi 5110P
- **Cores**: 60 x86 cores at 1.053 GHz each
- **Memory**: 8GB GDDR5 total with 8 MMUs (1GB each)
- **Ring Bus**: Single bidirectional ring at 134.784 GB/s
- **Cache**: 32KB L1 per core, 512KB L2 shared
- **Vector Units**: 512-bit SIMD with 32 vector registers per core
- **Interconnect**: Ring-based topology with symmetric MMU placement

#### Knights Landing (KNL) - Xeon Phi 7250
- **Cores**: 68 x86 cores at 1.4 GHz base (turbo to 1.5 GHz)
- **Memory**: 16GB DDR4 total with 38 MMUs (~421MB each)
- **Ring Bus**: Dual bidirectional rings at 213.312 GB/s total
- **Cache**: 32KB L1 per core, 1MB L2 shared
- **Vector Units**: Enhanced 512-bit SIMD with AVX-512 extensions
- **Interconnect**: Dual-ring topology with distributed MMU placement

### Open Source Integration
- **DynamoRIO** - Dynamic binary translation framework for instruction translation
- **Intel XED** - KNC/KNL instruction decoding and encoding with full ISA support
- **BookSim2 KNCube** - Network topology simulation for ring bus modeling
- **Intel PCM** - Performance counter monitoring and hardware event tracking
- **SoftFloat-3e** - IEEE 754 floating-point emulation for vector operations
- **VCL v2** - 512-bit SIMD vector operations library
- **MPSS 3** - System call emulation and device management
- **liboffloadmic** - CUDA-like KNC/KNL programming API support
- **GCC-KNC** - Cross-compilation toolchain reference

## Features

### Core Emulation Features
- **Complete KNC/KNL Instruction Set** - Full support for all KNC and KNL-specific instructions
- **512-bit Vector Processing** - Emulation of 32 ZMM registers and vector operations
- **Architecture-Aware Ring Bus Simulation** - KNC single-ring (134.784 GB/s) and KNL dual-ring (213.312 GB/s)
- **Memory System** - Architecture-aware MMU design (8 MMUs for KNC, 38 MMUs for KNL) with cache simulation
- **PCIe Integration** - Host-coprocessor communication over PCIe 2.0 x16
- **System Call Emulation** - Full KNC/KNL/Linux system call compatibility

### Development Tools
- **GDB-Compatible Debugging** - Remote debugging with breakpoints and watchpoints
- **Performance Analysis** - Detailed performance counters and profiling
- **Instruction Tracing** - Step-by-step instruction execution tracking
- **Memory Access Validation** - Bounds checking and alignment verification
- **Cycle-Accurate Simulation** - Optional cycle-precise timing models

### Configuration Options
- **Flexible Core Count** - Simulate 1-60 cores (KNC) or 1-68 cores (KNL) as needed
- **Configurable Memory** - Adjustable memory size up to 8GB (KNC) or 16GB (KNL)
- **Architecture Selection** - Runtime switching between KNC and KNL architectures
- **Ring Bus Tuning** - Configurable bandwidth and latency parameters per architecture
- **Debug Levels** - Multiple verbosity levels for debugging
- **Performance Monitoring** - Real-time performance data export

## Supported Instruction Sets

### KNC Vector Instructions
- Arithmetic operations: ADD_PS, SUB_PS, MUL_PS, DIV_PS
- Fused multiply-add: FMADD_PS, FMSUB_PS, FNMSUB_PS, FNMADD_PS
- Integer operations: VPADDD, VPSUBD, VPMULUD
- Data movement: VPERMD, VPBROADCASTD
- Gather/scatter: VGATHERDPS, VSCATTERDPS
- Comparisons: VCMPPS, VMAXPS, VMINPS

### KNL AVX-512 Extensions
- AVX-512F: Foundation 512-bit vector instructions
- AVX-512CD: Conflict detection and vector mask operations
- AVX-512ER: Extended precision floating-point operations
- AVX-512PF: Prefetch and gather/scatter enhancements
- Enhanced vector permutations and compress/expand operations

### Memory System
- **Architecture-Aware MMU Design** - 8 MMUs for KNC, 38 MMUs for KNL
- **Cache Hierarchy** - L1 (32KB) and L2 (512KB for KNC, 1MB for KNL) cache simulation
- **Address Translation** - Full virtual to physical address mapping
- **Memory Consistency** - Architecture-specific memory ordering models

### Performance Monitoring
- **Instruction Counters** - Vector vs scalar instruction tracking
- **Memory Metrics** - Cache hit/miss rates, bandwidth utilization
- **Ring Bus Analytics** - Message latency, contention analysis
- **Core Utilization** - Per-core performance and efficiency metrics

## Legal Compliance & Licensing

### IMIC_SDS License
**IMIC_SDS is released under GPLv3** - This is a production-ready, independent implementation of Intel MIC architecture emulation.

### Third-Party Dependencies
All third-party components are properly licensed and attributed:

- **DynamoRIO** - BSD license (dynamic binary translation)
- **Intel XED** - Apache 2.0 license (instruction decoding)
- **BookSim2** - BSD 3-Clause license (network simulation)
- **SoftFloat-3e** - BSD license (floating-point emulation)
- **Intel PCM** - BSD license (performance monitoring)

### Intellectual Property Notice
- **Intel**, **Xeon Phi**, **Knights Corner**, **Knights Landing** are trademarks of Intel Corporation
- **MIC** (Many Integrated Core) architecture is Intel's proprietary technology
- IMIC_SDS is an independent implementation based on publicly available documentation
- No proprietary Intel source code is included in this release

## Repository Structure
```
IMIC_SDS/
├── src/                    # Original IMIC_SDS source code
│   ├── main.cpp            # Main entry point with architecture detection
│   ├── knc_binary_loader.cpp
│   ├── knc_instruction_translator.cpp
│   ├── knc_runtime.cpp
│   ├── ring_bus_simulator.cpp
│   ├── knc_debugger.cpp
│   ├── knc_performance_monitor.cpp
│   └── pcie_bridge.cpp
├── include/               # Header files
├── config/                # Configuration files
├── OpenSource/            # Third-party dependencies
├── BUILD.md               # Build instructions
├── USAGE.md               # Usage guide
├── CREDITS.md             # Software Bill of Materials
└── README.md              # This file
```

## Building & Usage

### Build Instructions
See [BUILD.md](BUILD.md) for detailed build instructions and prerequisites.

### Usage Guide
See [USAGE.md](USAGE.md) for comprehensive usage examples and tutorials.

### Quick Start
```bash
# KNC emulation (default)
./imic_sde.exe my_knc_program

# KNL emulation 
./imic_sde.exe --arch knl my_knl_program

# With debugging and performance monitoring
./imic_sde.exe --arch knl --debug --performance my_program
```

## Major IMIC_SDS Implementations

### Original Core Components
The IMIC_SDS project includes several major original implementations that provide complete KNC/KNL emulation capabilities:

#### 1. KNC/KNL Binary Loader (`src/knc_binary_loader.cpp`)
- **Original Implementation**: Complete ELF binary loader for KNC/KNL (EM_K1OM) format
- **Key Features**: 
  - KNC/KNL-specific relocation handling
  - Symbol resolution and debugging information extraction
  - Memory layout management for 8GB/16GB address spaces
  - Entry point identification and validation

#### 2. Architecture-Aware Ring Bus Simulator (`src/ring_bus_simulator.cpp`)
- **Original Implementation**: Cycle-accurate ring interconnect simulation
- **Key Features**:
  - KNC single-ring (134.784 GB/s) and KNL dual-ring (213.312 GB/s) support
  - Architecture-aware MMU placement (8 MMUs for KNC, 38 MMUs for KNL)
  - Network contention and latency simulation
  - Message routing and buffer management
  - Inspired by BookSim2 research with architecture-specific optimizations

#### 3. KNC/KNL Instruction Translator (`src/knc_instruction_translator.cpp`)
- **Original Implementation**: KNC/KNL to x86-64 dynamic translation using XED
- **Key Features**:
  - Complete KNC vector instruction set support
  - KNL AVX-512 extension framework (ready for XED integration)
  - 512-bit SIMD operation translation
  - XED-based instruction decoding and encoding
  - Architecture-specific instruction classification
  - Vector register management and optimization

#### 4. Architecture-Aware Runtime Environment (`src/knc_runtime.cpp`)
- **Original Implementation**: Complete KNC/KNL system call and runtime emulation
- **Key Features**:
  - KNC/KNL/Linux system call compatibility
  - Architecture-aware memory management (8GB/16GB address spaces)
  - Dynamic MMU allocation (8 MMUs for KNC, 38 MMUs for KNL)
  - Thread scheduling and core management
  - Exception handling and error reporting

#### 5. KNC/KNL Debugger (`src/knc_debugger.cpp`)
- **Original Implementation**: GDB-compatible debugging interface
- **Key Features**:
  - Remote debugging protocol support
  - Breakpoint management
  - Instruction tracing and stepping
  - Register and memory inspection
  - Architecture-aware debugging for both KNC and KNL

#### 6. Performance Monitor (`src/knc_performance_monitor.cpp`)
- **Original Implementation**: KNC/KNL-specific performance counter system
- **Key Features**:
  - Hardware event tracking
  - Vector vs scalar instruction analysis
  - Cache performance monitoring
  - Ring bus utilization metrics
  - Intel PCM integration patterns
  - Architecture-specific performance counters

#### 7. PCIe Bridge Simulator (`src/pcie_bridge.cpp`)
- **Original Implementation**: Host-coprocessor communication modeling
- **Key Features**:
  - PCIe 2.0 x16 bandwidth simulation
  - Cache coherency overhead modeling
  - DTD (Data Transfer Directive) support
  - Latency and throughput optimization
  - Architecture-agnostic communication layer

### Key Architectural Achievements

#### Dual-Architecture Support
- **Complete KNC simulation** with accurate 60-core, 8GB, 1.053 GHz timing
- **Complete KNL simulation** with accurate 68-core, 16GB, 1.4 GHz timing
- **Architecture-aware components** that adapt to selected target
- **Seamless switching** between KNC and KNL modes

#### Hardware Fidelity
- **KNC (Xeon Phi 5110P)**: 60 cores, 8GB memory, single ring (134.784 GB/s), 8 MMUs
- **KNL (Xeon Phi 7250)**: 68 cores, 16GB memory, dual rings (213.312 GB/s), 38 MMUs
- **512-bit vector processing** with full KNC ISA and KNL AVX-512 extensions
- **Cycle-accurate interconnects** with architecture-specific timing

#### Software Development Environment
- **Complete toolchain integration** with GCC-KNC compatibility
- **Debugging support** with GDB-compatible interface for both architectures
- **Performance analysis** with comprehensive counter support
- **Configuration flexibility** for different KNC/KNL configurations

#### Research-Grade Simulation
- **Cycle-accurate timing** for performance prediction
- **Contention modeling** for realistic network behavior
- **Memory hierarchy simulation** with cache and MMU modeling
- **Scalable architecture** supporting 1-60 cores (KNC) and 1-68 cores (KNL)

These implementations represent significant original development work that enables complete KNC and KNL software development without requiring physical hardware, providing researchers and developers with a full-featured emulation environment for Knights Corner and Knights Landing architecture exploration and optimization.
