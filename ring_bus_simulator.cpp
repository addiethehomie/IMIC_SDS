#include "ring_bus_simulator.h"
#include <iostream>
#include <algorithm>
#include <cstring>

// Windows compatibility
#ifdef _WIN32
#include <windows.h>
#define usleep(x) Sleep((x) / 1000)
#else
#include <unistd.h>
#endif

// DTD helper functions
bool RingBusSimulator::is_memory_request(const void* data, uint32_t size) {
    // Simple heuristic: check if data looks like a memory request
    // In practice, this would be based on message type/protocol
    return size >= 8;  // Minimum size for address + data
}

uint64_t RingBusSimulator::extract_memory_address(const void* data, uint32_t size) {
    if (size >= 8) {
        const uint64_t* addr_ptr = static_cast<const uint64_t*>(data);
        return *addr_ptr;  // First 8 bytes as address
    }
    return 0;
}

uint32_t RingBusSimulator::get_dtd_home_node(uint64_t address) {
    return address % config.num_nodes;  // Global hash for DTD home node
}

uint32_t RingBusSimulator::get_mmu_home_node(uint64_t address) {
    // Architecture-aware MMU placement
    if (config.architecture == ARCH_KNL) {
        return address % KNL_NUM_MMUS;  // KNL uses 38 MMUs
    } else {
        return address % KNC_NUM_MMUS;  // KNC uses 8 MMUs symmetrically
    }
}

dtd_cache_line_t* RingBusSimulator::find_cache_line(uint64_t address) {
    uint64_t cache_line_addr = address & ~(cache_line_size - 1);  // 64-byte align
    uint32_t home_tile = get_dtd_home_node(address);
    
    if (home_tile >= dtd_tiles.size()) {
        return nullptr;
    }
    
    dtd_tile_state_t& tile_state = dtd_tiles[home_tile];
    
    // Search in directory (simple linear search - could be optimized with hash)
    for (auto& cache_line : tile_state.cache_directory) {
        if (cache_line.cache_line_address == cache_line_addr) {
            return &cache_line;
        }
    }
    
    return nullptr;  // Cache miss
}

bool RingBusSimulator::dtd_check_coherency(uint64_t address, uint32_t requesting_tile) {
    dtd_cache_line_t* cache_line = find_cache_line(address);
    if (!cache_line) {
        return true;  // Cache miss - no coherency issue
    }
    
    // Check if requesting tile already has access
    if (cache_line->owner_tile == requesting_tile) {
        return true;  // Already owner
    }
    
    // Need to invalidate or request ownership
    if (cache_line->is_modified) {
        // Write-back required - extra latency
        return false;
    }
    
    // Can share or transition to exclusive
    return true;
}

void RingBusSimulator::dtd_invalidate_cacheline(uint64_t address, uint32_t requesting_tile) {
    dtd_cache_line_t* cache_line = find_cache_line(address);
    if (!cache_line) {
        return;
    }
    
    // Send invalidation to current sharers
    uint32_t sharer_mask = cache_line->sharer_mask & ~(1 << requesting_tile);
    
    // Update state
    cache_line->sharer_mask = sharer_mask;
    cache_line->owner_tile = requesting_tile;
    cache_line->timestamp = simulation_time.load();
    
    // Track invalidation statistics
    uint32_t home_tile = get_dtd_home_node(address);
    if (home_tile < dtd_tiles.size()) {
        dtd_tiles[home_tile].invalidation_requests++;
    }
}

void RingBusSimulator::dtd_update_ownership(uint64_t address, uint32_t owner_tile, bool is_modified) {
    dtd_cache_line_t* cache_line = find_cache_line(address);
    if (!cache_line) {
        // Create new cache line entry
        dtd_cache_line_t new_line;
        new_line.cache_line_address = address & ~(cache_line_size - 1);
        new_line.owner_tile = owner_tile;
        new_line.sharer_mask = 1 << owner_tile;
        new_line.is_modified = is_modified;
        new_line.is_exclusive = true;
        new_line.is_shared = false;
        new_line.timestamp = simulation_time.load();
        
        uint32_t home_tile = get_dtd_home_node(address);
        if (home_tile < dtd_tiles.size()) {
            dtd_tiles[home_tile].cache_directory.push_back(new_line);
        }
        return;
    }
    
    // Update existing cache line
    cache_line->owner_tile = owner_tile;
    cache_line->sharer_mask = 1 << owner_tile;
    cache_line->is_modified = is_modified;
    cache_line->is_exclusive = is_modified;
    cache_line->is_shared = !is_modified;
    cache_line->timestamp = simulation_time.load();
}

uint32_t RingBusSimulator::calculate_dtd_latency(uint32_t source_tile, uint32_t dest_tile, uint64_t address) {
    uint32_t base_latency = calculate_distance(source_tile, dest_tile);
    
    if (!dtd_enabled) {
        return base_latency;
    }
    
    // Add DTD check latency (1-2 extra hops for cache coherency)
    dtd_cache_line_t* cache_line = find_cache_line(address);
    if (!cache_line) {
        return base_latency + 2;  // Cache miss + directory lookup
    }
    
    if (cache_line->is_modified && cache_line->owner_tile != source_tile) {
        return base_latency + 3;  // Write-back required
    }
    
    if (cache_line->sharer_mask != (1 << source_tile)) {
        return base_latency + 1;  // Share invalidate required
    }
    
    return base_latency;  // Cache hit with no coherency actions
}

RingBusSimulator::RingBusSimulator(uint32_t num_nodes, knc_architecture_t arch) {
    config.num_nodes = num_nodes;
    config.architecture = arch;
    
    // Configure based on architecture
    if (arch == ARCH_KNL) {
        config.num_rings = 2;
        config.bandwidth_mbps = 213312;  // KNL dual ring: 2 × 106.656 GB/s
        config.latency_cycles = 2;       // 2-cycle latency
    } else {
        config.num_rings = 1;
        config.bandwidth_mbps = 134784;  // KNC single ring: 134.784 GB/s
        config.latency_cycles = 2;       // 2-cycle latency
    }
    config.buffer_size = 1024;        // 1KB buffer per node
    config.enable_contention = true;
    config.enable_latency_modeling = true;
    
    simulation_time.store(0);
    total_messages.store(0);
    total_bytes.store(0);
    total_latency.store(0);
    max_contention.store(0);
    running.store(false);
    
    // Initialize DTD (Distributed Tag Directory) state
    dtd_enabled = true;
    cache_line_size = 64;  // KNC uses 64-byte cache lines
    associativity = 8;      // 8-way set associative
    
    dtd_home_nodes.resize(num_nodes);
    dtd_tiles.resize(num_nodes);
    
    for (uint32_t i = 0; i < num_nodes; i++) {
        dtd_home_nodes[i] = i;  // Each tile has a DTD home node
        
        // Initialize DTD tile state
        dtd_tiles[i].tile_id = i;
        dtd_tiles[i].directory_size = 1024;  // Directory entries per tile
        dtd_tiles[i].cache_directory.reserve(1024);
        dtd_tiles[i].snoop_requests = 0;
        dtd_tiles[i].invalidation_requests = 0;
        dtd_tiles[i].cache_misses = 0;
        dtd_tiles[i].cache_hits = 0;
    }
}

RingBusSimulator::~RingBusSimulator() {
    shutdown();
}

bool RingBusSimulator::initialize() {
    std::lock_guard<std::mutex> lock(network_mutex);
    
    // Initialize nodes
    nodes.resize(config.num_nodes);
    for (uint32_t i = 0; i < config.num_nodes; i++) {
        nodes[i].node_id = i;
        nodes[i].buffer_occupancy = 0;
        nodes[i].last_activity_time = 0;
        nodes[i].messages_sent = 0;
        nodes[i].messages_received = 0;
        nodes[i].bytes_transmitted = 0;
        nodes[i].contention_cycles = 0;
    }
    
    // Build ring topology
    build_ring_topology();
    
    std::cout << "Ring bus simulator initialized with " << config.num_nodes << " nodes\n";
    std::cout << "Bandwidth: " << config.bandwidth_mbps << " MB/s\n";
    std::cout << "Latency: " << config.latency_cycles << " cycles\n";
    
    return true;
}

void RingBusSimulator::build_ring_topology() {
    routing_table.resize(config.num_nodes);
    
    for (uint32_t i = 0; i < config.num_nodes; i++) {
        routing_table[i].resize(config.num_nodes);
        
        for (uint32_t j = 0; j < config.num_nodes; j++) {
            if (i == j) {
                routing_table[i][j] = i;  // Self
            } else {
                // Ring topology: next hop in ring direction
                routing_table[i][j] = (i + 1) % config.num_nodes;
            }
        }
    }
    
    // Optimize for bidirectional ring
    for (uint32_t i = 0; i < config.num_nodes; i++) {
        for (uint32_t j = 0; j < config.num_nodes; j++) {
            if (i != j) {
                uint32_t clockwise_dist = (j - i + config.num_nodes) % config.num_nodes;
                uint32_t counter_clockwise_dist = (i - j + config.num_nodes) % config.num_nodes;
                
                if (counter_clockwise_dist < clockwise_dist) {
                    // Go counter-clockwise
                    routing_table[i][j] = (i - 1 + config.num_nodes) % config.num_nodes;
                }
            }
        }
    }
}

uint32_t RingBusSimulator::get_next_hop(uint32_t source, uint32_t dest) {
    if (source < config.num_nodes && dest < config.num_nodes) {
        return routing_table[source][dest];
    }
    return source;  // Invalid destination
}

uint32_t RingBusSimulator::calculate_distance(uint32_t node1, uint32_t node2) {
    // Shortest Distance Algorithm (SDA) for ring topology
    // distance = min(abs(dest - src), num_nodes - abs(dest - src))
    int32_t direct_distance = std::abs((int32_t)node2 - (int32_t)node1);
    int32_t wrap_distance = config.num_nodes - direct_distance;
    return std::min(direct_distance, wrap_distance);
}

bool RingBusSimulator::send_message(uint32_t source_node, uint32_t dest_node,
                                  const void* data, uint32_t size, uint32_t priority) {
    if (source_node >= config.num_nodes || dest_node >= config.num_nodes) {
        return false;
    }
    
    std::lock_guard<std::mutex> lock(network_mutex);
    
    // Check buffer space
    auto& source = nodes[source_node];
    if (source.buffer_occupancy + size > config.buffer_size) {
        return false;  // Buffer full
    }
    
    uint32_t actual_dest = dest_node;
    uint32_t dtd_latency = 0;
    
    // Enhanced DTD check for cache coherency
    if (dtd_enabled && is_memory_request(data, size)) {
        uint64_t address = extract_memory_address(data, size);
        uint32_t mmu_home = get_mmu_home_node(address);  // Modulo 8 hash
        uint32_t dtd_home = get_dtd_home_node(address);
        
        // Check cache coherency state
        bool coherency_ok = dtd_check_coherency(address, source_node);
        
        if (!coherency_ok) {
            // Need to invalidate cache line - extra latency
            dtd_invalidate_cacheline(address, source_node);
            
            // Route to DTD home node first for cache coherency
            actual_dest = dtd_home;
            
            // Update statistics
            if (dtd_home < dtd_tiles.size()) {
                dtd_tiles[dtd_home].snoop_requests++;
            }
        } else {
            // Cache hit or clean line - can route directly
            dtd_update_ownership(address, source_node, (size > 8));  // Assume write if > 8 bytes
        }
        
        // Calculate enhanced latency with DTD and MMU considerations
        uint32_t base_distance = calculate_distance(source_node, actual_dest);
        dtd_latency = calculate_dtd_latency(source_node, actual_dest, address);
    }
    
    // Create message
    ring_bus_message_t message;
    message.source_node = source_node;
    message.dest_node = actual_dest;
    message.priority = priority;
    message.size = size;
    message.timestamp = simulation_time.load();
    
    // Calculate delivery time
    uint32_t distance = calculate_distance(source_node, actual_dest);
    uint32_t base_latency = distance * config.latency_cycles;
    message.delivery_time = message.timestamp + base_latency + dtd_latency;
    
    // Copy data
    message.data = new uint8_t[size];
    memcpy(message.data, data, size);
    
    // Add to outbound queue
    source.outbound_queue.push(message);
    source.buffer_occupancy += size;
    source.last_activity_time = simulation_time.load();
    
    // Update statistics
    nodes[source_node].last_activity_time = simulation_time.load();
    
    total_messages++;
    total_bytes += size;
    
    return true;
}

bool RingBusSimulator::receive_message(uint32_t node_id, ring_bus_message_t& message) {
    if (node_id >= config.num_nodes) {
        return false;
    }
    
    std::lock_guard<std::mutex> lock(network_mutex);
    
    if (nodes[node_id].inbound_queue.empty()) {
        return false;
    }
    
    message = nodes[node_id].inbound_queue.front();
    
    // Check if message has arrived
    if (simulation_time.load() >= message.delivery_time) {
        nodes[node_id].inbound_queue.pop();
        nodes[node_id].messages_received++;
        nodes[node_id].last_activity_time = simulation_time.load();
        
        // Update performance stats
        uint32_t actual_latency = simulation_time.load() - message.timestamp;
        update_performance_stats(message, actual_latency);
        
        // Clean up message data
        delete[] message.data;
        
        return true;
    }
    
    return false;
}

uint32_t RingBusSimulator::calculate_contention_delay(uint32_t source, uint32_t dest) {
    // Simple contention model based on recent activity
    uint32_t delay = 0;
    
    for (uint32_t i = 0; i < config.num_nodes; i++) {
        if (i != source && i != dest) {
            uint64_t time_since_activity = simulation_time.load() - nodes[i].last_activity_time;
            if (time_since_activity < 10) {  // Recent activity
                delay += 1;
            }
        }
    }
    
    return delay;
}

void RingBusSimulator::update_performance_stats(const ring_bus_message_t& message, uint32_t latency) {
    total_latency += latency;
    
    if (latency > max_contention.load()) {
        max_contention.store(latency);
    }
}

void RingBusSimulator::start_simulation() {
    if (running.load()) {
        return;  // Already running
    }
    
    running.store(true);
    simulation_thread = std::thread(&RingBusSimulator::simulation_loop, this);
    
    std::cout << "Ring bus simulation started\n";
}

void RingBusSimulator::stop_simulation() {
    if (!running.load()) {
        return;  // Already stopped
    }
    
    running.store(false);
    if (simulation_thread.joinable()) {
        simulation_thread.join();
    }
    
    std::cout << "Ring bus simulation stopped\n";
}

void RingBusSimulator::simulation_loop() {
    while (running.load()) {
        advance_simulation();
        process_pending_messages();
        
        // Small delay to prevent busy waiting
        std::this_thread::sleep_for(std::chrono::microseconds(100));
    }
}

void RingBusSimulator::advance_simulation() {
    simulation_time.fetch_add(1);
}

void RingBusSimulator::process_pending_messages() {
    std::lock_guard<std::mutex> lock(network_mutex);
    
    // Process each node's outbound queue
    for (uint32_t i = 0; i < config.num_nodes; i++) {
        process_node_queue(i);
    }
}

void RingBusSimulator::process_node_queue(uint32_t node_id) {
    auto& node = nodes[node_id];
    
    while (!node.outbound_queue.empty()) {
        auto& message = node.outbound_queue.front();
        
        // Route message through network
        if (route_message(message)) {
            node.outbound_queue.pop();
            node.buffer_occupancy -= message.size;
            node.messages_sent++;
            node.bytes_transmitted += message.size;
        } else {
            break;  // Cannot route more messages right now
        }
    }
}

bool RingBusSimulator::route_message(const ring_bus_message_t& message) {
    uint32_t current_node = message.source_node;
    uint32_t next_hop = get_next_hop(current_node, message.dest_node);
    
    // Route to next hop
    if (next_hop != message.dest_node) {
        // Multi-hop routing - create a new message for the next hop
        uint32_t source_tile = current_node / KNC_CORES_PER_TILE;
        uint32_t dest_tile = next_hop / KNC_CORES_PER_TILE;
        
        return send_message(source_tile, dest_tile, message.data, message.size, message.priority);
    } else {
        // Final destination reached
        return true;
    }
}

bool RingBusSimulator::simulate_tile_communication(uint32_t source_tile, uint32_t dest_tile,
                                               const void* data, uint32_t size) {
    return send_message(source_tile, dest_tile, data, size, 0);
}

bool RingBusSimulator::simulate_broadcast(uint32_t source_tile, const void* data, uint32_t size) {
    bool success = true;
    
    for (uint32_t i = 0; i < config.num_nodes; i++) {
        if (i != source_tile) {
            success &= send_message(source_tile, i, data, size, 1);  // High priority for broadcast
        }
    }
    
    return success;
}

bool RingBusSimulator::is_running() const {
    return running.load();
}

uint64_t RingBusSimulator::get_simulation_time() const {
    return simulation_time.load();
}

void RingBusSimulator::print_performance_stats() const {
    uint64_t total_msgs = total_messages.load();
    uint64_t total_bytes_val = total_bytes.load();
    uint64_t total_latency_val = total_latency.load();
    uint64_t max_contention_val = max_contention.load();
    
    std::cout << "\n=== Ring Bus Performance Statistics ===\n";
    std::cout << "Total messages: " << total_msgs << "\n";
    std::cout << "Total bytes: " << total_bytes_val << "\n";
    
    if (total_msgs > 0) {
        std::cout << "Average message size: " << (total_bytes_val / total_msgs) << " bytes\n";
        std::cout << "Average latency: " << (total_latency_val / total_msgs) << " cycles\n";
    }
    
    std::cout << "Maximum contention delay: " << max_contention_val << " cycles\n";
    std::cout << "Simulation time: " << simulation_time.load() << " cycles\n";
}

void RingBusSimulator::dump_network_state() const {
    std::cout << "=== Ring Bus Network State ===\n";
    std::cout << "Nodes: " << config.num_nodes << "\n";
    std::cout << "Bandwidth: " << config.bandwidth_mbps << " MB/s\n";
    std::cout << "Latency: " << config.latency_cycles << " cycles\n";
    std::cout << "DTD Enabled: " << (dtd_enabled ? "Yes" : "No") << "\n";
    
    std::cout << "\nNode States:\n";
    for (uint32_t i = 0; i < config.num_nodes; i++) {
        const auto& node = nodes[i];
        std::cout << "Node " << i << ":\n";
        std::cout << "  Inbound queue: " << node.inbound_queue.size() << " messages\n";
        std::cout << "  Outbound queue: " << node.outbound_queue.size() << " messages\n";
        std::cout << "  Buffer occupancy: " << node.buffer_occupancy << " bytes\n";
    }
}

void RingBusSimulator::shutdown() {
    std::cout << "Shutting down Ring Bus Simulator\n";
    running = false;
}

void RingBusSimulator::step_simulation() {
    advance_simulation();
    process_pending_messages();
}

void RingBusSimulator::reset_simulation() {
    std::lock_guard<std::mutex> lock(network_mutex);
    
    // Reset simulation time
    simulation_time.store(0);
    
    // Reset all nodes
    for (auto& node : nodes) {
        while (!node.inbound_queue.empty()) {
            node.inbound_queue.pop();
        }
        while (!node.outbound_queue.empty()) {
            node.outbound_queue.pop();
        }
        node.buffer_occupancy = 0;
        node.last_activity_time = 0;
        node.messages_sent = 0;
        node.messages_received = 0;
        node.bytes_transmitted = 0;
        node.contention_cycles = 0;
    }
    
    // Reset performance counters
    total_messages.store(0);
    total_bytes.store(0);
    total_latency.store(0);
    max_contention.store(0);
}

uint32_t RingBusSimulator::calculate_distance_public(uint32_t node1, uint32_t node2) {
    return calculate_distance(node1, node2);
}

void RingBusSimulator::get_performance_stats(uint64_t& total_msgs, uint64_t& total_bytes_val, 
                                           uint64_t& avg_latency, uint64_t& max_contention_cycles) const {
    total_msgs = total_messages.load();
    total_bytes_val = total_bytes.load();
    
    if (total_msgs > 0) {
        avg_latency = total_latency.load() / total_msgs;
    } else {
        avg_latency = 0;
    }
    
    max_contention_cycles = max_contention.load();
}
