/*
 * Copyright (c) 2026 IMIC_SDS Development Team
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <iostream>
#include <string>
#include <cstring>
#include <getopt.h>

#include "knc_types.h"
#include "knc_binary_loader.h"
#include "knc_runtime.h"
#include "ring_bus_simulator.h"
#include "knc_debugger.h"
#include "knc_performance_monitor.h"

// Configuration structure
struct imic_sde_config {
    std::string binary_path;
    bool enable_debugging;
    bool enable_performance_monitoring;
    bool enable_ring_bus_simulation;
    knc_architecture_t target_architecture;
    uint32_t num_cores;
    uint64_t memory_size;
    std::string config_file;
};

// Print usage information
void print_usage(const char* program_name) {
    std::cout << "IMIC_SDS - Independent Many Integrated Core Software Development Suite\n";
    std::cout << "Supports KNC, KNL architectures\n\n";
    std::cout << "Usage: " << program_name << " [options] <knc_binary>\n\n";
    std::cout << "Options:\n";
    std::cout << "  -h, --help                    Show this help message\n";
    std::cout << "  -d, --debug                   Enable debugging mode\n";
    std::cout << "  -p, --performance             Enable performance monitoring\n";
    std::cout << "  -r, --ring-bus               Enable ring bus simulation\n";
    std::cout << "  -a, --arch <architecture>     Target architecture (knc, knl)\n";
    std::cout << "  -c, --cores <num>             Number of cores to simulate (default: auto)\n";
    std::cout << "  -m, --memory <size>            Memory size in MB (default: auto)\n";
    std::cout << "  -f, --config <file>           Configuration file\n";
    std::cout << "\nArchitectures:\n";
    std::cout << "  knc - Knights Corner (Xeon Phi 5110P, 60 cores, 8GB)\n";
    std::cout << "  knl - Knights Landing (Xeon Phi 7250, 68 cores, 16GB)\n";
    std::cout << "\nExamples:\n";
    std::cout << "  " << program_name << " --arch knl --debug --performance my_knl_program\n";
    std::cout << "  " << program_name << " --ring-bus --cores 30 vector_benchmark\n";
}

// Parse command line arguments
bool parse_arguments(int argc, char* argv[], struct imic_sde_config& config) {
    // Set defaults
    config.enable_debugging = false;
    config.enable_performance_monitoring = false;
    config.enable_ring_bus_simulation = false;
    config.target_architecture = detect_host_architecture();
    config.num_cores = get_num_cores(config.target_architecture);
    config.memory_size = get_memory_size(config.target_architecture);
    config.config_file = "config/imic_sde.conf"; // Relative path
    
    static struct option long_options[] = {
        {"help", no_argument, 0, 'h'},
        {"debug", no_argument, 0, 'd'},
        {"performance", no_argument, 0, 'p'},
        {"ring-bus", no_argument, 0, 'r'},
        {"arch", required_argument, 0, 'a'},
        {"cores", required_argument, 0, 'c'},
        {"memory", required_argument, 0, 'm'},
        {"config", required_argument, 0, 'f'},
        {0, 0, 0, 0}
    };
    
    int option_index = 0;
    int c;
    
    while ((c = getopt_long(argc, argv, "hdpr:a:c:m:f:", long_options, &option_index)) != -1) {
        switch (c) {
            case 'h':
                print_usage(argv[0]);
                return false;
            case 'd':
                config.enable_debugging = true;
                break;
            case 'p':
                config.enable_performance_monitoring = true;
                break;
            case 'r':
                config.enable_ring_bus_simulation = true;
                break;
            case 'a':
                if (strcmp(optarg, "knc") == 0) {
                    config.target_architecture = ARCH_KNC;
                } else if (strcmp(optarg, "knl") == 0) {
                    config.target_architecture = ARCH_KNL;
                } else {
                    std::cerr << "Error: Unsupported architecture '" << optarg << "'. Supported: knc, knl\n";
                    return false;
                }
                // Update cores and memory based on architecture
                config.num_cores = get_num_cores(config.target_architecture);
                config.memory_size = get_memory_size(config.target_architecture);
                break;
            case 'c':
                config.num_cores = static_cast<uint32_t>(strtoul(optarg, nullptr, 10));
                break;
            case 'm':
                config.memory_size = static_cast<uint64_t>(strtoull(optarg, nullptr, 10)) * 1024 * 1024;
                break;
            case 'f':
                config.config_file = std::string(optarg);
                break;
            case '?':
                return false;
            default:
                return false;
        }
    }
    
    // Validate configuration
    uint32_t max_cores = get_num_cores(config.target_architecture);
    uint64_t max_memory = get_memory_size(config.target_architecture);
    
    if (config.num_cores == 0) {
        std::cerr << "Error: Number of cores must be at least 1\n";
        return false;
    }
    
    if (config.memory_size == 0) {
        std::cerr << "Error: Memory size must be at least 1 MB\n";
        return false;
    }
    
    if (config.num_cores > max_cores) {
        std::cerr << "Error: Number of cores cannot exceed " << max_cores << " for " 
                  << get_architecture_name(config.target_architecture) << "\n";
        return false;
    }
    
    if (config.memory_size > max_memory) {
        std::cerr << "Error: Memory size cannot exceed " << (max_memory / (1024*1024)) << " MB for "
                  << get_architecture_name(config.target_architecture) << "\n";
        return false;
    }
    
    // Get binary path
    if (optind >= argc) {
        std::cerr << "Error: No KNC binary specified\n";
        return false;
    }
    
    config.binary_path = std::string(argv[optind]);
    return true;
}

// Main emulation loop
int run_emulation(const struct imic_sde_config& config) {
    std::cout << "Starting IMIC_SDS emulation...\n";
    std::cout << "Binary: " << config.binary_path << "\n";
    std::cout << "Architecture: " << get_architecture_name(config.target_architecture) << "\n";
    std::cout << "Cores: " << config.num_cores << "\n";
    std::cout << "Memory: " << (config.memory_size / (1024*1024)) << " MB\n";
    
    // Initialize components
    KNCBinaryLoader loader;
    KNCRuntime runtime(config.num_cores, config.memory_size, config.target_architecture);
    RingBusSimulator ring_bus(config.num_cores, config.target_architecture);
    KNCDebugger debugger;
    KNCPerformanceMonitor perf_monitor;
    
    // Load KNC binary
    if (!loader.load_binary(config.binary_path)) {
        std::cerr << "Error: Failed to load KNC binary: " << config.binary_path << "\n";
        return -1;
    }
    
    // Initialize runtime
    if (!runtime.initialize()) {
        std::cerr << "Error: Failed to initialize KNC runtime\n";
        return -1;
    }
    
    // Initialize ring bus simulator if requested
    if (config.enable_ring_bus_simulation) {
        if (!ring_bus.initialize()) {
            std::cerr << "Error: Failed to initialize ring bus simulator\n";
            return -1;
        }
        runtime.set_ring_bus_simulator(&ring_bus);
    }
    
    // Initialize debugger if requested
    if (config.enable_debugging) {
        if (!debugger.initialize()) {
            std::cerr << "Error: Failed to initialize debugger\n";
            return -1;
        }
        runtime.set_debugger(&debugger);
    }
    
    // Initialize performance monitor if requested
    if (config.enable_performance_monitoring) {
        if (!perf_monitor.initialize()) {
            std::cerr << "Error: Failed to initialize performance monitor\n";
            return -1;
        }
        runtime.set_performance_monitor(&perf_monitor);
    }
    
    // Load binary into memory
    if (!runtime.load_program(loader.get_binary_data(), loader.get_binary_size())) {
        std::cerr << "Error: Failed to load program into memory\n";
        return -1;
    }
    
    std::cout << "Emulation started. Press Ctrl+C to stop.\n";
    
    // Run emulation
    knc_error_t result = runtime.run();
    
    // Print final statistics
    if (config.enable_performance_monitoring) {
        knc_performance_counters_t counters = perf_monitor.get_aggregate_counters();
        std::cout << "\n=== Performance Statistics ===\n";
        std::cout << "Instructions retired: " << counters.instructions_retired << "\n";
        std::cout << "Vector instructions: " << counters.vector_instructions << "\n";
        std::cout << "Memory accesses: " << counters.memory_accesses << "\n";
        std::cout << "L1 hits: " << counters.l1_hits << "\n";
        std::cout << "L1 misses: " << counters.l1_misses << "\n";
        std::cout << "L2 hits: " << counters.l2_hits << "\n";
        std::cout << "L2 misses: " << counters.l2_misses << "\n";
        std::cout << "Ring bus transactions: " << counters.ring_bus_transactions << "\n";
        std::cout << "Total cycles: " << counters.cycles << "\n";
        
        if (counters.cycles > 0) {
            double ipc = (double)counters.instructions_retired / counters.cycles;
            std::cout << "IPC: " << ipc << "\n";
        }
    }
    
    std::cout << "Emulation " << (result == KNC_SUCCESS ? "completed successfully" : "failed with error") << "\n";
    return result;
}

int main(int argc, char* argv[]) {
    struct imic_sde_config config;
    
    if (!parse_arguments(argc, argv, config)) {
        return -1;
    }
    
    return run_emulation(config);
}
