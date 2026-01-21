#include "knc_performance_monitor.h"
#include <iostream>
#include <fstream>
#include <iomanip>
#include <algorithm>
#include <cstring>
#include <mutex>
#include <thread>

KNCPerformanceMonitor::KNCPerformanceMonitor(uint32_t cores) : num_cores(cores) {
    monitoring_enabled = false;
    collection_active.store(false);
    
    core_data.resize(num_cores);
    pcm_handle = nullptr;
    
    start_time = std::chrono::high_resolution_clock::now();
    last_update = start_time;
    
    // Initialize aggregate counters
    memset(&aggregate_counters_val, 0, sizeof(aggregate_counters_val));
}

KNCPerformanceMonitor::~KNCPerformanceMonitor() {
    shutdown();
}

bool KNCPerformanceMonitor::initialize() {
    std::cout << "KNC Performance Monitor initialized for " << num_cores << " cores\n";
    
    initialize_counters();
    reset_counters();
    
    return true;
}

void KNCPerformanceMonitor::shutdown() {
    stop_collection();
    cleanup_pcm();
}

void KNCPerformanceMonitor::initialize_counters() {
    counter_configs.clear();
    
    // Initialize default counter configurations
    knc_perf_counter_config_t config;
    
    config.event_type = KNC_PERF_INSTRUCTIONS_RETIRED;
    config.enabled = true;
    config.core_mask = 0xFFFFFFFF;
    config.threshold = 0;
    config.overflow_enabled = false;
    counter_configs.push_back(config);
    
    config.event_type = KNC_PERF_VECTOR_INSTRUCTIONS;
    counter_configs.push_back(config);
    
    config.event_type = KNC_PERF_MEMORY_ACCESSES;
    counter_configs.push_back(config);
    
    config.event_type = KNC_PERF_L1_HITS;
    counter_configs.push_back(config);
    
    config.event_type = KNC_PERF_L1_MISSES;
    counter_configs.push_back(config);
    
    config.event_type = KNC_PERF_L2_HITS;
    counter_configs.push_back(config);
    
    config.event_type = KNC_PERF_L2_MISSES;
    counter_configs.push_back(config);
    
    config.event_type = KNC_PERF_RING_BUS_TRANSACTIONS;
    counter_configs.push_back(config);
    
    config.event_type = KNC_PERF_CYCLES;
    counter_configs.push_back(config);
}

void KNCPerformanceMonitor::reset_counters() {
    std::lock_guard<std::mutex> lock(data_mutex);
    
    for (auto& core : core_data) {
        memset(&core, 0, sizeof(core));
        core.last_update_time = 0;
    }
    
    memset(&aggregate_counters_val, 0, sizeof(aggregate_counters_val));
}

void KNCPerformanceMonitor::enable_monitoring(bool enable) {
    monitoring_enabled = enable;
}

bool KNCPerformanceMonitor::is_monitoring_enabled() const {
    return monitoring_enabled;
}

void KNCPerformanceMonitor::record_instruction(uint32_t core_id, knc_instruction_type_t inst_type) {
    if (!monitoring_enabled || core_id >= num_cores) {
        return;
    }
    
    std::lock_guard<std::mutex> lock(data_mutex);
    
    core_data[core_id].instructions_retired++;
    
    // Check if this is a vector instruction
    if (inst_type >= KNC_INST_VPADDD && inst_type <= KNC_INST_VMINPS) {
        core_data[core_id].vector_instructions++;
    }
    
    aggregate_counters_val.instructions_retired++;
}

void KNCPerformanceMonitor::record_memory_access(uint32_t core_id, uint64_t address, size_t size, bool is_write) {
    if (!monitoring_enabled || core_id >= num_cores) {
        return;
    }
    
    std::lock_guard<std::mutex> lock(data_mutex);
    
    core_data[core_id].memory_accesses++;
    aggregate_counters_val.memory_accesses++;
    
    // Model cache behavior
    bool l1_hit = is_l1_hit(address, core_id);
    bool l2_hit = is_l2_hit(address, core_id);
    
    update_cache_stats(address, l1_hit, l2_hit, core_id);
}

void KNCPerformanceMonitor::record_cache_event(uint32_t core_id, bool is_l1_hit, bool is_l2_hit) {
    if (!monitoring_enabled || core_id >= num_cores) {
        return;
    }
    
    std::lock_guard<std::mutex> lock(data_mutex);
    
    if (is_l1_hit) {
        core_data[core_id].l1_hits++;
        aggregate_counters_val.l1_hits++;
    } else {
        core_data[core_id].l1_misses++;
        aggregate_counters_val.l1_misses++;
    }
    
    if (is_l2_hit) {
        core_data[core_id].l2_hits++;
        aggregate_counters_val.l2_hits++;
    } else {
        core_data[core_id].l2_misses++;
        aggregate_counters_val.l2_misses++;
    }
}

void KNCPerformanceMonitor::record_ring_bus_transaction(uint32_t core_id, uint32_t dest_tile, size_t size) {
    if (!monitoring_enabled || core_id >= num_cores) {
        return;
    }
    
    std::lock_guard<std::mutex> lock(data_mutex);
    
    core_data[core_id].ring_bus_transactions++;
    aggregate_counters_val.ring_bus_transactions++;
}

void KNCPerformanceMonitor::record_branch_event(uint32_t core_id, bool taken, bool mispredicted) {
    if (!monitoring_enabled || core_id >= num_cores) {
        return;
    }
    
    std::lock_guard<std::mutex> lock(data_mutex);
    
    core_data[core_id].branches_taken++;
    aggregate_counters_val.branches_taken++;
    
    if (mispredicted) {
        core_data[core_id].branches_mispredicted++;
        aggregate_counters_val.branches_mispredicted++;
    }
}

void KNCPerformanceMonitor::record_cycle(uint32_t core_id, uint64_t cycles) {
    if (!monitoring_enabled || core_id >= num_cores) {
        return;
    }
    
    std::lock_guard<std::mutex> lock(data_mutex);
    
    core_data[core_id].cycles += cycles;
    aggregate_counters_val.cycles += cycles;
}

bool KNCPerformanceMonitor::is_l1_hit(uint64_t address, uint32_t core_id) {
    // Simple L1 cache model - 32KB per core
    // Use simple hash function for cache line mapping
    uint64_t cache_line = address / 64;  // 64-byte cache lines
    uint64_t cache_index = cache_line % (KNC_L1_CACHE_SIZE / 64);
    
    // Simplified: 90% hit rate for random access
    return (cache_index % 10) != 0;
}

bool KNCPerformanceMonitor::is_l2_hit(uint64_t address, uint32_t core_id) {
    // Simple L2 cache model - 512KB shared
    uint64_t cache_line = address / 64;
    uint64_t cache_index = cache_line % (KNC_L2_CACHE_SIZE / 64);
    
    // Simplified: 80% hit rate for L2
    return (cache_index % 5) != 0;
}

void KNCPerformanceMonitor::update_cache_stats(uint64_t address, bool is_l1_hit, bool is_l2_hit, uint32_t core_id) {
    if (is_l1_hit) {
        core_data[core_id].l1_hits++;
        aggregate_counters_val.l1_hits++;
    } else {
        core_data[core_id].l1_misses++;
        aggregate_counters_val.l1_misses++;
        
        if (is_l2_hit) {
            core_data[core_id].l2_hits++;
            aggregate_counters_val.l2_hits++;
        } else {
            core_data[core_id].l2_misses++;
            aggregate_counters_val.l2_misses++;
        }
    }
}

const knc_core_perf_data_t& KNCPerformanceMonitor::get_core_data(uint32_t core_id) const {
    if (core_id < num_cores) {
        return core_data[core_id];
    }
    static knc_core_perf_data_t dummy_data;
    return dummy_data;
}

const knc_performance_counters_t& KNCPerformanceMonitor::get_aggregate_counters() const {
    return aggregate_counters_val;
}

uint64_t KNCPerformanceMonitor::get_counter_value(uint32_t core_id, knc_perf_event_type_t event_type) const {
    if (core_id >= num_cores) {
        return 0;
    }
    
    const auto& core = core_data[core_id];
    
    switch (event_type) {
        case KNC_PERF_INSTRUCTIONS_RETIRED:
            return core.instructions_retired;
        case KNC_PERF_VECTOR_INSTRUCTIONS:
            return core.vector_instructions;
        case KNC_PERF_MEMORY_ACCESSES:
            return core.memory_accesses;
        case KNC_PERF_L1_HITS:
            return core.l1_hits;
        case KNC_PERF_L1_MISSES:
            return core.l1_misses;
        case KNC_PERF_L2_HITS:
            return core.l2_hits;
        case KNC_PERF_L2_MISSES:
            return core.l2_misses;
        case KNC_PERF_RING_BUS_TRANSACTIONS:
            return core.ring_bus_transactions;
        case KNC_PERF_CYCLES:
            return core.cycles;
        case KNC_PERF_BRANCHES_TAKEN:
            return core.branches_taken;
        case KNC_PERF_BRANCHES_MISPREDICTED:
            return core.branches_mispredicted;
        default:
            return 0;
    }
}

void KNCPerformanceMonitor::print_performance_report() const {
    std::cout << "\n=== KNC Performance Report ===\n";
    print_aggregate_statistics();
    
    for (uint32_t i = 0; i < std::min(num_cores, 4U); i++) {
        print_core_statistics(i);
    }
    
    if (num_cores > 4) {
        std::cout << "... (" << (num_cores - 4) << " more cores)\n";
    }
}

void KNCPerformanceMonitor::print_core_statistics(uint32_t core_id) const {
    if (core_id >= num_cores) {
        return;
    }
    
    const auto& core = core_data[core_id];
    
    std::cout << "\n--- Core " << core_id << " ---\n";
    std::cout << "Instructions: " << core.instructions_retired << "\n";
    std::cout << "Vector instructions: " << core.vector_instructions << "\n";
    std::cout << "Memory accesses: " << core.memory_accesses << "\n";
    std::cout << "L1 hits/misses: " << core.l1_hits << "/" << core.l1_misses;
    
    if (core.l1_hits + core.l1_misses > 0) {
        double l1_hit_rate = (double)core.l1_hits / (core.l1_hits + core.l1_misses) * 100.0;
        std::cout << " (" << std::fixed << std::setprecision(1) << l1_hit_rate << "%)";
    }
    std::cout << "\n";
    
    std::cout << "L2 hits/misses: " << core.l2_hits << "/" << core.l2_misses;
    
    if (core.l2_hits + core.l2_misses > 0) {
        double l2_hit_rate = (double)core.l2_hits / (core.l2_hits + core.l2_misses) * 100.0;
        std::cout << " (" << std::fixed << std::setprecision(1) << l2_hit_rate << "%)";
    }
    std::cout << "\n";
    
    std::cout << "Ring bus transactions: " << core.ring_bus_transactions << "\n";
    std::cout << "Cycles: " << core.cycles << "\n";
    
    if (core.cycles > 0) {
        double ipc = (double)core.instructions_retired / core.cycles;
        std::cout << "IPC: " << std::fixed << std::setprecision(3) << ipc << "\n";
    }
}

void KNCPerformanceMonitor::print_aggregate_statistics() const {
    std::cout << "Aggregate Statistics:\n";
    std::cout << "Total instructions: " << aggregate_counters_val.instructions_retired << "\n";
    std::cout << "Vector instructions: " << aggregate_counters_val.vector_instructions << "\n";
    std::cout << "Memory accesses: " << aggregate_counters_val.memory_accesses << "\n";
    std::cout << "Ring bus transactions: " << aggregate_counters_val.ring_bus_transactions << "\n";
    std::cout << "Total cycles: " << aggregate_counters_val.cycles << "\n";
    
    if (aggregate_counters_val.cycles > 0) {
        double avg_ipc = (double)aggregate_counters_val.instructions_retired / aggregate_counters_val.cycles;
        std::cout << "Average IPC: " << std::fixed << std::setprecision(3) << avg_ipc << "\n";
    }
    
    if (aggregate_counters_val.l1_hits + aggregate_counters_val.l1_misses > 0) {
        double l1_hit_rate = (double)aggregate_counters_val.l1_hits / 
                           (aggregate_counters_val.l1_hits + aggregate_counters_val.l1_misses) * 100.0;
        std::cout << "L1 hit rate: " << std::fixed << std::setprecision(1) << l1_hit_rate << "%\n";
    }
    
    if (aggregate_counters_val.l2_hits + aggregate_counters_val.l2_misses > 0) {
        double l2_hit_rate = (double)aggregate_counters_val.l2_hits / 
                           (aggregate_counters_val.l2_hits + aggregate_counters_val.l2_misses) * 100.0;
        std::cout << "L2 hit rate: " << std::fixed << std::setprecision(1) << l2_hit_rate << "%\n";
    }
}

void KNCPerformanceMonitor::export_csv(const std::string& filename) const {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open file " << filename << " for writing\n";
        return;
    }
    
    // Write header
    file << "core_id,instructions_retired,vector_instructions,memory_accesses,";
    file << "l1_hits,l1_misses,l2_hits,l2_misses,";
    file << "ring_bus_transactions,cycles,ipc\n";
    
    // Write data for each core
    for (uint32_t i = 0; i < num_cores; i++) {
        const auto& core = core_data[i];
        file << i << ",";
        file << core.instructions_retired << ",";
        file << core.vector_instructions << ",";
        file << core.memory_accesses << ",";
        file << core.l1_hits << ",";
        file << core.l1_misses << ",";
        file << core.l2_hits << ",";
        file << core.l2_misses << ",";
        file << core.ring_bus_transactions << ",";
        file << core.cycles << ",";
        
        double ipc = (core.cycles > 0) ? (double)core.instructions_retired / core.cycles : 0.0;
        file << std::fixed << std::setprecision(3) << ipc;
        file << "\n";
    }
    
    file.close();
    std::cout << "Performance data exported to " << filename << "\n";
}

bool KNCPerformanceMonitor::initialize_pcm() {
    // Placeholder for Intel PCM initialization
    // In practice, this would initialize the Intel PCM library
    pcm_handle = reinterpret_cast<void*>(0x12345678);  // Dummy handle
    return true;
}

void KNCPerformanceMonitor::cleanup_pcm() {
    pcm_handle = nullptr;
}

void KNCPerformanceMonitor::start_collection() {
    collection_active.store(true);
    start_time = std::chrono::high_resolution_clock::now();
    last_update = start_time;
}

void KNCPerformanceMonitor::stop_collection() {
    collection_active.store(false);
}

void KNCPerformanceMonitor::reset_collection() {
    reset_counters();
    start_time = std::chrono::high_resolution_clock::now();
    last_update = start_time;
}
