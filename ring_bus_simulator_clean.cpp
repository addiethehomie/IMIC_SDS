#include "ring_bus_simulator.h"
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <thread>
#include <chrono>

// DTD helper functions
bool RingBusSimulator::is_memory_request(const void* data, uint32_t size) {
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
    // Modulo 8 hash for MMU placement - KNC uses 8 MMUs symmetrically
    return address % KNC_NUM_MMUS;
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

// Keep all the existing implementations from the original file
// ... (rest of the functions would be copied here)
