#ifndef CPYTHON_CPP_COMPILER_CODE_OBJECT_HPP
#define CPYTHON_CPP_COMPILER_CODE_OBJECT_HPP

#include "opcode.hpp"
#include <string>
#include <vector>
#include <variant>
#include <memory>
#include <cstdint>

namespace cpython_cpp {
namespace compiler {

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
    std::shared_ptr<struct CodeObject>  // Nested code object
>;

/**
 * Get string representation of a constant
 */
std::string constant_to_string(const PyConstant& c);

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
    
    // === Helper Methods ===
    
    /**
     * Add a constant and return its index
     */
    int add_const(const PyConstant& value);
    
    /**
     * Add a name and return its index
     */
    int add_name(const std::string& name);
    
    /**
     * Add a local variable and return its index
     */
    int add_varname(const std::string& name);
    
    /**
     * Find index of a local variable (-1 if not found)
     */
    int find_varname(const std::string& name) const;
    
    /**
     * Add a free variable and return its index
     */
    int add_freevar(const std::string& name);
    
    /**
     * Add a cell variable and return its index
     */
    int add_cellvar(const std::string& name);
    
    /**
     * Emit an instruction
     */
    void emit(Opcode op, int arg = -1, int lineno = 0);
    
    /**
     * Get current instruction offset
     */
    int current_offset() const;
    
    /**
     * Assemble instructions into raw bytecode
     */
    void assemble();
    
    /**
     * Calculate required stack size
     */
    void calculate_stacksize();
    
    /**
     * Disassemble to string (like dis.dis())
     */
    std::string disassemble() const;
};

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

} // namespace compiler
} // namespace cpython_cpp

#endif // CPYTHON_CPP_COMPILER_CODE_OBJECT_HPP
