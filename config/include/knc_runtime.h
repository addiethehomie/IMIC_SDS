#ifndef KNC_RUNTIME_H
#define KNC_RUNTIME_H

#include <vector>
#include <memory>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>

#include "knc_types.h"

// Forward declarations
class RingBusSimulator;
class KNCDebugger;
class KNCPerformanceMonitor;
class PCIeBridge;

class KNCRuntime {
private:
    // Architecture
    knc_architecture_t architecture;
    
    // Core states
    std::vector<knc_core_state_t> core_states;
    std::vector<std::thread> core_threads;
    uint8_t* memory;
    
    // MMU memory management
    knc_memory_system_t memory_system;
    std::vector<knc_mmu_t> mmus;
    std::mutex mmu_mutex;
    
    // Configuration
    uint32_t num_cores;
    uint64_t memory_size;
    
    // Synchronization
    std::atomic<bool> should_halt;
    std::atomic<uint64_t> global_cycle_count;
    std::mutex memory_mutex;
    std::condition_variable barrier_cv;
    
    // Component references
    RingBusSimulator* ring_bus;
    KNCDebugger* debugger;
    KNCPerformanceMonitor* perf_monitor;
    PCIeBridge* pcie_bridge;
    
    // Runtime state
    bool initialized;
    std::atomic<bool> running;
    
    // Core execution functions
    void execute_core(uint32_t core_id);
    knc_error_t execute_instruction(knc_core_state_t& core, uint8_t* instruction);
    knc_error_t handle_vector_instruction(knc_core_state_t& core, uint8_t* instruction);
    knc_error_t handle_scalar_instruction(knc_core_state_t& core, uint8_t* instruction);
    knc_error_t handle_memory_instruction(knc_core_state_t& core, uint8_t* instruction);
    knc_error_t handle_control_instruction(knc_core_state_t& core, uint8_t* instruction);
    
    // Memory access functions
    knc_error_t read_memory(uint64_t address, void* data, size_t size);
    knc_error_t write_memory(uint64_t address, const void* data, size_t size);
    knc_error_t read_vector_memory(uint64_t address, __m512i& data);
    knc_error_t write_vector_memory(uint64_t address, const __m512i& data);
    
    // System call handling
    knc_error_t handle_system_call(knc_core_state_t& core, knc_syscall_type_t syscall);
    knc_error_t syscall_exit(knc_core_state_t& core, int exit_code);
    knc_error_t syscall_read(knc_core_state_t& core);
    knc_error_t syscall_write(knc_core_state_t& core);
    knc_error_t syscall_open(knc_core_state_t& core);
    knc_error_t syscall_close(knc_core_state_t& core);
    knc_error_t syscall_mmap(knc_core_state_t& core);
    knc_error_t syscall_munmap(knc_core_state_t& core);
    
    // Synchronization functions
    void barrier_wait(uint32_t core_id);
    void synchronize_cores();
    void update_global_cycle_count();
    
    // Performance monitoring
    void update_performance_counters(knc_core_state_t& core, knc_instruction_type_t inst_type);
    void report_performance_stats();
    
public:
    KNCRuntime(uint32_t cores = KNC_NUM_CORES, uint64_t mem_size = KNC_MEMORY_SIZE, 
               knc_architecture_t arch = ARCH_KNC);
    ~KNCRuntime();
    
    // Initialization
    bool initialize();
    void shutdown();
    
    // Program loading
    bool load_program(const uint8_t* program_data, uint64_t program_size);
    bool set_entry_point(uint64_t entry_point);
    
    // Component registration
    void set_ring_bus_simulator(RingBusSimulator* simulator);
    void set_debugger(KNCDebugger* debugger);
    void set_performance_monitor(KNCPerformanceMonitor* monitor);
    void set_pcie_bridge(PCIeBridge* bridge);
    
    // MMU memory management (public for testing)
    uint32_t address_to_mmu(uint64_t address);
    bool is_valid_address(uint64_t address);
    knc_error_t mmu_write(uint64_t address, const void* data, size_t size);
    knc_error_t mmu_read(uint64_t address, void* data, size_t size);
    void get_mmu_stats(uint32_t mmu_id, uint64_t& accesses, uint64_t& hits, uint64_t& misses);
    
    // Execution control
    knc_error_t run();
    knc_error_t step();
    knc_error_t pause();
    knc_error_t resume();
    knc_error_t halt();
    
    // State queries
    bool is_running() const;
    uint64_t get_cycle_count() const;
    const knc_core_state_t& get_core_state(uint32_t core_id) const;
    const knc_memory_t& get_memory() const;
    
    // Debugging interface
    void dump_core_state(uint32_t core_id) const;
    void dump_memory(uint64_t address, size_t size) const;
    void dump_registers(uint32_t core_id) const;
    
    // Configuration
    void set_num_cores(uint32_t cores);
    void set_memory_size(uint64_t size);
    
    // Statistics
    void print_statistics() const;
};

#endif // KNC_RUNTIME_H
