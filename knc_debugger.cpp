#include "knc_debugger.h"
#include <iostream>
#include <sstream>
#include <algorithm>

// Windows compatibility
#ifdef _WIN32
#include <windows.h>
#define usleep(x) Sleep((x) / 1000)
#else
#include <unistd.h>
#endif

KNCDebugger::KNCDebugger() {
    debugging_enabled.store(false);
    break_requested.store(false);
    
    session.is_debugging = false;
    session.is_paused = false;
    session.current_core = 0;
    session.current_address = 0;
    
    breakpoints_hit = 0;
    watchpoints_hit = 0;
    instructions_stepped = 0;
}

KNCDebugger::~KNCDebugger() {
    shutdown();
}

bool KNCDebugger::initialize() {
    std::cout << "KNC Debugger initialized\n";
    return true;
}

void KNCDebugger::shutdown() {
    if (command_thread.joinable()) {
        command_thread.join();
    }
}

void KNCDebugger::enable_debugging(bool enable) {
    debugging_enabled.store(enable);
    if (enable && !session.is_debugging) {
        session.is_debugging = true;
        command_thread = std::thread(&KNCDebugger::command_loop, this);
    }
}

bool KNCDebugger::is_debugging_enabled() const {
    return debugging_enabled.load();
}

void KNCDebugger::request_break() {
    break_requested.store(true);
}

void KNCDebugger::clear_break_request() {
    break_requested.store(false);
}

bool KNCDebugger::should_break(uint64_t address, uint32_t core_id) {
    if (!debugging_enabled.load()) {
        return false;
    }
    
    return check_breakpoints(address, core_id) || break_requested.load();
}

bool KNCDebugger::should_pause() const {
    return session.is_paused || break_requested.load();
}

void KNCDebugger::notify_breakpoint_hit(uint64_t address, uint32_t core_id) {
    breakpoints_hit++;
    session.current_address = address;
    session.current_core = core_id;
    session.is_paused = true;
    
    std::cout << "\nBreakpoint hit at 0x" << std::hex << address 
              << " on core " << core_id << std::dec << "\n";
    print_prompt();
}

void KNCDebugger::notify_watchpoint_hit(uint64_t address, uint32_t core_id, bool is_write) {
    watchpoints_hit++;
    session.current_address = address;
    session.current_core = core_id;
    session.is_paused = true;
    
    std::cout << "\nWatchpoint hit at 0x" << std::hex << address 
              << " on core " << core_id << " (" 
              << (is_write ? "write" : "read") << ")" << std::dec << "\n";
    print_prompt();
}

bool KNCDebugger::add_breakpoint(uint64_t address, knc_breakpoint_type_t type, uint32_t core_mask) {
    std::lock_guard<std::mutex> lock(breakpoints_mutex);
    
    // Check if breakpoint already exists
    for (const auto& bp : breakpoints) {
        if (bp.address == address) {
            return false; // Breakpoint already exists
        }
    }
    
    knc_breakpoint_t bp;
    bp.address = address;
    bp.type = type;
    bp.core_id = core_mask; // Use core_id field for core_mask
    bp.enabled = true;
    bp.hit_count = 0;
    bp.condition = "";
    
    breakpoints.push_back(bp);
    breakpoint_index[address] = breakpoints.size() - 1;
    return true;
}

bool KNCDebugger::set_watchpoint(uint64_t address, size_t size, bool is_write) {
    std::lock_guard<std::mutex> lock(breakpoints_mutex);
    
    knc_breakpoint_t wp;
    wp.address = address;
    wp.type = KNC_BREAKPOINT_MEMORY;
    wp.core_id = 0xFFFFFFFF;  // All cores
    wp.enabled = true;
    wp.hit_count = 0;
    
    watchpoints.push_back(wp);
    watchpoint_index[address] = watchpoints.size() - 1;
    
    std::cout << "Watchpoint set at 0x" << std::hex << address 
              << " size " << size << " (" << (is_write ? "write" : "read") 
              << ")" << std::dec << "\n";
    return true;
}

bool KNCDebugger::check_breakpoints(uint64_t address, uint32_t core_id) {
    std::lock_guard<std::mutex> lock(breakpoints_mutex);
    
    auto it = breakpoint_index.find(address);
    if (it != breakpoint_index.end()) {
        size_t index = it->second;
        if (index < breakpoints.size() && breakpoints[index].enabled) {
            breakpoints[index].hit_count++;
            return true;
        }
    }
    return false;
}

bool KNCDebugger::check_watchpoints(uint64_t address, size_t size, bool is_write) {
    std::lock_guard<std::mutex> lock(breakpoints_mutex);
    
    for (auto& wp : watchpoints) {
        if (wp.enabled && address >= wp.address && address < wp.address + size) {
            wp.hit_count++;
            return true;
        }
    }
    return false;
}

void KNCDebugger::command_loop() {
    std::string command_line;
    
    while (session.is_debugging) {
        print_prompt();
        std::getline(std::cin, command_line);
        
        if (!command_line.empty()) {
            session.command_history.push_back(command_line);
            process_command(command_line);
        }
    }
}

void KNCDebugger::process_command(const std::string& command_line) {
    std::istringstream iss(command_line);
    std::string command;
    iss >> command;
    
    std::vector<std::string> args;
    std::string arg;
    while (iss >> arg) {
        args.push_back(arg);
    }
    
    if (command == "continue" || command == "c") {
        cmd_continue(args);
    } else if (command == "step" || command == "s") {
        cmd_step(args);
    } else if (command == "registers" || command == "r") {
        cmd_registers(args);
    } else if (command == "memory" || command == "m") {
        cmd_memory(args);
    } else if (command == "break" || command == "b") {
        cmd_break(args);
    } else if (command == "quit" || command == "q") {
        cmd_quit(args);
    } else if (command == "help" || command == "h") {
        print_help();
    } else {
        std::cout << "Unknown command: " << command << "\n";
        print_help();
    }
}

void KNCDebugger::cmd_continue(const std::vector<std::string>& args) {
    session.is_paused = false;
    clear_break_request();
    std::cout << "Continuing execution...\n";
}

void KNCDebugger::cmd_step(const std::vector<std::string>& args) {
    instructions_stepped++;
    session.is_paused = false;
    std::cout << "Stepping one instruction...\n";
}

void KNCDebugger::cmd_registers(const std::vector<std::string>& args) {
    std::cout << "\n=== Core " << session.current_core << " Registers ===\n";
    std::cout << "RIP: 0x" << std::hex << session.current_address << std::dec << "\n";
    std::cout << "RAX: 0x" << std::hex << "0x12345678" << std::dec << "\n";
    std::cout << "RBX: 0x" << std::hex << "0x87654321" << std::dec << "\n";
    std::cout << "RCX: 0x" << std::hex << "0xABCDEF00" << std::dec << "\n";
    std::cout << "RDX: 0x" << std::hex << "0x12345678" << std::dec << "\n";
}

void KNCDebugger::cmd_memory(const std::vector<std::string>& args) {
    if (args.size() < 2) {
        std::cout << "Usage: memory <address> <size>\n";
        return;
    }
    
    uint64_t address = parse_address(args[0]);
    size_t size = std::stoul(args[1]);
    
    dump_memory(address, size);
}

void KNCDebugger::cmd_break(const std::vector<std::string>& args) {
    if (args.empty()) {
        list_breakpoints();
        return;
    }
    
    uint64_t address = parse_address(args[0]);
    add_breakpoint(address, KNC_BREAKPOINT_EXECUTION, 0xFFFFFFFF);
}

void KNCDebugger::cmd_quit(const std::vector<std::string>& args) {
    session.is_debugging = false;
    session.is_paused = false;
    std::cout << "Exiting debugger...\n";
}

uint64_t KNCDebugger::parse_address(const std::string& addr_str) {
    if (addr_str.substr(0, 2) == "0x") {
        return std::stoull(addr_str.substr(2), nullptr, 16);
    }
    return std::stoull(addr_str);
}

void KNCDebugger::dump_memory(uint64_t address, size_t size) {
    std::cout << "\n=== Memory at 0x" << std::hex << address 
              << " (+" << size << " bytes) ===" << std::dec << "\n";
    
    for (size_t i = 0; i < size; i += 16) {
        std::cout << "0x" << std::hex << (address + i) << ": ";
        
        for (size_t j = 0; j < 16 && (i + j) < size; j++) {
            std::cout << "00 ";  // Placeholder data
        }
        
        std::cout << "\n";
    }
}

void KNCDebugger::list_breakpoints() {
    std::lock_guard<std::mutex> lock(breakpoints_mutex);
    
    std::cout << "\n=== Breakpoints ===\n";
    for (size_t i = 0; i < breakpoints.size(); i++) {
        const auto& bp = breakpoints[i];
        std::cout << i << ": 0x" << std::hex << bp.address 
                  << std::dec << " (" << (bp.enabled ? "enabled" : "disabled") 
                  << ") hits: " << bp.hit_count << "\n";
    }
}

void KNCDebugger::print_help() {
    std::cout << "\n=== KNC Debugger Commands ===\n";
    std::cout << "continue (c)     - Continue execution\n";
    std::cout << "step (s)         - Single step instruction\n";
    std::cout << "registers (r)     - Show registers\n";
    std::cout << "memory <addr> <size> - Display memory\n";
    std::cout << "break <addr>      - Set breakpoint\n";
    std::cout << "quit (q)          - Exit debugger\n";
    std::cout << "help (h)          - Show this help\n";
}

void KNCDebugger::print_prompt() {
    std::cout << "(knc-debug) ";
}

void KNCDebugger::print_debug_statistics() const {
    std::cout << "\n=== Debug Statistics ===\n";
    std::cout << "Breakpoints hit: " << breakpoints_hit << "\n";
    std::cout << "Watchpoints hit: " << watchpoints_hit << "\n";
    std::cout << "Instructions stepped: " << instructions_stepped << "\n";
}
