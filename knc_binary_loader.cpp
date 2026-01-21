/*
 * Copyright (c) 2026 IMIC_SDS Development Team
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "knc_binary_loader.h"
#include <iostream>
#include <fstream>
#include <cstring>

// Windows compatibility
#ifdef _WIN32
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <arpa/inet.h>
#endif

// ELF constants
#define ELF_MAGIC 0x464C457F
#define EM_K1OM 181  // KNC architecture identifier
#define ET_EXEC 2
#define PT_LOAD 1
#define PF_X 1
#define PF_W 2
#define PF_R 4
#define SHT_SYMTAB 2
#define SHT_STRTAB 3
#define SHT_RELA 4

KNCBinaryLoader::KNCBinaryLoader() {
    memset(&binary_info, 0, sizeof(binary_info));
}

KNCBinaryLoader::~KNCBinaryLoader() {
}

bool KNCBinaryLoader::load_binary(const std::string& filename) {
    binary_info.filename = filename;
    
    FILE* file = fopen(filename.c_str(), "rb");
    if (!file) {
        std::cerr << "Error: Cannot open file " << filename << "\n";
        return false;
    }
    
    // Read ELF header
    elf64_header_t header;
    if (fread(&header, sizeof(header), 1, file) != 1) {
        std::cerr << "Error: Cannot read ELF header\n";
        fclose(file);
        return false;
    }
    
    // Validate ELF header
    if (!validate_elf_header(&header)) {
        fclose(file);
        return false;
    }
    
    binary_info.entry_point = header.e_entry;
    binary_info.is_knc_binary = is_knc_machine(header.e_machine);
    
    if (!binary_info.is_knc_binary) {
        std::cerr << "Warning: Binary may not be for KNC architecture (machine: " 
                  << header.e_machine << ")\n";
    }
    
    // Read program headers
    if (!read_program_headers(file, &header)) {
        fclose(file);
        return false;
    }
    
    // Read section headers
    if (!read_section_headers(file, &header)) {
        fclose(file);
        return false;
    }
    
    fclose(file);
    
    std::cout << "Successfully loaded KNC binary: " << filename << "\n";
    std::cout << "Entry point: 0x" << std::hex << binary_info.entry_point << std::dec << "\n";
    
    return true;
}

bool KNCBinaryLoader::validate_elf_header(const elf64_header_t* header) {
    // Check ELF magic number
    if (header->e_ident[0] != 0x7F || 
        header->e_ident[1] != 'E' || 
        header->e_ident[2] != 'L' || 
        header->e_ident[3] != 'F') {
        std::cerr << "Error: Invalid ELF magic number\n";
        return false;
    }
    
    // Check 64-bit ELF
    if (header->e_ident[4] != 2) {
        std::cerr << "Error: Not a 64-bit ELF file\n";
        return false;
    }
    
    // Check little-endian
    if (header->e_ident[5] != 1) {
        std::cerr << "Error: Not little-endian\n";
        return false;
    }
    
    return true;
}

bool KNCBinaryLoader::read_program_headers(FILE* file, const elf64_header_t* header) {
    if (header->e_phnum == 0) {
        return true;  // No program headers is valid
    }
    
    fseek(file, header->e_phoff, SEEK_SET);
    
    for (int i = 0; i < header->e_phnum; i++) {
        elf64_program_header_t phdr;
        if (fread(&phdr, sizeof(phdr), 1, file) != 1) {
            std::cerr << "Error: Cannot read program header " << i << "\n";
            return false;
        }
        
        // Load loadable segments
        if (phdr.p_type == PT_LOAD) {
            std::vector<uint8_t> segment_data(phdr.p_memsz);
            
            // Seek to segment data
            fseek(file, phdr.p_offset, SEEK_SET);
            
            // Read segment data
            if (fread(segment_data.data(), phdr.p_filesz, 1, file) != 1) {
                std::cerr << "Error: Cannot read segment data\n";
                return false;
            }
            
            // Zero-fill remaining bytes if memsz > filesz
            if (phdr.p_memsz > phdr.p_filesz) {
                memset(segment_data.data() + phdr.p_filesz, 0, 
                       phdr.p_memsz - phdr.p_filesz);
            }
            
            // Store segment based on permissions
            if (phdr.p_flags & PF_X) {
                // Executable segment (text)
                binary_info.text_section = segment_data;
            } else if (phdr.p_flags & PF_W) {
                // Writable segment (data)
                binary_info.data_section = segment_data;
            }
        }
    }
    
    return true;
}

bool KNCBinaryLoader::read_section_headers(FILE* file, const elf64_header_t* header) {
    if (header->e_shnum == 0) {
        return true;  // No section headers is valid
    }
    
    fseek(file, header->e_shoff, SEEK_SET);
    
    // Read all section headers
    std::vector<elf64_section_header_t> sections(header->e_shnum);
    if (fread(sections.data(), sizeof(elf64_section_header_t), header->e_shnum, file) != header->e_shnum) {
        std::cerr << "Error: Cannot read section headers\n";
        return false;
    }
    
    // Process each section
    for (int i = 0; i < header->e_shnum; i++) {
        const elf64_section_header_t& shdr = sections[i];
        
        switch (shdr.sh_type) {
            case SHT_SYMTAB:
                if (!read_symbol_table(file, &shdr)) {
                    return false;
                }
                break;
                
            case SHT_RELA:
                if (!read_relocation_table(file, &shdr)) {
                    return false;
                }
                break;
        }
    }
    
    return true;
}

bool KNCBinaryLoader::read_symbol_table(FILE* file, const elf64_section_header_t* section) {
    if (section->sh_entsize == 0) {
        return true;  // Empty symbol table
    }
    
    fseek(file, section->sh_offset, SEEK_SET);
    
    int num_symbols = section->sh_size / section->sh_entsize;
    binary_info.symbols.resize(num_symbols);
    
    if (fread(binary_info.symbols.data(), section->sh_entsize, num_symbols, file) != num_symbols) {
        std::cerr << "Error: Cannot read symbol table\n";
        return false;
    }
    
    return true;
}

bool KNCBinaryLoader::read_relocation_table(FILE* file, const elf64_section_header_t* section) {
    if (section->sh_entsize == 0) {
        return true;  // Empty relocation table
    }
    
    fseek(file, section->sh_offset, SEEK_SET);
    
    int num_relocations = section->sh_size / section->sh_entsize;
    binary_info.relocations.resize(num_relocations);
    
    if (fread(binary_info.relocations.data(), section->sh_entsize, num_relocations, file) != num_relocations) {
        std::cerr << "Error: Cannot read relocation table\n";
        return false;
    }
    
    return true;
}

bool KNCBinaryLoader::is_knc_machine(uint16_t machine) {
    return machine == EM_K1OM;
}

const knc_binary_info_t& KNCBinaryLoader::get_binary_info() const {
    return binary_info;
}

const uint8_t* KNCBinaryLoader::get_binary_data() const {
    if (!binary_info.text_section.empty()) {
        return binary_info.text_section.data();
    }
    return binary_info.data_section.data();
}

uint64_t KNCBinaryLoader::get_binary_size() const {
    if (!binary_info.text_section.empty()) {
        return binary_info.text_section.size();
    }
    return binary_info.data_section.size();
}

uint64_t KNCBinaryLoader::get_entry_point() const {
    return binary_info.entry_point;
}

bool KNCBinaryLoader::resolve_symbol(const std::string& name, uint64_t& address) {
    for (const auto& symbol : binary_info.symbols) {
        // This is simplified - in practice we'd need string table support
        if (symbol.st_value != 0) {
            address = symbol.st_value;
            return true;
        }
    }
    return false;
}

std::vector<std::string> KNCBinaryLoader::get_symbol_names() const {
    std::vector<std::string> names;
    // Simplified - would need string table support
    return names;
}

bool KNCBinaryLoader::process_relocations() {
    // Simplified relocation processing
    // In practice, this would handle all relocation types
    return true;
}

void KNCBinaryLoader::print_binary_info() const {
    std::cout << "=== KNC Binary Information ===\n";
    std::cout << "Filename: " << binary_info.filename << "\n";
    std::cout << "Entry point: 0x" << std::hex << binary_info.entry_point << std::dec << "\n";
    std::cout << "KNC binary: " << (binary_info.is_knc_binary ? "Yes" : "No") << "\n";
    std::cout << "Text section size: " << binary_info.text_section.size() << " bytes\n";
    std::cout << "Data section size: " << binary_info.data_section.size() << " bytes\n";
    std::cout << "Symbols: " << binary_info.symbols.size() << "\n";
    std::cout << "Relocations: " << binary_info.relocations.size() << "\n";
}

void KNCBinaryLoader::print_symbols() const {
    std::cout << "=== Symbols ===\n";
    for (size_t i = 0; i < binary_info.symbols.size(); i++) {
        const auto& symbol = binary_info.symbols[i];
        std::cout << i << ": value=0x" << std::hex << symbol.st_value 
                  << std::dec << " size=" << symbol.st_size << "\n";
    }
}

void KNCBinaryLoader::print_relocations() const {
    std::cout << "=== Relocations ===\n";
    for (size_t i = 0; i < binary_info.relocations.size(); i++) {
        const auto& reloc = binary_info.relocations[i];
        std::cout << i << ": offset=0x" << std::hex << reloc.r_offset 
                  << std::dec << " info=0x" << std::hex << reloc.r_info 
                  << std::dec << " addend=" << reloc.r_addend << "\n";
    }
}
