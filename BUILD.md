# IMIC_SDE Build Instructions

**Intel MIC Software Development Emulator - Build Guide**

This guide provides instructions for building the IMIC_SDE emulator, supporting both Knights Corner (KNC) and Knights Landing (KNL) architectures.

## Overview
IMIC_SDE emulates Intel Xeon Phi processors with full KNC/KNL instruction set support, ring bus simulation, and comprehensive debugging capabilities.

## Repository Structure
```
IMIC_SDE/
├── src/                    # Source files
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
└── OpenSource/            # Third-party dependencies
```

## Prerequisites
- **GCC 7.0+** with AVX-512 support (or compatible C++17 compiler)
- **Windows 10/11** (Linux support with minor modifications)
- **CMake 3.10+** (optional, for CMake builds)
- **No external dependencies required** - all headers included

## Build Methods

### Method 1: Direct Compilation (Recommended)
```bash
g++ -std=c++17 -mavx512f -Iinclude \
    src/main.cpp src/knc_binary_loader.cpp \
    src/knc_instruction_translator.cpp src/knc_runtime.cpp \
    src/ring_bus_simulator.cpp src/knc_debugger.cpp \
    src/knc_performance_monitor.cpp src/pcie_bridge.cpp \
    -o imic_sde.exe
```

### Method 2: Using CMake
```bash
mkdir build
cd build
cmake ..
make  # or 'cmake --build .' on Windows
```

### Method 3: Using Makefile (if available)
```bash
make -f Makefile.win  # Windows
# or
make                  # Linux/Unix
```

## Verification
```bash
./imic_sde.exe --help
./imic_sde.exe --debug --cores 4 --memory 8192 test_binary
```

## Configuration
The emulator can be configured via:
- Command line arguments
- Configuration files (config/imic_sde.conf)
- Environment variables

## Supported Hardware

### Knights Corner (KNC) - Xeon Phi 5110P
- **Memory**: 8GB total with 8 MMUs (1GB each)
- **MMU Placement**: Symmetrically distributed around ring bus
- **Cores**: 60 x86 cores at 1.053 GHz
- **Ring Bus**: Single bidirectional ring at 134.784 GB/s

### Knights Landing (KNL) - Xeon Phi 7250
- **Memory**: 16GB total with 38 MMUs (~421MB each)
- **MMU Placement**: Distributed around dual ring buses
- **Cores**: 68 x86 cores at 1.4 GHz base (turbo to 1.5 GHz)
- **Ring Bus**: Dual bidirectional rings at 213.312 GB/s total
