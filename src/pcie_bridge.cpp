#include "pcie_bridge.h"
#include <iostream>
#include <cstring>
#include <thread>

PCIeBridge::PCIeBridge() {
    bytes_transferred_host_to_device.store(0);
    bytes_transferred_device_to_host.store(0);
    total_transfers.store(0);
    total_latency_ns.store(0);
    cache_coherency_overhead_cycles.store(0);
    dtd_invalidation_count.store(0);
}

PCIeBridge::~PCIeBridge() {
    // Cleanup
}

double PCIeBridge::calculate_transfer_time_ns(size_t bytes) const {
    // PCIe 2.0 x16: 8GB/s per direction
    // Time = bytes / (8GB/s) * 1e9 nanoseconds
    double bytes_per_second = PCIE_BANDWIDTH_GBPS * BYTES_PER_GB;
    return (static_cast<double>(bytes) / bytes_per_second) * 1e9;
}

uint64_t PCIeBridge::calculate_cycle_delay(size_t bytes) const {
    // Calculate transfer time in nanoseconds
    double transfer_time_ns = calculate_transfer_time_ns(bytes);
    
    // Add base PCIe latency (~100ns)
    transfer_time_ns += PCIE_LATENCY_NS;
    
    // Convert to cycles (using KNC 1.053 GHz clock)
    // 1 cycle = 1 / KNC_CLOCK_FREQUENCY_HZ seconds = KNC_CYCLE_TIME_NS nanoseconds
    double cycles = transfer_time_ns / KNC_CYCLE_TIME_NS;
    
    return static_cast<uint64_t>(cycles);
}

bool PCIeBridge::transferDataHostToDevice(const void* data, size_t bytes, uint64_t device_address) {
    if (!data || bytes == 0) {
        return false;
    }
    
    std::lock_guard<std::mutex> lock(bridge_mutex);
    
    // Calculate transfer time
    double transfer_time_ns = calculate_transfer_time_ns(bytes);
    uint64_t cycle_delay = calculate_cycle_delay(bytes);
    
    // Simulate transfer delay (0.12-0.125 seconds for typical transfers)
    // For 1GB transfer: 1GB / 8GB/s = 0.125 seconds
    auto delay = std::chrono::duration<double, std::milli>(transfer_time_ns / 1e6);
    std::this_thread::sleep_for(delay);
    
    // Update statistics
    bytes_transferred_host_to_device.fetch_add(bytes);
    total_transfers.fetch_add(1);
    total_latency_ns.fetch_add(static_cast<uint64_t>(transfer_time_ns));
    
    return true;
}

bool PCIeBridge::transferDataDeviceToHost(uint64_t device_address, void* data, size_t bytes) {
    if (!data || bytes == 0) {
        return false;
    }
    
    std::lock_guard<std::mutex> lock(bridge_mutex);
    
    // Calculate transfer time
    double transfer_time_ns = calculate_transfer_time_ns(bytes);
    uint64_t cycle_delay = calculate_cycle_delay(bytes);
    
    // Simulate transfer delay
    auto delay = std::chrono::duration<double, std::milli>(transfer_time_ns / 1e6);
    std::this_thread::sleep_for(delay);
    
    // Update statistics
    bytes_transferred_device_to_host.fetch_add(bytes);
    total_transfers.fetch_add(1);
    total_latency_ns.fetch_add(static_cast<uint64_t>(transfer_time_ns));
    
    return true;
}

bool PCIeBridge::transferData(size_t bytes, bool host_to_device) {
    if (bytes == 0) {
        return false;
    }
    
    std::lock_guard<std::mutex> lock(bridge_mutex);
    
    // Calculate transfer time based on PCIe 2.0 x16 spec
    double transfer_time_ns = calculate_transfer_time_ns(bytes);
    uint64_t cycle_delay = calculate_cycle_delay(bytes);
    
    // Simulate transfer delay (0.12-0.125 seconds for typical transfers)
    // For 1GB transfer: 1GB / 8GB/s = 0.125 seconds
    auto delay = std::chrono::duration<double, std::milli>(transfer_time_ns / 1e6);
    std::this_thread::sleep_for(delay);
    
    // Update statistics
    if (host_to_device) {
        bytes_transferred_host_to_device.fetch_add(bytes);
    } else {
        bytes_transferred_device_to_host.fetch_add(bytes);
    }
    total_transfers.fetch_add(1);
    total_latency_ns.fetch_add(static_cast<uint64_t>(transfer_time_ns));
    
    return true;
}

void PCIeBridge::getStatistics(uint64_t& bytes_host_to_device, 
                               uint64_t& bytes_device_to_host,
                               uint64_t& total_transfers_out,
                               uint64_t& avg_latency_ns) const {
    bytes_host_to_device = bytes_transferred_host_to_device.load();
    bytes_device_to_host = bytes_transferred_device_to_host.load();
    total_transfers_out = total_transfers.load();
    
    uint64_t total_latency = total_latency_ns.load();
    avg_latency_ns = (total_transfers_out > 0) ? (total_latency / total_transfers_out) : 0;
}

void PCIeBridge::getDTDStatistics(uint64_t& coherency_overhead_cycles,
                                  uint64_t& invalidation_count) const {
    coherency_overhead_cycles = cache_coherency_overhead_cycles.load();
    invalidation_count = dtd_invalidation_count.load();
}

void PCIeBridge::resetStatistics() {
    bytes_transferred_host_to_device.store(0);
    bytes_transferred_device_to_host.store(0);
    total_transfers.store(0);
    total_latency_ns.store(0);
    cache_coherency_overhead_cycles.store(0);
    dtd_invalidation_count.store(0);
}

double PCIeBridge::getExpectedTransferTime(size_t bytes) const {
    return calculate_transfer_time_ns(bytes) / 1e6; // Convert to milliseconds
}

double PCIeBridge::getBandwidthUtilization() const {
    uint64_t total_bytes = bytes_transferred_host_to_device.load() + 
                          bytes_transferred_device_to_host.load();
    
    // Calculate utilization percentage
    double max_bandwidth = PCIE_TOTAL_BANDWIDTH_GBPS * BYTES_PER_GB;
    return (static_cast<double>(total_bytes) / max_bandwidth) * 100.0;
}
