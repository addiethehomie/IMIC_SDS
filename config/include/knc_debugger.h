#ifndef KNC_DEBUGGER_H
#define KNC_DEBUGGER_H

#include <string>
#include <vector>
#include <unordered_map>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>

#include "knc_types.h"

// Breakpoint types
typedef enum {
    KNC_BREAKPOINT_NONE = 0,
    KNC_BREAKPOINT_EXECUTION = 1,
    KNC_BREAKPOINT_MEMORY = 2,
    KNC_BREAKPOINT_REGISTER = 3,
    KNC_BREAKPOINT_SYSTEM_CALL = 4
} knc_breakpoint_type_t;

// Breakpoint information
typedef struct {
    uint64_t address;
    knc_breakpoint_type_t type;
    uint32_t core_id;
    bool enabled;
    uint32_t hit_count;
    std::string condition;
    std::string command;
} knc_breakpoint_t;

// Debug command types
typedef enum {
    KNC_CMD_CONTINUE = 0,
    KNC_CMD_STEP = 1,
    KNC_CMD_STEP_OVER = 2,
    KNC_CMD_STEP_OUT = 3,
    KNC_CMD_BREAK = 4,
    KNC_CMD_INFO = 5,
    KNC_CMD_REGISTERS = 6,
    KNC_CMD_MEMORY = 7,
    KNC_CMD_DISASSEMBLE = 8,
    KNC_CMD_BREAKPOINT = 9,
    KNC_CMD_WATCHPOINT = 10,
    KNC_CMD_QUIT = 11
} knc_debug_command_t;

// Debug session state
typedef struct {
    bool is_debugging;
    bool is_paused;
    uint32_t current_core;
    uint64_t current_address;
    std::string last_command;
    std::vector<std::string> command_history;
} knc_debug_session_t;

class KNCDebugger {
private:
    // Debug state
    knc_debug_session_t session;
    std::atomic<bool> debugging_enabled;
    std::atomic<bool> break_requested;
    
    // Breakpoints
    std::vector<knc_breakpoint_t> breakpoints;
    std::unordered_map<uint64_t, size_t> breakpoint_index;
    std::mutex breakpoints_mutex;
    
    // Watchpoints (memory access breakpoints)
    std::vector<knc_breakpoint_t> watchpoints;
    std::unordered_map<uint64_t, size_t> watchpoint_index;
    
    // Command interface
    std::thread command_thread;
    std::mutex command_mutex;
    std::condition_variable command_cv;
    std::string pending_command;
    
    // Symbol table for debugging
    std::unordered_map<std::string, uint64_t> symbols;
    std::unordered_map<uint64_t, std::string> reverse_symbols;
    
    // Statistics
    uint64_t breakpoints_hit;
    uint64_t watchpoints_hit;
    uint64_t instructions_stepped;
    
    // Command processing
    void command_loop();
    void process_command(const std::string& command);
    std::vector<std::string> parse_command(const std::string& command);
    
    // Command implementations
    void cmd_continue(const std::vector<std::string>& args);
    void cmd_step(const std::vector<std::string>& args);
    void cmd_step_over(const std::vector<std::string>& args);
    void cmd_step_out(const std::vector<std::string>& args);
    void cmd_break(const std::vector<std::string>& args);
    void cmd_info(const std::vector<std::string>& args);
    void cmd_registers(const std::vector<std::string>& args);
    void cmd_memory(const std::vector<std::string>& args);
    void cmd_disassemble(const std::vector<std::string>& args);
    void cmd_watchpoint(const std::vector<std::string>& args);
    void cmd_quit(const std::vector<std::string>& args);
    
    // Breakpoint management
    bool add_breakpoint(uint64_t address, knc_breakpoint_type_t type, uint32_t core_mask);
    bool remove_breakpoint(uint64_t address);
    bool enable_breakpoint(uint64_t address, bool enabled);
    bool check_breakpoints(uint64_t address, uint32_t core_id);
    bool check_watchpoints(uint64_t address, size_t size, bool is_write);
    
    // Utility functions
    uint64_t parse_address(const std::string& addr_str);
    std::string format_address(uint64_t address);
    std::string format_value(uint64_t value, int size = 8);
    std::vector<std::string> split_command(const std::string& command);
    void print_help();
    void print_prompt();
    
    // Symbol resolution
    bool load_symbols(const std::string& filename);
    bool resolve_symbol(const std::string& name, uint64_t& address);
    bool resolve_address(uint64_t address, std::string& name);
    
public:
    KNCDebugger();
    ~KNCDebugger();
    
    // Initialization
    bool initialize();
    void shutdown();
    
    // Debug control
    void enable_debugging(bool enable);
    bool is_debugging_enabled() const;
    void request_break();
    void clear_break_request();
    
    // Breakpoint interface
    bool set_breakpoint(uint64_t address, const std::string& condition = "");
    bool set_watchpoint(uint64_t address, size_t size, bool is_write = true);
    void list_breakpoints();
    void clear_breakpoints();
    
    // Execution control
    bool should_break(uint64_t address, uint32_t core_id);
    bool should_pause() const;
    void notify_breakpoint_hit(uint64_t address, uint32_t core_id);
    void notify_watchpoint_hit(uint64_t address, uint32_t core_id, bool is_write);
    
    // State inspection
    void dump_registers(uint32_t core_id);
    void dump_memory(uint64_t address, size_t size);
    void dump_stack(uint32_t core_id, size_t num_words = 16);
    void disassemble_memory(uint64_t address, size_t num_instructions = 10);
    
    // Symbol interface
    bool add_symbol(const std::string& name, uint64_t address);
    bool remove_symbol(const std::string& name);
    void list_symbols();
    
    // Statistics
    void print_debug_statistics() const;
    
    // GDB compatibility
    bool handle_gdb_command(const std::string& command);
    std::string generate_gdb_response(const std::string& command);
    
    // Integration with runtime
    void set_runtime_state(void* runtime_state);
    void update_core_state(uint32_t core_id, const knc_core_state_t& state);
};

#endif // KNC_DEBUGGER_H
