#ifndef KNC_PERFORMANCE_MONITOR_H
#define KNC_PERFORMANCE_MONITOR_H

#include <vector>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <string>
#include <functional>
#include "knc_types.h"

// Performance event types
typedef enum {
    KNC_PERF_INSTRUCTIONS_RETIRED = 0,
    KNC_PERF_VECTOR_INSTRUCTIONS = 1,
    KNC_PERF_MEMORY_ACCESSES = 2,
    KNC_PERF_L1_HITS = 3,
    KNC_PERF_L1_MISSES = 4,
    KNC_PERF_L2_HITS = 5,
    KNC_PERF_L2_MISSES = 6,
    KNC_PERF_RING_BUS_TRANSACTIONS = 7,
    KNC_PERF_CYCLES = 8,
    KNC_PERF_BRANCHES_TAKEN = 9,
    KNC_PERF_BRANCHES_MISPREDICTED = 10,
    KNC_PERF_CACHE_REFERENCES = 11,
    KNC_PERF_TLB_HITS = 12,
    KNC_PERF_TLB_MISSES = 13
} knc_perf_event_type_t;

// Performance counter configuration
typedef struct {
    knc_perf_event_type_t event_type;
    bool enabled;
    uint32_t core_mask;  // Bit mask of cores to monitor
    uint64_t threshold;   // Threshold for overflow detection
    bool overflow_enabled;
} knc_perf_counter_config_t;

// Core-specific performance data
typedef struct {
    uint64_t instructions_retired;
    uint64_t vector_instructions;
    uint64_t memory_accesses;
    uint64_t l1_hits;
    uint64_t l1_misses;
    uint64_t l2_hits;
    uint64_t l2_misses;
    uint64_t ring_bus_transactions;
    uint64_t cycles;
    uint64_t branches_taken;
    uint64_t branches_mispredicted;
    uint64_t cache_references;
    uint64_t tlb_hits;
    uint64_t tlb_misses;
    uint64_t last_update_time;
} knc_core_perf_data_t;

class KNCPerformanceMonitor {
private:
    // Configuration
    std::vector<knc_perf_counter_config_t> counter_configs;
    uint32_t num_cores;
    bool monitoring_enabled;
    
    // Performance data
    std::vector<knc_core_perf_data_t> core_data;
    knc_performance_counters_t aggregate_counters_val;
    
    // Synchronization
    std::mutex data_mutex;
    std::atomic<bool> collection_active;
    
    // Timing
    std::chrono::high_resolution_clock::time_point start_time;
    std::chrono::high_resolution_clock::time_point last_update;
    
    // PCM integration
    void* pcm_handle;  // Handle to Intel PCM library
    
    // Performance counter management
    void initialize_counters();
    void reset_counters();
    void update_core_counters(uint32_t core_id, knc_perf_event_type_t event_type, uint64_t count);
    void aggregate_counters_func();
    
    // PCM-specific functions
    bool initialize_pcm();
    void cleanup_pcm();
    uint64_t read_pcm_counter(uint32_t core_id, knc_perf_event_type_t event_type);
    
    // KNC-specific performance modeling
    uint64_t model_instruction_latency(knc_instruction_type_t inst_type);
    uint64_t model_memory_latency(uint64_t address, size_t size, bool is_write);
    uint64_t model_ring_bus_latency(uint32_t source_tile, uint32_t dest_tile, size_t size);
    
    // Cache modeling
    bool is_l1_hit(uint64_t address, uint32_t core_id);
    bool is_l2_hit(uint64_t address, uint32_t core_id);
    void update_cache_stats(uint64_t address, bool is_l1_hit, bool is_l2_hit, uint32_t core_id);
    
    // Statistics calculation
    void calculate_derived_metrics();
    double calculate_ipc(uint32_t core_id);
    double calculate_cache_hit_ratio(uint32_t core_id, bool l1_cache);
    double calculate_bandwidth_utilization();
    
public:
    KNCPerformanceMonitor(uint32_t cores = KNC_NUM_CORES);
    ~KNCPerformanceMonitor();
    
    // Initialization
    bool initialize();
    void shutdown();
    
    // Configuration
    void enable_monitoring(bool enable);
    bool is_monitoring_enabled() const;
    void configure_counter(knc_perf_event_type_t event_type, bool enabled, 
                       uint32_t core_mask = 0xFFFFFFFF);
    void set_overflow_threshold(knc_perf_event_type_t event_type, uint64_t threshold);
    
    // Event recording
    void record_instruction(uint32_t core_id, knc_instruction_type_t inst_type);
    void record_memory_access(uint32_t core_id, uint64_t address, size_t size, bool is_write);
    void record_cache_event(uint32_t core_id, bool is_l1_hit, bool is_l2_hit);
    void record_ring_bus_transaction(uint32_t core_id, uint32_t dest_tile, size_t size);
    void record_branch_event(uint32_t core_id, bool taken, bool mispredicted);
    void record_cycle(uint32_t core_id, uint64_t cycles);
    
    // Data retrieval
    const knc_core_perf_data_t& get_core_data(uint32_t core_id) const;
    const knc_performance_counters_t& get_aggregate_counters() const;
    uint64_t get_counter_value(uint32_t core_id, knc_perf_event_type_t event_type) const;
    
    // Statistics and reporting
    void print_performance_report() const;
    void print_core_statistics(uint32_t core_id) const;
    void print_aggregate_statistics() const;
    void export_csv(const std::string& filename) const;
    
    // Real-time monitoring
    void start_collection();
    void stop_collection();
    void reset_collection();
    
    // Performance analysis
    void analyze_performance_bottlenecks();
    void generate_performance_report(const std::string& filename);
    std::vector<std::string> get_performance_recommendations();
    
    // Integration with Intel PCM
    bool enable_pcm_monitoring(bool enable);
    bool is_pcm_available() const;
    void sync_with_pcm();
    
    // Threshold monitoring
    bool check_thresholds();
    void set_threshold_callback(knc_perf_event_type_t event_type, 
                             std::function<void(uint32_t, uint64_t)> callback);
    
    // Debugging
    void dump_counter_state() const;
    void print_configuration() const;
};

#endif // KNC_PERFORMANCE_MONITOR_H
