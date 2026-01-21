#ifndef KNC_TYPES_H
#define KNC_TYPES_H

#include <cstdint>
#include <immintrin.h>

// Architecture Detection
typedef enum {
    ARCH_KNC = 0,    // Knights Corner (Xeon Phi 5110P/7120P)
    ARCH_KNL = 1,    // Knights Landing (Xeon Phi 7210/7250)
    ARCH_KNF = 2     // Knights Ferry (internal prototype)
} knc_architecture_t;

// KNC Architecture Constants (Knights Corner)
#define KNC_NUM_CORES 60
#define KNC_CORES_PER_TILE 4
#define KNC_NUM_TILES (KNC_NUM_CORES / KNC_CORES_PER_TILE)
#define KNC_NUM_VECTOR_REGISTERS 32
#define KNC_VECTOR_SIZE 512
#define KNC_VECTOR_BYTES (KNC_VECTOR_SIZE / 8)
#define KNC_NUM_MASK_REGISTERS 8
#define KNC_L1_CACHE_SIZE 32 * 1024
#define KNC_L2_CACHE_SIZE 512 * 1024
#define KNC_MEMORY_SIZE (8ULL * 1024 * 1024 * 1024)  // 8GB
#define KNC_NUM_MMUS 8
#define KNC_MMU_SIZE (KNC_MEMORY_SIZE / KNC_NUM_MMUS)  // 1GB per MMU

// Xeon Phi 5110P Clock Speed
#define KNC_CLOCK_FREQUENCY_HZ 1053000000ULL  // 1.053 GHz (1053 MHz)
#define KNC_CLOCK_FREQUENCY_MHZ 1053          // 1053 MHz
#define KNC_CLOCK_FREQUENCY_GHZ 1.053         // 1.053 GHz
#define KNC_CYCLE_TIME_NS 0.95                // 1 cycle = 0.95 nanoseconds (1 / 1.053e9)

// KNL Architecture Constants (Knights Landing)
#define KNL_NUM_CORES 68
#define KNL_CORES_PER_TILE 2
#define KNL_NUM_TILES (KNL_NUM_CORES / KNL_CORES_PER_TILE)
#define KNL_NUM_VECTOR_REGISTERS 32
#define KNL_VECTOR_SIZE 512
#define KNL_VECTOR_BYTES (KNL_VECTOR_SIZE / 8)
#define KNL_NUM_MASK_REGISTERS 8
#define KNL_L1_CACHE_SIZE 32 * 1024
#define KNL_L2_CACHE_SIZE 1024 * 1024  // 1MB per tile
#define KNL_MEMORY_SIZE (16ULL * 1024 * 1024 * 1024)  // 16GB
#define KNL_NUM_MMUS 38
#define KNL_MMU_SIZE (KNL_MEMORY_SIZE / KNL_NUM_MMUS)  // ~421MB per MMU

// Xeon Phi 7210/7250 Clock Speed
#define KNL_CLOCK_FREQUENCY_HZ 1400000000ULL  // 1.4 GHz base, turbo up to 1.5 GHz
#define KNL_CLOCK_FREQUENCY_MHZ 1400          // 1.4 GHz base
#define KNL_CLOCK_FREQUENCY_GHZ 1.4           // 1.4 GHz base
#define KNL_CYCLE_TIME_NS 0.71                // 1 cycle = 0.71 nanoseconds (1 / 1.4e9)

// KNL Ring Bus Constants (Dual bidirectional rings)
#define KNL_NUM_RINGS 2
#define KNL_RING_BANDWIDTH_GBPS 213.312  // Dual ring: 2 × 106.656 GB/s
#define KNL_RING_LATENCY_CYCLES 2

// Runtime Architecture Selection
extern knc_architecture_t current_architecture;

// KNC Register Types
typedef struct {
    __m512i zmm[KNC_NUM_VECTOR_REGISTERS];  // 512-bit vector registers
    __mmask16 k[KNC_NUM_MASK_REGISTERS];    // Mask registers
    uint64_t gpr[16];                           // General purpose registers
    uint64_t rip;                               // Instruction pointer
    uint64_t rflags;                            // Flags register
} knc_register_file_t;

// KNC Memory Types
typedef struct {
    uint8_t* data;
    uint64_t size;
    uint64_t base_address;
} knc_memory_t;

// KNC MMU (Memory Management Unit) - 8 units spaced around ring bus
typedef struct {
    uint32_t mmu_id;
    uint64_t base_address;
    uint64_t size;
    uint32_t tile_id;  // Associated tile for symmetric placement
    uint64_t accesses;
    uint64_t cache_hits;
    uint64_t cache_misses;
} knc_mmu_t;

// KNC Memory System with 8 MMUs
typedef struct {
    knc_mmu_t mmus[KNC_NUM_MMUS];
    uint64_t total_size;
    uint32_t active_mmus;
} knc_memory_system_t;

// KNC Core State
typedef struct {
    knc_register_file_t registers;
    uint32_t core_id;
    uint32_t tile_id;
    bool is_halted;
    uint64_t cycles_executed;
} knc_core_state_t;

// KNC Instruction Types
typedef enum {
    KNC_INST_ADD_PS = 0x58,
    KNC_INST_SUB_PS = 0x5C,
    KNC_INST_MUL_PS = 0x59,
    KNC_INST_DIV_PS = 0x5E,
    KNC_INST_FMADD_PS = 0x69,
    KNC_INST_FMSUB_PS = 0x6A,
    KNC_INST_FNMSUB_PS = 0x6B,
    KNC_INST_FNMADD_PS = 0x6D,
    KNC_INST_VPADDD = 0xFE,
    KNC_INST_VPSUBD = 0xFA,
    KNC_INST_VPMULUD = 0x71,
    KNC_INST_VPERMD = 0x7E,
    KNC_INST_VPBROADCASTD = 0x7C,
    KNC_INST_VGATHERDPS = 0x7D,
    KNC_INST_VSCATTERDPS = 0x7F,
    KNC_INST_VCMPPS = 0xC2,
    KNC_INST_VMAXPS = 0x5F,
    KNC_INST_VMINPS = 0x5D,
    // KNL-specific instructions
    KNL_INST_VEXPANDPD = 0x82,
    KNL_INST_VCOMPRESSPD = 0x8A,
    KNL_INST_VPERMILPD = 0x8C,
    KNL_INST_VPERMD2 = 0x8D,
    KNL_INST_VPERMT2D = 0x8E,
    KNL_INST_VPMOVD = 0x8F,
    KNL_INST_VFMADDPD231PS = 0x98,
    KNL_INST_VFMADDSUBPD231PS = 0x9A,
    KNL_INST_VFMADDSUB132PS = 0x9B,
    KNL_INST_VFMSUBADDPD231PS = 0x9C,
    KNL_INST_VFMSUBADD132PS = 0x9D
} knc_instruction_type_t;

// KNC Ring Bus Types
typedef struct {
    uint32_t source_tile;
    uint32_t dest_tile;
    uint8_t* data;
    uint32_t size;
    uint64_t timestamp;
} knc_ring_message_t;

typedef struct {
    uint32_t tile_id;
    uint32_t bandwidth_mbps;
    uint32_t latency_cycles;
    uint32_t buffer_size;
} knc_ring_node_t;

// KNC Performance Counters
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
    uint64_t branch_instructions;
    uint64_t branch_mispredictions;
} knc_performance_counters_t;

// KNC System Call Types
typedef enum {
    KNC_SYSCALL_EXIT = 60,
    KNC_SYSCALL_READ = 0,
    KNC_SYSCALL_WRITE = 1,
    KNC_SYSCALL_OPEN = 2,
    KNC_SYSCALL_CLOSE = 3,
    KNC_SYSCALL_MMAP = 9,
    KNC_SYSCALL_MUNMAP = 11,
    KNC_SYSCALL_BRK = 12,
    KNC_SYSCALL_RT_SIGACTION = 13,
    KNC_SYSCALL_RT_SIGPROCMASK = 14,
    KNC_SYSCALL_RT_SIGRETURN = 15,
    KNC_SYSCALL_IOCTL = 16,
    KNC_SYSCALL_PREAD64 = 17,
    KNC_SYSCALL_PWRITE64 = 18,
    KNC_SYSCALL_READV = 19,
    KNC_SYSCALL_WRITEV = 20,
    KNC_SYSCALL_ACCESS = 21,
    KNC_SYSCALL_PIPE = 22,
    KNC_SYSCALL_SELECT = 23
} knc_syscall_type_t;

// Error codes
typedef enum {
    KNC_SUCCESS = 0,
    KNC_ERROR_INVALID_ARGUMENT = -1,
    KNC_ERROR_MEMORY_ACCESS = -2,
    KNC_ERROR_INVALID_INSTRUCTION = -3,
    KNC_ERROR_DIVIDE_BY_ZERO = -4,
    KNC_ERROR_OVERFLOW = -5,
    KNC_ERROR_UNDERFLOW = -6,
    KNC_ERROR_SYSTEM_CALL = -7
} knc_error_t;

// Architecture Detection Functions
static inline knc_architecture_t detect_host_architecture() {
    // Simple CPUID-based detection for now
    // In a real implementation, this would use CPUID instructions
    // For now, default to KNC for backward compatibility
    return ARCH_KNC;
}

static inline const char* get_architecture_name(knc_architecture_t arch) {
    switch (arch) {
        case ARCH_KNC: return "Knights Corner (KNC)";
        case ARCH_KNL: return "Knights Landing (KNL)";
        case ARCH_KNF: return "Knights Ferry (KNF)";
        default: return "Unknown";
    }
}

static inline uint32_t get_num_cores(knc_architecture_t arch) {
    switch (arch) {
        case ARCH_KNC: return KNC_NUM_CORES;
        case ARCH_KNL: return KNL_NUM_CORES;
        case ARCH_KNF: return KNC_NUM_CORES; // Assume same as KNC
        default: return KNC_NUM_CORES;
    }
}

static inline uint64_t get_memory_size(knc_architecture_t arch) {
    switch (arch) {
        case ARCH_KNC: return KNC_MEMORY_SIZE;
        case ARCH_KNL: return KNL_MEMORY_SIZE;
        case ARCH_KNF: return KNC_MEMORY_SIZE; // Assume same as KNC
        default: return KNC_MEMORY_SIZE;
    }
}

static inline uint64_t get_clock_frequency(knc_architecture_t arch) {
    switch (arch) {
        case ARCH_KNC: return KNC_CLOCK_FREQUENCY_HZ;
        case ARCH_KNL: return KNL_CLOCK_FREQUENCY_HZ;
        case ARCH_KNF: return KNC_CLOCK_FREQUENCY_HZ; // Assume same as KNC
        default: return KNC_CLOCK_FREQUENCY_HZ;
    }
}

static inline uint32_t get_num_mmus(knc_architecture_t arch) {
    switch (arch) {
        case ARCH_KNC: return KNC_NUM_MMUS;
        case ARCH_KNL: return KNL_NUM_MMUS;
        case ARCH_KNF: return KNC_NUM_MMUS; // Assume same as KNC
        default: return KNC_NUM_MMUS;
    }
}

#endif // KNC_TYPES_H
