#ifndef RING_BUS_SIMULATOR_H
#define RING_BUS_SIMULATOR_H

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>

#include "knc_types.h"

// Ring bus configuration
typedef struct {
    uint32_t num_nodes;
    uint32_t bandwidth_mbps;
    uint32_t latency_cycles;
    uint32_t buffer_size;
    bool enable_contention;
    bool enable_latency_modeling;
    knc_architecture_t architecture;
    uint32_t num_rings;  // 1 for KNC, 2 for KNL
} ring_bus_config_t;

// Ring bus message with priority
typedef struct {
    uint32_t source_node;
    uint32_t dest_node;
    uint32_t priority;
    uint8_t* data;
    uint32_t size;
    uint64_t timestamp;
    uint64_t delivery_time;
} ring_bus_message_t;

// Ring bus node state
typedef struct {
    uint32_t node_id;
    std::queue<ring_bus_message_t> inbound_queue;
    std::queue<ring_bus_message_t> outbound_queue;
    uint32_t buffer_occupancy;
    uint64_t last_activity_time;
    uint64_t messages_sent;
    uint64_t messages_received;
    uint64_t bytes_transmitted;
    uint64_t contention_cycles;
} ring_bus_node_t;

// DTD (Distributed Tag Directory) entry for cache line tracking
typedef struct {
    uint64_t cache_line_address;  // Cache line address (64-byte aligned)
    uint32_t owner_tile;         // Current owner tile
    uint32_t sharer_mask;        // Bitmask of tiles sharing this line
    bool is_modified;            // Line is dirty (M state)
    bool is_exclusive;          // Line is exclusive (E state)
    bool is_shared;             // Line is shared (S state)
    uint64_t timestamp;         // Last access timestamp
} dtd_cache_line_t;

// DTD state per tile
typedef struct {
    uint32_t tile_id;
    std::vector<dtd_cache_line_t> cache_directory;
    uint64_t directory_size;
    uint32_t snoop_requests;
    uint32_t invalidation_requests;
    uint32_t cache_misses;
    uint32_t cache_hits;
} dtd_tile_state_t;

class RingBusSimulator {
private:
    // Configuration
    ring_bus_config_t config;
    
    // Network topology
    std::vector<ring_bus_node_t> nodes;
    std::vector<std::vector<uint32_t>> routing_table;
    
    // DTD (Distributed Tag Directory) state
    bool dtd_enabled;
    std::vector<uint32_t> dtd_home_nodes;
    std::vector<dtd_tile_state_t> dtd_tiles;
    uint32_t cache_line_size;  // 64 bytes for KNC
    uint32_t associativity;    // 8-way set associative
    
    // Simulation state
    std::atomic<bool> running;
    std::atomic<uint64_t> simulation_time;
    std::thread simulation_thread;
    
    // Synchronization
    std::mutex network_mutex;
    std::condition_variable network_cv;
    
    // Performance tracking
    std::atomic<uint64_t> total_messages;
    std::atomic<uint64_t> total_bytes;
    std::atomic<uint64_t> total_latency;
    std::atomic<uint64_t> max_contention;
    
    // Ring topology functions
    void build_ring_topology();
    uint32_t get_next_hop(uint32_t source, uint32_t dest);
    uint32_t calculate_distance(uint32_t node1, uint32_t node2);
    std::vector<uint32_t> find_shortest_path(uint32_t source, uint32_t dest);
    
    // DTD (Distributed Tag Directory) functions
    bool is_memory_request(const void* data, uint32_t size);
    uint64_t extract_memory_address(const void* data, uint32_t size);
    uint32_t get_dtd_home_node(uint64_t address);
    uint32_t get_mmu_home_node(uint64_t address);  // Modulo 8 hash for MMU
    dtd_cache_line_t* find_cache_line(uint64_t address);
    bool dtd_check_coherency(uint64_t address, uint32_t requesting_tile);
    void dtd_invalidate_cacheline(uint64_t address, uint32_t requesting_tile);
    void dtd_update_ownership(uint64_t address, uint32_t owner_tile, bool is_modified);
    uint32_t calculate_dtd_latency(uint32_t source_tile, uint32_t dest_tile, uint64_t address);
    
    // Message routing
    bool route_message(const ring_bus_message_t& message);
    void deliver_message(uint32_t node_id, const ring_bus_message_t& message);
    void process_node_queue(uint32_t node_id);
    
    // Contention modeling
    uint32_t calculate_contention_delay(uint32_t source, uint32_t dest);
    void update_contention_stats(uint32_t node_id, uint32_t delay);
    
    // Simulation loop
    void simulation_loop();
    void advance_simulation();
    void process_pending_messages();
    
    // Performance monitoring
    void update_performance_stats(const ring_bus_message_t& message, uint32_t latency);
    
public:
    RingBusSimulator(uint32_t num_nodes = KNC_NUM_TILES, knc_architecture_t arch = ARCH_KNC);
    ~RingBusSimulator();
    
    // Initialization
    bool initialize();
    void shutdown();
    
    // Configuration
    void set_bandwidth(uint32_t bandwidth_mbps);
    void set_latency(uint32_t latency_cycles);
    void set_buffer_size(uint32_t buffer_size);
    void enable_contention_modeling(bool enable);
    void enable_latency_modeling(bool enable);
    
    // Message passing interface
    bool send_message(uint32_t source_node, uint32_t dest_node, 
                    const void* data, uint32_t size, uint32_t priority = 0);
    bool receive_message(uint32_t node_id, ring_bus_message_t& message);
    bool has_pending_messages(uint32_t node_id);
    
    // Simulation control
    void start_simulation();
    void stop_simulation();
    void step_simulation();
    void reset_simulation();
    
    // State queries
    bool is_running() const;
    uint64_t get_simulation_time() const;
    const ring_bus_node_t& get_node_state(uint32_t node_id) const;
    
    // Public interface for testing
    uint32_t calculate_distance_public(uint32_t node1, uint32_t node2);
    
    // Performance statistics
    void get_performance_stats(uint64_t& total_msgs, uint64_t& total_bytes, 
                           uint64_t& avg_latency, uint64_t& max_contention_cycles) const;
    void print_performance_stats() const;
    
    // KNC-specific interface
    bool simulate_tile_communication(uint32_t source_tile, uint32_t dest_tile,
                                const void* data, uint32_t size);
    bool simulate_broadcast(uint32_t source_tile, const void* data, uint32_t size);
    bool simulate_reduce_operation(uint32_t tile_group, const void* data, uint32_t size);
    
    // Debugging
    void dump_network_state() const;
    void dump_routing_table() const;
    void print_message_flow(uint32_t source, uint32_t dest) const;
};

#endif // RING_BUS_SIMULATOR_H
