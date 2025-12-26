#ifndef CPYTHON_CPP_COMPILER_CODE_OBJECT_HPP
#define CPYTHON_CPP_COMPILER_CODE_OBJECT_HPP

#include "opcode.hpp"
#include <string>
#include <vector>
#include <variant>
#include <memory>
#include <cstdint>
#include <sstream>
#include <iomanip>
#include <algorithm>

namespace cpython_cpp {
namespace compiler {

// Forward declaration
struct CodeObject;

/**
 * Python constant value
 * Represents values that can be stored in co_consts
 */
using PyConstant = std::variant<
    std::monostate,         // None
    bool,                   // True/False
    int64_t,                // Integer
    double,                 // Float
    std::string,            // String
    std::vector<uint8_t>,   // Bytes
    std::shared_ptr<CodeObject>  // Nested code object
>;

/**
 * Get string representation of a constant
 */
inline std::string constant_to_string(const PyConstant& c) {
    return std::visit([](auto&& arg) -> std::string {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, std::monostate>) {
            return "None";
        } else if constexpr (std::is_same_v<T, bool>) {
            return arg ? "True" : "False";
        } else if constexpr (std::is_same_v<T, int64_t>) {
            return std::to_string(arg);
        } else if constexpr (std::is_same_v<T, double>) {
            std::ostringstream oss;
            oss << arg;
            return oss.str();
        } else if constexpr (std::is_same_v<T, std::string>) {
            return "'" + arg + "'";
        } else if constexpr (std::is_same_v<T, std::vector<uint8_t>>) {
            std::ostringstream oss;
            oss << "b'";
            for (auto b : arg) {
                if (b >= 32 && b < 127) {
                    oss << static_cast<char>(b);
                } else {
                    oss << "\\x" << std::hex << std::setfill('0') 
                        << std::setw(2) << static_cast<int>(b);
                }
            }
            oss << "'";
            return oss.str();
        } else if constexpr (std::is_same_v<T, std::shared_ptr<CodeObject>>) {
            return "<code object " + arg->co_name + ">";
        } else {
            return "<unknown>";
        }
    }, c);
}

/**
 * Code flags (co_flags)
 * Reference: Include/cpython/code.h
 */
namespace CodeFlags {
    constexpr uint32_t CO_OPTIMIZED         = 0x0001;  // Uses fast locals
    constexpr uint32_t CO_NEWLOCALS         = 0x0002;  // New local namespace
    constexpr uint32_t CO_VARARGS           = 0x0004;  // Has *args
    constexpr uint32_t CO_VARKEYWORDS       = 0x0008;  // Has **kwargs
    constexpr uint32_t CO_NESTED            = 0x0010;  // Nested function
    constexpr uint32_t CO_GENERATOR         = 0x0020;  // Generator function
    constexpr uint32_t CO_NOFREE            = 0x0040;  // No free variables
    constexpr uint32_t CO_COROUTINE         = 0x0080;  // Coroutine function
    constexpr uint32_t CO_ITERABLE_COROUTINE = 0x0100; // Iterable coroutine
    constexpr uint32_t CO_ASYNC_GENERATOR   = 0x0200;  // Async generator
    constexpr uint32_t CO_FUTURE_DIVISION   = 0x20000; // from __future__ import division
    constexpr uint32_t CO_FUTURE_ABSOLUTE_IMPORT = 0x40000;
    constexpr uint32_t CO_FUTURE_WITH_STATEMENT = 0x80000;
    constexpr uint32_t CO_FUTURE_PRINT_FUNCTION = 0x100000;
    constexpr uint32_t CO_FUTURE_UNICODE_LITERALS = 0x200000;
    constexpr uint32_t CO_FUTURE_ANNOTATIONS = 0x1000000;
}

/**
 * CodeObject - Python code object structure
 * Reference: Include/cpython/code.h (PyCodeObject)
 * 
 * This represents compiled Python code, similar to CPython's code object.
 */
struct CodeObject {
    // === Metadata ===
    std::string co_name;            // Function/module name
    std::string co_qualname;        // Qualified name
    std::string co_filename;        // Source filename
    int co_firstlineno;             // First line number
    
    // === Code ===
    std::vector<Instruction> instructions;  // Bytecode instructions
    std::vector<uint8_t> co_code;           // Raw bytecode bytes
    
    // === Constants and Names ===
    std::vector<PyConstant> co_consts;      // Constants used in code
    std::vector<std::string> co_names;      // Names used (globals, attrs)
    std::vector<std::string> co_varnames;   // Local variable names
    std::vector<std::string> co_freevars;   // Free variable names (closures)
    std::vector<std::string> co_cellvars;   // Cell variable names (closures)
    
    // === Counts ===
    int co_argcount;        // Number of positional arguments
    int co_posonlyargcount; // Number of positional-only arguments
    int co_kwonlyargcount;  // Number of keyword-only arguments
    int co_nlocals;         // Number of local variables
    int co_stacksize;       // Maximum stack depth
    
    // === Flags ===
    uint32_t co_flags;      // Code flags (see below)
    
    // === Line Number Table ===
    std::vector<std::pair<int, int>> co_linetable;  // (offset, lineno) pairs
    
    CodeObject()
        : co_firstlineno(1)
        , co_argcount(0)
        , co_posonlyargcount(0)
        , co_kwonlyargcount(0)
        , co_nlocals(0)
        , co_stacksize(0)
        , co_flags(0) {}
    
    CodeObject(const std::string& name, const std::string& filename, int firstlineno)
        : co_name(name)
        , co_filename(filename)
        , co_firstlineno(firstlineno)
        , co_argcount(0)
        , co_posonlyargcount(0)
        , co_kwonlyargcount(0)
        , co_nlocals(0)
        , co_stacksize(0)
        , co_flags(0) {}
    
    // === Helper Methods ===
    
    /**
     * Add a constant and return its index
     */
    int add_const(const PyConstant& value) {
        // Check if constant already exists
        for (size_t i = 0; i < co_consts.size(); ++i) {
            if (constant_to_string(co_consts[i]) == constant_to_string(value)) {
                return static_cast<int>(i);
            }
        }
        co_consts.push_back(value);
        return static_cast<int>(co_consts.size() - 1);
    }
    
    /**
     * Add a name and return its index
     */
    int add_name(const std::string& name) {
        auto it = std::find(co_names.begin(), co_names.end(), name);
        if (it != co_names.end()) {
            return static_cast<int>(it - co_names.begin());
        }
        co_names.push_back(name);
        return static_cast<int>(co_names.size() - 1);
    }
    
    /**
     * Add a local variable and return its index
     */
    int add_varname(const std::string& name) {
        auto it = std::find(co_varnames.begin(), co_varnames.end(), name);
        if (it != co_varnames.end()) {
            return static_cast<int>(it - co_varnames.begin());
        }
        co_varnames.push_back(name);
        co_nlocals = static_cast<int>(co_varnames.size());
        return static_cast<int>(co_varnames.size() - 1);
    }
    
    /**
     * Find index of a local variable (-1 if not found)
     */
    int find_varname(const std::string& name) const {
        auto it = std::find(co_varnames.begin(), co_varnames.end(), name);
        if (it != co_varnames.end()) {
            return static_cast<int>(it - co_varnames.begin());
        }
        return -1;
    }
    
    /**
     * Add a free variable and return its index
     */
    int add_freevar(const std::string& name) {
        auto it = std::find(co_freevars.begin(), co_freevars.end(), name);
        if (it != co_freevars.end()) {
            return static_cast<int>(it - co_freevars.begin());
        }
        co_freevars.push_back(name);
        return static_cast<int>(co_freevars.size() - 1);
    }
    
    /**
     * Add a cell variable and return its index
     */
    int add_cellvar(const std::string& name) {
        auto it = std::find(co_cellvars.begin(), co_cellvars.end(), name);
        if (it != co_cellvars.end()) {
            return static_cast<int>(it - co_cellvars.begin());
        }
        co_cellvars.push_back(name);
        return static_cast<int>(co_cellvars.size() - 1);
    }
    
    /**
     * Emit an instruction
     */
    void emit(Opcode op, int arg = -1, int lineno = 0) {
        Instruction instr(op, arg, lineno);
        instr.offset = current_offset();
        instructions.push_back(instr);
    }
    
    /**
     * Get current instruction offset
     */
    int current_offset() const {
        int offset = 0;
        for (const auto& instr : instructions) {
            offset += 2;  // Base instruction size
            if (instr.arg > 0xFF) {
                offset += 2;
                if (instr.arg > 0xFFFF) {
                    offset += 2;
                    if (instr.arg > 0xFFFFFF) {
                        offset += 2;
                    }
                }
            }
        }
        return offset;
    }
    
    /**
     * Assemble instructions into raw bytecode
     */
    void assemble() {
        co_code.clear();
        
        for (const auto& instr : instructions) {
            int arg = instr.arg >= 0 ? instr.arg : 0;
            
            // Handle EXTENDED_ARG for large arguments
            if (arg > 0xFFFFFF) {
                co_code.push_back(static_cast<uint8_t>(Opcode::EXTENDED_ARG));
                co_code.push_back((arg >> 24) & 0xFF);
            }
            if (arg > 0xFFFF) {
                co_code.push_back(static_cast<uint8_t>(Opcode::EXTENDED_ARG));
                co_code.push_back((arg >> 16) & 0xFF);
            }
            if (arg > 0xFF) {
                co_code.push_back(static_cast<uint8_t>(Opcode::EXTENDED_ARG));
                co_code.push_back((arg >> 8) & 0xFF);
            }
            
            // Emit the actual instruction
            co_code.push_back(static_cast<uint8_t>(instr.opcode));
            co_code.push_back(arg & 0xFF);
        }
    }
    
    /**
     * Calculate required stack size
     */
    void calculate_stacksize() {
        int depth = 0;
        int max_depth = 0;
        
        for (const auto& instr : instructions) {
            int effect = opcode_stack_effect(instr.opcode, instr.arg >= 0 ? instr.arg : 0);
            depth += effect;
            if (depth > max_depth) {
                max_depth = depth;
            }
            if (depth < 0) {
                depth = 0;
            }
        }
        
        co_stacksize = max_depth + 1;
    }
    
    /**
     * Disassemble to string (like dis.dis())
     */
    std::string disassemble() const {
        std::ostringstream oss;
        
        // Header
        oss << "Disassembly of " << co_name << ":\n";
        oss << "  co_argcount: " << co_argcount << "\n";
        oss << "  co_nlocals: " << co_nlocals << "\n";
        oss << "  co_stacksize: " << co_stacksize << "\n";
        oss << "  co_flags: 0x" << std::hex << co_flags << std::dec << "\n";
        
        // Constants
        if (!co_consts.empty()) {
            oss << "  co_consts:\n";
            for (size_t i = 0; i < co_consts.size(); ++i) {
                oss << "    " << i << ": " << constant_to_string(co_consts[i]) << "\n";
            }
        }
        
        // Names
        if (!co_names.empty()) {
            oss << "  co_names: (";
            for (size_t i = 0; i < co_names.size(); ++i) {
                if (i > 0) oss << ", ";
                oss << "'" << co_names[i] << "'";
            }
            oss << ")\n";
        }
        
        // Varnames
        if (!co_varnames.empty()) {
            oss << "  co_varnames: (";
            for (size_t i = 0; i < co_varnames.size(); ++i) {
                if (i > 0) oss << ", ";
                oss << "'" << co_varnames[i] << "'";
            }
            oss << ")\n";
        }
        
        // Instructions
        int last_lineno = -1;
        int offset = 0;
        
        for (const auto& instr : instructions) {
            // Line number
            if (instr.lineno != last_lineno && instr.lineno > 0) {
                if (last_lineno != -1) {
                    oss << "\n";
                }
                oss << std::setw(4) << instr.lineno << " ";
                last_lineno = instr.lineno;
            } else {
                oss << "     ";
            }
            
            // Offset
            oss << std::setw(4) << offset << " ";
            
            // Opcode name
            oss << std::left << std::setw(24) << opcode_name(instr.opcode);
            
            // Argument
            if (instr.has_arg()) {
                oss << std::setw(5) << instr.arg;
                
                // Argument annotation
                switch (instr.opcode) {
                    case Opcode::LOAD_CONST:
                    case Opcode::LOAD_SMALL_INT:
                        if (instr.arg >= 0 && static_cast<size_t>(instr.arg) < co_consts.size()) {
                            oss << " (" << constant_to_string(co_consts[instr.arg]) << ")";
                        }
                        break;
                        
                    case Opcode::LOAD_NAME:
                    case Opcode::STORE_NAME:
                    case Opcode::DELETE_NAME:
                    case Opcode::LOAD_GLOBAL:
                    case Opcode::STORE_GLOBAL:
                    case Opcode::DELETE_GLOBAL:
                    case Opcode::LOAD_ATTR:
                    case Opcode::STORE_ATTR:
                    case Opcode::DELETE_ATTR:
                    case Opcode::IMPORT_NAME:
                    case Opcode::IMPORT_FROM:
                        if (instr.arg >= 0 && static_cast<size_t>(instr.arg) < co_names.size()) {
                            oss << " (" << co_names[instr.arg] << ")";
                        }
                        break;
                        
                    case Opcode::LOAD_FAST:
                    case Opcode::STORE_FAST:
                    case Opcode::DELETE_FAST:
                        if (instr.arg >= 0 && static_cast<size_t>(instr.arg) < co_varnames.size()) {
                            oss << " (" << co_varnames[instr.arg] << ")";
                        }
                        break;
                        
                    case Opcode::BINARY_OP:
                        {
                            const char* op_names[] = {
                                "+", "&", "//", "<<", "@", "*", "%", "|",
                                "**", ">>", "-", "/", "^",
                                "+=", "&=", "//=", "<<=", "@=", "*=", "%=", "|=",
                                "**=", ">>=", "-=", "/=", "^="
                            };
                            if (instr.arg >= 0 && instr.arg < 26) {
                                oss << " (" << op_names[instr.arg] << ")";
                            }
                        }
                        break;
                        
                    case Opcode::COMPARE_OP:
                        {
                            const char* cmp_names[] = {"<", "<=", "==", "!=", ">", ">="};
                            if (instr.arg >= 0 && instr.arg < 6) {
                                oss << " (" << cmp_names[instr.arg] << ")";
                            }
                        }
                        break;
                        
                    case Opcode::JUMP_FORWARD:
                    case Opcode::JUMP_BACKWARD:
                    case Opcode::POP_JUMP_IF_FALSE:
                    case Opcode::POP_JUMP_IF_TRUE:
                    case Opcode::FOR_ITER:
                        oss << " (to " << instr.arg << ")";
                        break;
                        
                    default:
                        break;
                }
            }
            
            oss << "\n";
            offset += 2;
        }
        
        return oss.str();
    }
};

} // namespace compiler
} // namespace cpython_cpp

#endif // CPYTHON_CPP_COMPILER_CODE_OBJECT_HPP
