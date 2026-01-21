#ifndef KNC_INSTRUCTION_TRANSLATOR_H
#define KNC_INSTRUCTION_TRANSLATOR_H

#include <vector>
#include <unordered_map>
#include <cstdint>
#include <string>
#include <memory>

// XED includes for KNC instruction translation
#include <xed/xed-types.h>
#include <xed/xed-decoded-inst.h>
#include <xed/xed-encode.h>
#include <xed/xed-iclass-enum.h>

#include "knc_types.h"

// Translation context for instruction decoding
typedef struct {
    xed_decoded_inst_t xedd;
    knc_instruction_type_t knc_type;
    uint32_t instruction_length;
    uint8_t* instruction_bytes;
    bool is_vector_instruction;
    bool is_knc_specific;
    bool needs_translation;
} knc_translation_context_t;

// Translated instruction information
typedef struct {
    std::vector<uint8_t> translated_bytes;
    uint32_t translated_length;
    std::string description;
    bool is_emulated;
    uint32_t emulation_overhead_cycles;
} knc_translated_instruction_t;

// Instruction translation cache
typedef struct {
    uint64_t original_address;
    uint8_t original_bytes[16];  // Max KNC instruction length
    knc_translated_instruction_t translated;
    uint32_t access_count;
    bool is_valid;
} knc_translation_cache_entry_t;

class KNCInstructionTranslator {
private:
    // XED decoder state
    xed_state_t xed_state;
    xed_decoded_inst_t xedd;
    
    // Translation cache
    std::vector<knc_translation_cache_entry_t> translation_cache;
    std::unordered_map<uint64_t, size_t> cache_index;
    static const size_t CACHE_SIZE = 16384;  // 16K entries
    
    // Instruction mapping tables
    std::unordered_map<xed_iclass_enum_t, knc_instruction_type_t> xed_to_knc_map;
    std::unordered_map<knc_instruction_type_t, std::string> knc_instruction_names;
    
    // Statistics
    uint64_t instructions_translated;
    uint64_t cache_hits;
    uint64_t cache_misses;
    uint64_t knc_specific_instructions;
    uint64_t vector_instructions;
    
    // Initialization
    void initialize_instruction_maps();
    void setup_xed_decoder();
    
    // Cache management
    size_t get_cache_index(uint64_t address);
    void add_to_cache(uint64_t address, const uint8_t* original_bytes, 
                    const knc_translated_instruction_t& translated);
    bool lookup_in_cache(uint64_t address, knc_translated_instruction_t& translated);
    void invalidate_cache_entry(uint64_t address);
    
    // Instruction translation functions
    knc_translated_instruction_t translate_vector_instruction(const knc_translation_context_t& ctx);
    knc_translated_instruction_t translate_scalar_instruction(const knc_translation_context_t& ctx);
    knc_translated_instruction_t translate_memory_instruction(const knc_translation_context_t& ctx);
    knc_translated_instruction_t translate_control_instruction(const knc_translation_context_t& ctx);
    
    // KNC-specific instruction translations
    knc_translated_instruction_t translate_vpaddd(const knc_translation_context_t& ctx);
    knc_translated_instruction_t translate_vpsubd(const knc_translation_context_t& ctx);
    knc_translated_instruction_t translate_vpmulud(const knc_translation_context_t& ctx);
    knc_translated_instruction_t translate_vpermd(const knc_translation_context_t& ctx);
    knc_translated_instruction_t translate_vpbroadcastd(const knc_translation_context_t& ctx);
    knc_translated_instruction_t translate_vgatherdps(const knc_translation_context_t& ctx);
    knc_translated_instruction_t translate_vscatterdps(const knc_translation_context_t& ctx);
    knc_translated_instruction_t translate_vcmpps(const knc_translation_context_t& ctx);
    knc_translated_instruction_t translate_vmaxps(const knc_translation_context_t& ctx);
    knc_translated_instruction_t translate_vminps(const knc_translation_context_t& ctx);
    
    // Vector operation helpers
    std::vector<uint8_t> generate_avx512_vector_op(const knc_translation_context_t& ctx, 
                                                  const std::string& avx512_mnemonic);
    std::vector<uint8_t> generate_vector_broadcast(const knc_translation_context_t& ctx);
    std::vector<uint8_t> generate_vector_gather(const knc_translation_context_t& ctx);
    std::vector<uint8_t> generate_vector_scatter(const knc_translation_context_t& ctx);
    std::vector<uint8_t> generate_vector_permute(const knc_translation_context_t& ctx);
    
    // Register mapping
    uint32_t map_knc_register_to_x86(uint32_t knc_reg);
    std::string get_x86_register_name(uint32_t knc_reg);
    
    // Memory access helpers
    std::vector<uint8_t> generate_memory_load(const knc_translation_context_t& ctx, uint32_t size);
    std::vector<uint8_t> generate_memory_store(const knc_translation_context_t& ctx, uint32_t size);
    
    // Utility functions
    bool is_knc_vector_instruction(xed_iclass_enum_t iclass);
    bool is_knc_specific_instruction(xed_iclass_enum_t iclass);
    knc_instruction_type_t classify_instruction(const xed_decoded_inst_t& xedd);
    
public:
    KNCInstructionTranslator();
    ~KNCInstructionTranslator();
    
    // Initialization
    bool initialize();
    void shutdown();
    
    // Main translation interface
    knc_translated_instruction_t translate_instruction(uint64_t address, const uint8_t* instruction_bytes);
    knc_translated_instruction_t translate_block(uint64_t start_address, const uint8_t* block_bytes, size_t block_size);
    
    // Cache management
    void flush_translation_cache();
    void invalidate_cache_range(uint64_t start_address, uint64_t size);
    
    // State queries
    const knc_translation_context_t& get_last_translation_context() const;
    
    // Statistics
    void print_translation_statistics() const;
    uint64_t get_instructions_translated() const;
    uint64_t get_cache_hit_rate() const;
    
    // Debugging
    void dump_translation_cache() const;
    void print_instruction_translation(uint64_t address, const uint8_t* original, 
                                   const knc_translated_instruction_t& translated) const;
    
    // Configuration
    void set_cache_size(size_t size);
    void enable_debug_output(bool enable);
};

#endif // KNC_INSTRUCTION_TRANSLATOR_H
