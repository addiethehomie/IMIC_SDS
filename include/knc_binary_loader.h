#ifndef KNC_BINARY_LOADER_H
#define KNC_BINARY_LOADER_H

#include <string>
#include <vector>
#include <cstdint>

// ELF structure definitions for KNC binaries
typedef struct {
    uint8_t e_ident[16];
    uint16_t e_type;
    uint16_t e_machine;
    uint32_t e_version;
    uint64_t e_entry;
    uint64_t e_phoff;
    uint64_t e_shoff;
    uint32_t e_flags;
    uint16_t e_ehsize;
    uint16_t e_phentsize;
    uint16_t e_phnum;
    uint16_t e_shentsize;
    uint16_t e_shnum;
    uint16_t e_shstrndx;
} elf64_header_t;

typedef struct {
    uint32_t p_type;
    uint32_t p_flags;
    uint64_t p_offset;
    uint64_t p_vaddr;
    uint64_t p_paddr;
    uint64_t p_filesz;
    uint64_t p_memsz;
    uint64_t p_align;
} elf64_program_header_t;

typedef struct {
    uint32_t sh_name;
    uint32_t sh_type;
    uint64_t sh_flags;
    uint64_t sh_addr;
    uint64_t sh_offset;
    uint64_t sh_size;
    uint32_t sh_link;
    uint32_t sh_info;
    uint64_t sh_addralign;
    uint64_t sh_entsize;
} elf64_section_header_t;

// Symbol table entry
typedef struct {
    uint32_t st_name;
    uint8_t st_info;
    uint8_t st_other;
    uint16_t st_shndx;
    uint64_t st_value;
    uint64_t st_size;
} elf64_symbol_t;

// Relocation entry
typedef struct {
    uint64_t r_offset;
    uint32_t r_info;
    int32_t r_addend;
} elf64_relocation_t;

// KNC Binary information
typedef struct {
    std::string filename;
    uint64_t entry_point;
    std::vector<uint8_t> text_section;
    std::vector<uint8_t> data_section;
    std::vector<elf64_symbol_t> symbols;
    std::vector<elf64_relocation_t> relocations;
    bool is_knc_binary;
} knc_binary_info_t;

class KNCBinaryLoader {
private:
    knc_binary_info_t binary_info;
    bool validate_elf_header(const elf64_header_t* header);
    bool read_program_headers(FILE* file, const elf64_header_t* header);
    bool read_section_headers(FILE* file, const elf64_header_t* header);
    bool read_symbol_table(FILE* file, const elf64_section_header_t* section);
    bool read_relocation_table(FILE* file, const elf64_section_header_t* section);
    bool is_knc_machine(uint16_t machine);
    
public:
    KNCBinaryLoader();
    ~KNCBinaryLoader();
    
    // Load and parse KNC binary
    bool load_binary(const std::string& filename);
    
    // Get binary information
    const knc_binary_info_t& get_binary_info() const;
    const uint8_t* get_binary_data() const;
    uint64_t get_binary_size() const;
    uint64_t get_entry_point() const;
    
    // Symbol resolution
    bool resolve_symbol(const std::string& name, uint64_t& address);
    std::vector<std::string> get_symbol_names() const;
    
    // Relocation processing
    bool process_relocations();
    
    // Utility functions
    void print_binary_info() const;
    void print_symbols() const;
    void print_relocations() const;
};

#endif // KNC_BINARY_LOADER_H
