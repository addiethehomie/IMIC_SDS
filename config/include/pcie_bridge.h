#ifndef PCIE_BRIDGE_H
#define PCIE_BRIDGE_H

#include <cstdint>
#include <vector>
#include <mutex>
#include <atomic>
#include <chrono>
#include "knc_types.h"

// PCIe 2.0 x16 Specification
// 8GB/s each direction, 16GB/s total bidirectional bandwidth
// 5 GT/s (Gigatransfers per second) per lane
// 16 lanes = 80 GT/s total
// 8b/10b encoding reduces effective bandwidth to 80%

class PCIeBridge {
private:
    // PCIe 2.0 x16 specifications
    static constexpr double PCIE_BANDWIDTH_GBPS = 8.0;  // 8GB/s per direction
    static constexpr double PCIE_TOTAL_BANDWIDTH_GBPS = 16.0;  // 16GB/s total
    static constexpr double PCIE_LATENCY_NS = 100.0;  // Base latency ~100ns
    static constexpr uint64_t BYTES_PER_GB = 1024ULL * 1024 * 1024;
    
    // Bridge state
    std::atomic<uint64_t> bytes_transferred_host_to_device;
    std::atomic<uint64_t> bytes_transferred_device_to_host;
    std::atomic<uint64_t> total_transfers;
    std::atomic<uint64_t> total_latency_ns;
    
    // Statistics
    std::atomic<uint64_t> cache_coherency_overhead_cycles;
    std::atomic<uint64_t> dtd_invalidation_count;
    
    // Synchronization
    std::mutex bridge_mutex;
    
    // Calculate transfer time based on PCIe 2.0 x16 bandwidth
    double calculate_transfer_time_ns(size_t bytes) const;
    
    // Calculate cycle delay for simulation (0.12-0.125 seconds for typical transfers)
    uint64_t calculate_cycle_delay(size_t bytes) const;
    
public:
    PCIeBridge();
    ~PCIeBridge();
    
    // Transfer data from host to device (MMUs)
    // Returns true if transfer successful, false otherwise
    bool transferDataHostToDevice(const void* data, size_t bytes, uint64_t device_address);
    
    // Transfer data from device (MMUs) to host
    // Returns true if transfer successful, false otherwise
    bool transferDataDeviceToHost(uint64_t device_address, void* data, size_t bytes);
    
    // Generic transfer function (bidirectional)
    // Calculates latency based on PCIe 2.0 x16 spec
    bool transferData(size_t bytes, bool host_to_device);
    
    // Get bridge statistics
    void getStatistics(uint64_t& bytes_host_to_device, 
                      uint64_t& bytes_device_to_host,
                      uint64_t& total_transfers,
                      uint64_t& avg_latency_ns) const;
    
    // Get DTD coherency statistics
    void getDTDStatistics(uint64_t& coherency_overhead_cycles,
                         uint64_t& invalidation_count) const;
    
    // Reset bridge statistics
    void resetStatistics();
    
    // Calculate expected transfer time for given size
    double getExpectedTransferTime(size_t bytes) const;
    
    // Get bridge bandwidth utilization
    double getBandwidthUtilization() const;
};

#endif // PCIE_BRIDGE_H
