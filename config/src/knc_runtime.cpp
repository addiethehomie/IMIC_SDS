#include "knc_runtime.h"
#include "knc_instruction_translator.h"
#include "knc_debugger.h"
#include "knc_performance_monitor.h"
#include "pcie_bridge.h"
#include <iostream>
#include <cstring>
#include <cstdlib>

// Windows compatibility
#ifdef _WIN32
#include <windows.h>
#include <io.h>
#define usleep(x) Sleep((x) / 1000)
#else
#include <unistd.h>
#endif

KNCRuntime::KNCRuntime(uint32_t cores, uint64_t mem_size, knc_architecture_t arch) 
    : num_cores(cores), memory_size(mem_size), architecture(arch) {
    should_halt.store(false);
    global_cycle_count.store(0);
    running.store(false);
    initialized = false;
    
    core_states.resize(num_cores);
    memory = nullptr;
    
    ring_bus = nullptr;
    debugger = nullptr;
    perf_monitor = nullptr;
    pcie_bridge = nullptr;
    
    // Initialize MMU system
    memory_system.total_size = memory_size;
    
    uint32_t num_mmus, mmu_size;
    if (architecture == ARCH_KNL) {
        num_mmus = KNL_NUM_MMUS;
        mmu_size = KNL_MMU_SIZE;
    } else {
        num_mmus = KNC_NUM_MMUS;
        mmu_size = KNC_MMU_SIZE;
    }
    
    memory_system.active_mmus = num_mmus;
    mmus.resize(num_mmus);
    
    for (uint32_t i = 0; i < num_mmus; i++) {
        mmus[i].mmu_id = i;
        mmus[i].base_address = i * mmu_size;
        mmus[i].size = mmu_size;
        
        // Calculate tile distribution based on architecture
        uint32_t num_tiles = (architecture == ARCH_KNL) ? KNL_NUM_TILES : KNC_NUM_TILES;
        mmus[i].tile_id = i * (num_tiles / num_mmus);
        
        mmus[i].accesses = 0;
        mmus[i].cache_hits = 0;
        mmus[i].cache_misses = 0;
    }
    
    // Initialize core states
    for (uint32_t i = 0; i < num_cores; i++) {
        memset(&core_states[i], 0, sizeof(knc_core_state_t));
        core_states[i].core_id = i;
        core_states[i].tile_id = i / KNC_CORES_PER_TILE;
        core_states[i].is_halted = true;
        core_states[i].cycles_executed = 0;
    }
}

KNCRuntime::~KNCRuntime() {
    shutdown();
}

bool KNCRuntime::initialize() {
    std::cout << "Initializing KNC Runtime with " << num_cores << " cores\n";
    std::cout << "Memory size: " << (memory_size / (1024*1024)) << " MB\n";
    
    // Allocate memory
    memory = new uint8_t[memory_size];
    if (!memory) {
        std::cerr << "Error: Failed to allocate memory\n";
        return false;
    }
    
    // Initialize memory to zero
    memset(memory, 0, memory_size);
    
    // Initialize cores
    for (uint32_t i = 0; i < num_cores; i++) {
        // Initialize vector registers to zero
        for (int j = 0; j < KNC_NUM_VECTOR_REGISTERS; j++) {
            core_states[i].registers.zmm[j] = _mm512_setzero_si512();
        }
        
        // Initialize mask registers
        for (int j = 0; j < KNC_NUM_MASK_REGISTERS; j++) {
            core_states[i].registers.k[j] = 0;  // Simple mask initialization
        }
        
        // Initialize general purpose registers
        for (int j = 0; j < 16; j++) {
            core_states[i].registers.gpr[j] = 0;
        }
        
        core_states[i].registers.rip = 0;
        core_states[i].registers.rflags = 0;
    }
    
    initialized = true;
    std::cout << "KNC Runtime initialized successfully\n";
    return true;
}

void KNCRuntime::shutdown() {
    if (running.load()) {
        halt();
    }
    
    // Wait for all threads to finish
    for (auto& thread : core_threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    
    core_threads.clear();
    initialized = false;
    std::cout << "KNC Runtime shutdown\n";
}

bool KNCRuntime::load_program(const uint8_t* program_data, uint64_t program_size) {
    if (!initialized) {
        std::cerr << "Error: Runtime not initialized\n";
        return false;
    }
    
    if (program_size > memory_size) {
        std::cerr << "Error: Program too large for memory\n";
        return false;
    }
    
    // Copy program to memory starting at address 0
    memcpy(memory, program_data, program_size);
    
    // Set entry point for all cores
    for (uint32_t i = 0; i < num_cores; i++) {
        core_states[i].registers.rip = 0;  // Entry point at address 0
        core_states[i].is_halted = false;
    }
    
    std::cout << "Program loaded: " << program_size << " bytes\n";
    return true;
}

knc_error_t KNCRuntime::run() {
    if (!initialized) {
        return KNC_ERROR_INVALID_ARGUMENT;
    }
    
    running.store(true);
    should_halt.store(false);
    
    std::cout << "Starting KNC emulation on " << num_cores << " cores\n";
    
    // Start core execution threads
    core_threads.clear();
    for (uint32_t i = 0; i < num_cores; i++) {
        core_threads.emplace_back(&KNCRuntime::execute_core, this, i);
    }
    
    // Main emulation loop
    while (running.load() && !should_halt.load()) {
        update_global_cycle_count();
        
        // Check for debugger break requests
        if (debugger && debugger->should_pause()) {
            // Handle debugger interaction
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        
        // Small delay to prevent busy waiting
        std::this_thread::sleep_for(std::chrono::microseconds(100));
    }
    
    // Wait for all cores to finish
    for (auto& thread : core_threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    
    running.store(false);
    std::cout << "KNC emulation completed\n";
    
    return KNC_SUCCESS;
}

void KNCRuntime::execute_core(uint32_t core_id) {
    knc_core_state_t& core = core_states[core_id];
    
    while (running.load() && !should_halt.load() && !core.is_halted) {
        // Fetch instruction
        uint64_t rip = core.registers.rip;
        if (rip >= memory_size) {
            std::cerr << "Core " << core_id << ": Instruction pointer out of bounds\n";
            core.is_halted = true;
            return;
        }
        
        uint8_t* instruction_ptr = memory + rip;
        
        // Execute instruction
        knc_error_t result = execute_instruction(core, instruction_ptr);
        
        if (result != KNC_SUCCESS) {
            std::cerr << "Core " << core_id << ": Execution error " << result << " at RIP 0x" 
                      << std::hex << rip << std::dec << "\n";
            core.is_halted = true;
            return;
        }
        
        // Update instruction pointer
        core.registers.rip += 1;  // Simplified - should use actual instruction length
        core.cycles_executed++;
        
        // Check for debugger breakpoints
        if (debugger && debugger->should_break(core.registers.rip, core_id)) {
            debugger->notify_breakpoint_hit(core.registers.rip, core_id);
            // Wait for debugger commands
            while (debugger->should_pause()) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }
        
        // Performance monitoring
        if (perf_monitor) {
            perf_monitor->record_cycle(core_id, 1);
        }
    }
}

knc_error_t KNCRuntime::execute_instruction(knc_core_state_t& core, uint8_t* instruction) {
    // This is a simplified instruction execution
    // In practice, this would use the instruction translator
    
    // Check for system call instruction (simplified)
    if (instruction[0] == 0x0F && instruction[1] == 0x05) {
        return handle_system_call(core, static_cast<knc_syscall_type_t>(core.registers.gpr[0]));
    }
    
    // Simple instruction execution (placeholder)
    // Real implementation would decode and execute properly
    switch (instruction[0]) {
        case 0x90:  // NOP
            return KNC_SUCCESS;
            
        case 0xC3:  // RET
            core.is_halted = true;
            return KNC_SUCCESS;
            
        default:
            // For now, just increment RIP
            return KNC_SUCCESS;
    }
}

knc_error_t KNCRuntime::handle_system_call(knc_core_state_t& core, knc_syscall_type_t syscall) {
    switch (syscall) {
        case KNC_SYSCALL_EXIT:
            return syscall_exit(core, static_cast<int>(core.registers.gpr[0]));
            
        case KNC_SYSCALL_WRITE:
            return syscall_write(core);
            
        case KNC_SYSCALL_READ:
            return syscall_read(core);
            
        default:
            std::cerr << "Core " << core.core_id << ": Unimplemented system call " << syscall << "\n";
            return KNC_ERROR_SYSTEM_CALL;
    }
}

knc_error_t KNCRuntime::syscall_exit(knc_core_state_t& core, int exit_code) {
    std::cout << "Core " << core.core_id << ": Exit with code " << exit_code << "\n";
    core.is_halted = true;
    should_halt.store(true);
    return KNC_SUCCESS;
}

knc_error_t KNCRuntime::syscall_write(knc_core_state_t& core) {
    // Simplified write system call
    // Arguments: fd (rdi), buf (rsi), count (rdx)
    uint64_t fd = core.registers.gpr[1];  // RDI
    uint64_t buf = core.registers.gpr[2];  // RSI
    uint64_t count = core.registers.gpr[3];  // RDX
    
    if (fd == 1) {  // stdout
        std::cout.write(reinterpret_cast<const char*>(memory + buf), count);
        core.registers.gpr[0] = count;  // Return value
        return KNC_SUCCESS;
    }
    
    // Other file descriptors not implemented
    core.registers.gpr[0] = -1;  // Error
    return KNC_ERROR_SYSTEM_CALL;
}

knc_error_t KNCRuntime::syscall_read(knc_core_state_t& core) {
    // Simplified read system call
    // Arguments: fd (rdi), buf (rsi), count (rdx)
    uint64_t fd = core.registers.gpr[1];  // RDI
    uint64_t buf = core.registers.gpr[2];  // RSI
    uint64_t count = core.registers.gpr[3];  // RDX
    
    if (fd == 0) {  // stdin
        // For now, return 0 bytes read
        core.registers.gpr[0] = 0;
        return KNC_SUCCESS;
    }
    
    // Other file descriptors not implemented
    core.registers.gpr[0] = -1;  // Error
    return KNC_ERROR_SYSTEM_CALL;
}

knc_error_t KNCRuntime::read_memory(uint64_t address, void* data, size_t size) {
    if (!memory || address + size > memory_size) {
        return KNC_ERROR_INVALID_ARGUMENT;
    }
    
    memcpy(data, memory + address, size);
    return KNC_SUCCESS;
}

knc_error_t KNCRuntime::write_memory(uint64_t address, const void* data, size_t size) {
    if (!memory || address + size > memory_size) {
        return KNC_ERROR_MEMORY_ACCESS;
    }
    
    std::lock_guard<std::mutex> lock(memory_mutex);
    memcpy(memory + address, data, size);
    return KNC_SUCCESS;
}

void KNCRuntime::update_global_cycle_count() {
    global_cycle_count.fetch_add(1);
}

void KNCRuntime::set_ring_bus_simulator(RingBusSimulator* simulator) {
    ring_bus = simulator;
}

void KNCRuntime::set_debugger(KNCDebugger* dbg) {
    debugger = dbg;
}

void KNCRuntime::set_performance_monitor(KNCPerformanceMonitor* monitor) {
    perf_monitor = monitor;
}

void KNCRuntime::set_pcie_bridge(PCIeBridge* bridge) {
    pcie_bridge = bridge;
}

bool KNCRuntime::is_running() const {
    return running.load();
}

uint64_t KNCRuntime::get_cycle_count() const {
    return global_cycle_count.load();
}

const knc_core_state_t& KNCRuntime::get_core_state(uint32_t core_id) const {
    if (core_id < num_cores) {
        return core_states[core_id];
    }
    static knc_core_state_t dummy_state;
    return dummy_state;
}

const knc_memory_t& KNCRuntime::get_memory() const {
    static knc_memory_t dummy_memory;
    dummy_memory.base_address = 0;
    dummy_memory.data = memory;
    dummy_memory.size = memory_size;
    return dummy_memory;
}

void KNCRuntime::dump_core_state(uint32_t core_id) const {
    if (core_id >= num_cores) {
        std::cerr << "Invalid core ID: " << core_id << "\n";
        return;
    }
    
    const knc_core_state_t& core = core_states[core_id];
    
    std::cout << "\n=== Core " << core_id << " State ===\n";
    std::cout << "Tile ID: " << core.tile_id << "\n";
    std::cout << "RIP: 0x" << std::hex << core.registers.rip << std::dec << "\n";
    std::cout << "RFLAGS: 0x" << std::hex << core.registers.rflags << std::dec << "\n";
    std::cout << "Halted: " << (core.is_halted ? "Yes" : "No") << "\n";
    std::cout << "Cycles executed: " << core.cycles_executed << "\n";
    
    std::cout << "\nGeneral Purpose Registers:\n";
    for (int i = 0; i < 16; i++) {
        std::cout << "  R" << i << ": 0x" << std::hex << core.registers.gpr[i] 
                  << std::dec << "\n";
    }
}

void KNCRuntime::print_statistics() const {
    std::cout << "\n=== KNC Runtime Statistics ===\n";
    std::cout << "Total cycles: " << global_cycle_count.load() << "\n";
    
    uint64_t total_instructions = 0;
    uint64_t active_cores = 0;
    
    for (const auto& core : core_states) {
        if (core.cycles_executed > 0) {
            active_cores++;
        }
        total_instructions += core.cycles_executed;  // Simplified
    }
    
    std::cout << "Active cores: " << active_cores << "/" << num_cores << "\n";
    std::cout << "Total instructions: " << total_instructions << "\n";
    
    if (global_cycle_count.load() > 0) {
        double avg_ipc = (double)total_instructions / global_cycle_count.load();
        std::cout << "Average IPC: " << avg_ipc << "\n";
    }
}

knc_error_t KNCRuntime::halt() {
    std::cout << "KNC Runtime halted\n";
    running = false;
    return KNC_SUCCESS;
}

// MMU Memory Management Implementation
uint32_t KNCRuntime::address_to_mmu(uint64_t address) {
    if (address >= memory_size) {
        uint32_t max_mmus = (architecture == ARCH_KNL) ? KNL_NUM_MMUS : KNC_NUM_MMUS;
        return max_mmus; // Invalid MMU
    }
    // Architecture-aware MMU distribution
    if (architecture == ARCH_KNL) {
        return address % KNL_NUM_MMUS;  // KNL uses 38 MMUs
    } else {
        return address % KNC_NUM_MMUS;  // KNC uses 8 MMUs symmetrically
    }
}

bool KNCRuntime::is_valid_address(uint64_t address) {
    return address < memory_size;
}

knc_error_t KNCRuntime::mmu_write(uint64_t address, const void* data, size_t size) {
    std::lock_guard<std::mutex> lock(mmu_mutex);
    
    if (!is_valid_address(address) || !is_valid_address(address + size - 1)) {
        return KNC_ERROR_MEMORY_ACCESS;
    }
    
    uint32_t mmu_id = address_to_mmu(address);
    uint32_t max_mmus = (architecture == ARCH_KNL) ? KNL_NUM_MMUS : KNC_NUM_MMUS;
    if (mmu_id >= max_mmus) {
        return KNC_ERROR_MEMORY_ACCESS;
    }
    
    // Transfer data through PCIe bridge if available (simulating host-to-device transfer)
    if (pcie_bridge) {
        pcie_bridge->transferDataHostToDevice(data, size, address);
    }
    
    // Update MMU statistics
    mmus[mmu_id].accesses++;
    
    // Simple cache simulation (90% hit rate)
    if ((rand() % 100) < 90) {
        mmus[mmu_id].cache_hits++;
    } else {
        mmus[mmu_id].cache_misses++;
    }
    
    // Perform the write
    memcpy(memory + address, data, size);
    
    return KNC_SUCCESS;
}

knc_error_t KNCRuntime::mmu_read(uint64_t address, void* data, size_t size) {
    std::lock_guard<std::mutex> lock(mmu_mutex);
    
    if (!is_valid_address(address) || !is_valid_address(address + size - 1)) {
        return KNC_ERROR_MEMORY_ACCESS;
    }
    
    uint32_t mmu_id = address_to_mmu(address);
    uint32_t max_mmus = (architecture == ARCH_KNL) ? KNL_NUM_MMUS : KNC_NUM_MMUS;
    if (mmu_id >= max_mmus) {
        return KNC_ERROR_MEMORY_ACCESS;
    }
    
    // Perform the read
    memcpy(data, memory + address, size);
    
    // Transfer data through PCIe bridge if available (simulating device-to-host transfer)
    if (pcie_bridge) {
        pcie_bridge->transferDataDeviceToHost(address, data, size);
    }
    
    // Update MMU statistics
    mmus[mmu_id].accesses++;
    
    // Simple cache simulation (90% hit rate)
    if ((rand() % 100) < 90) {
        mmus[mmu_id].cache_hits++;
    } else {
        mmus[mmu_id].cache_misses++;
    }
    
    return KNC_SUCCESS;
}

void KNCRuntime::get_mmu_stats(uint32_t mmu_id, uint64_t& accesses, uint64_t& hits, uint64_t& misses) {
    std::lock_guard<std::mutex> lock(mmu_mutex);
    
    uint32_t max_mmus = (architecture == ARCH_KNL) ? KNL_NUM_MMUS : KNC_NUM_MMUS;
    if (mmu_id < max_mmus) {
        accesses = mmus[mmu_id].accesses;
        hits = mmus[mmu_id].cache_hits;
        misses = mmus[mmu_id].cache_misses;
    } else {
        accesses = hits = misses = 0;
    }
}
