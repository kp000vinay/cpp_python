#ifndef CPYTHON_CPP_COMPILER_OPCODE_HPP
#define CPYTHON_CPP_COMPILER_OPCODE_HPP

#include <cstdint>
#include <string>
#include <sstream>

namespace cpython_cpp {
namespace compiler {

/**
 * Python Bytecode Opcodes
 * Reference: CPython Include/opcode_ids.h (Python 3.12+)
 * 
 * This defines the instruction set for the bytecode compiler.
 * Opcodes are designed to be compatible with CPython's bytecode format.
 */

enum class Opcode : uint8_t {
    // === Stack Operations ===
    POP_TOP                 = 31,   // Pop top of stack
    PUSH_NULL               = 33,   // Push NULL onto stack
    SWAP                    = 117,  // Swap top two stack items
    COPY                    = 59,   // Copy item from stack position
    
    // === Constants ===
    LOAD_CONST              = 82,   // Load constant from co_consts
    LOAD_SMALL_INT          = 94,   // Load small integer (0-255) directly
    
    // === Local Variables ===
    LOAD_FAST               = 84,   // Load local variable
    STORE_FAST              = 112,  // Store local variable
    DELETE_FAST             = 63,   // Delete local variable
    LOAD_FAST_CHECK         = 88,   // Load local with unbound check
    LOAD_FAST_AND_CLEAR     = 85,   // Load and clear local
    LOAD_FAST_LOAD_FAST     = 89,   // Load two locals (optimization)
    STORE_FAST_LOAD_FAST    = 113,  // Store then load (optimization)
    STORE_FAST_STORE_FAST   = 114,  // Store two locals (optimization)
    
    // === Python 3.14+ Local Variable Optimizations ===
    LOAD_FAST_BORROW        = 86,   // Load local without incref (3.14+)
    LOAD_FAST_BORROW_LOAD_FAST_BORROW = 87, // Load two locals without incref (3.14+)
    
    // === Global/Name Variables ===
    LOAD_NAME               = 93,   // Load name from namespace
    STORE_NAME              = 116,  // Store name in namespace
    DELETE_NAME             = 65,   // Delete name from namespace
    LOAD_GLOBAL             = 92,   // Load global variable
    STORE_GLOBAL            = 115,  // Store global variable
    DELETE_GLOBAL           = 64,   // Delete global variable
    
    // === Closure Variables ===
    LOAD_DEREF              = 83,   // Load from closure cell
    STORE_DEREF             = 111,  // Store to closure cell
    DELETE_DEREF            = 62,   // Delete from closure cell
    MAKE_CELL               = 97,   // Create cell for closure
    COPY_FREE_VARS          = 60,   // Copy free variables
    LOAD_FROM_DICT_OR_DEREF = 90,   // Load from dict or closure
    LOAD_FROM_DICT_OR_GLOBALS = 91, // Load from dict or globals
    
    // === Attributes ===
    LOAD_ATTR               = 80,   // Load attribute
    STORE_ATTR              = 110,  // Store attribute
    DELETE_ATTR             = 61,   // Delete attribute
    LOAD_SUPER_ATTR         = 96,   // Load super attribute
    
    // === Subscript/Slice ===
    BINARY_SLICE            = 1,    // Binary slice operation
    STORE_SLICE             = 37,   // Store slice
    STORE_SUBSCR            = 38,   // Store subscript (obj[key] = value)
    DELETE_SUBSCR           = 8,    // Delete subscript
    
    // === Binary Operations ===
    BINARY_OP               = 44,   // Generic binary operation
    
    // === Unary Operations ===
    UNARY_NEGATIVE          = 41,   // Negate (-x)
    UNARY_NOT               = 42,   // Boolean not (not x)
    UNARY_INVERT            = 40,   // Bitwise invert (~x)
    TO_BOOL                 = 39,   // Convert to boolean
    
    // === Comparison ===
    COMPARE_OP              = 56,   // Comparison operation
    IS_OP                   = 74,   // Identity check (is/is not)
    CONTAINS_OP             = 57,   // Membership check (in/not in)
    
    // === Control Flow - Jumps ===
    NOT_TAKEN               = 28,   // Branch not taken hint (3.14+)
    JUMP_FORWARD            = 77,   // Jump forward (unconditional)
    JUMP_BACKWARD           = 75,   // Jump backward (unconditional)
    JUMP_BACKWARD_NO_INTERRUPT = 76, // Jump backward without interrupt check
    POP_JUMP_IF_FALSE       = 100,  // Pop and jump if false
    POP_JUMP_IF_TRUE        = 103,  // Pop and jump if true
    POP_JUMP_IF_NONE        = 101,  // Pop and jump if None
    POP_JUMP_IF_NOT_NONE    = 102,  // Pop and jump if not None
    
    // === Iteration ===
    GET_ITER                = 16,   // Get iterator from iterable
    GET_AITER               = 14,   // Get async iterator
    GET_ANEXT               = 15,   // Get next from async iterator
    FOR_ITER                = 70,   // Iterate (next or jump)
    END_FOR                 = 9,    // End for loop cleanup
    POP_ITER                = 30,   // Pop iterator from stack
    GET_YIELD_FROM_ITER     = 19,   // Get yield from iterator
    
    // === Function/Call ===
    CALL                    = 52,   // Call function
    CALL_FUNCTION_EX        = 4,    // Call with *args/**kwargs
    CALL_KW                 = 55,   // Call with keyword arguments
    RETURN_VALUE            = 35,   // Return from function
    RETURN_GENERATOR        = 34,   // Return generator object
    MAKE_FUNCTION           = 23,   // Create function object
    SET_FUNCTION_ATTRIBUTE  = 108,  // Set function attribute
    LOAD_BUILD_CLASS        = 21,   // Load __build_class__
    
    // === Building Objects ===
    BUILD_LIST              = 46,   // Build list from stack items
    BUILD_TUPLE             = 51,   // Build tuple from stack items
    BUILD_SET               = 48,   // Build set from stack items
    BUILD_MAP               = 47,   // Build dict from stack items
    BUILD_STRING            = 50,   // Build string (f-strings)
    BUILD_SLICE             = 49,   // Build slice object
    BUILD_TEMPLATE          = 2,    // Build template (t-strings)
    BUILD_INTERPOLATION     = 45,   // Build interpolation (t-strings)
    
    // === List/Set/Dict Operations ===
    LIST_APPEND             = 78,   // Append to list (comprehensions)
    LIST_EXTEND             = 79,   // Extend list
    SET_ADD                 = 107,  // Add to set (comprehensions)
    SET_UPDATE              = 109,  // Update set
    MAP_ADD                 = 98,   // Add to map (dict comprehensions)
    DICT_MERGE              = 66,   // Merge dicts
    DICT_UPDATE             = 67,   // Update dict
    
    // === Exception Handling ===
    PUSH_EXC_INFO           = 32,   // Push exception info
    POP_EXCEPT              = 29,   // Pop exception handler
    RAISE_VARARGS           = 104,  // Raise exception
    RERAISE                 = 105,  // Re-raise current exception
    CHECK_EXC_MATCH         = 6,    // Check exception match
    CHECK_EG_MATCH          = 5,    // Check exception group match
    CLEANUP_THROW           = 7,    // Cleanup after throw
    WITH_EXCEPT_START       = 43,   // Start with exception handling
    BEFORE_WITH             = 3,    // Setup context manager (calls __enter__)
    END_ASYNC_FOR           = 68,   // End async for loop
    BEFORE_ASYNC_WITH       = 17,   // Setup async context manager (calls __aenter__)
    
    // === Unpacking ===
    UNPACK_SEQUENCE         = 119,  // Unpack sequence to stack
    UNPACK_EX               = 118,  // Unpack with starred element
    GET_LEN                 = 18,   // Get length for unpacking
    
    // === Import ===
    IMPORT_NAME             = 73,   // Import module
    IMPORT_FROM             = 72,   // Import from module
    
    // === Pattern Matching ===
    MATCH_MAPPING           = 25,   // Match mapping pattern
    MATCH_SEQUENCE          = 26,   // Match sequence pattern
    MATCH_KEYS              = 24,   // Match dict keys
    MATCH_CLASS             = 99,   // Match class pattern
    
    // === Async/Await ===
    GET_AWAITABLE           = 71,   // Get awaitable
    SEND                    = 106,  // Send to generator/coroutine
    YIELD_VALUE             = 120,  // Yield value
    END_SEND                = 10,   // End send operation
    
    // === Format ===
    FORMAT_SIMPLE           = 12,   // Simple format
    FORMAT_WITH_SPEC        = 13,   // Format with spec
    CONVERT_VALUE           = 58,   // Convert value (!s, !r, !a)
    
    // === Misc ===
    NOP                     = 27,   // No operation
    EXTENDED_ARG            = 69,   // Extended argument prefix
    RESUME                  = 128,  // Resume execution
    CACHE                   = 0,    // Cache slot (internal)
    SETUP_ANNOTATIONS       = 36,   // Setup __annotations__
    LOAD_LOCALS             = 22,   // Load locals dict
    LOAD_SPECIAL            = 95,   // Load special method
    LOAD_COMMON_CONSTANT    = 81,   // Load common constant
    CALL_INTRINSIC_1        = 53,   // Call intrinsic (1 arg)
    CALL_INTRINSIC_2        = 54,   // Call intrinsic (2 args)
    INTERPRETER_EXIT        = 20,   // Exit interpreter
    EXIT_INIT_CHECK         = 11,   // Check __init__ exit
};

/**
 * Binary operation sub-opcodes for BINARY_OP
 * These are passed as the argument to BINARY_OP
 */
enum class BinaryOpCode : uint8_t {
    NB_ADD              = 0,
    NB_AND              = 1,
    NB_FLOOR_DIVIDE     = 2,
    NB_LSHIFT           = 3,
    NB_MATRIX_MULTIPLY  = 4,
    NB_MULTIPLY         = 5,
    NB_REMAINDER        = 6,
    NB_OR               = 7,
    NB_POWER            = 8,
    NB_RSHIFT           = 9,
    NB_SUBTRACT         = 10,
    NB_TRUE_DIVIDE      = 11,
    NB_XOR              = 12,
    // Inplace variants
    NB_INPLACE_ADD              = 13,
    NB_INPLACE_AND              = 14,
    NB_INPLACE_FLOOR_DIVIDE     = 15,
    NB_INPLACE_LSHIFT           = 16,
    NB_INPLACE_MATRIX_MULTIPLY  = 17,
    NB_INPLACE_MULTIPLY         = 18,
    NB_INPLACE_REMAINDER        = 19,
    NB_INPLACE_OR               = 20,
    NB_INPLACE_POWER            = 21,
    NB_INPLACE_RSHIFT           = 22,
    NB_INPLACE_SUBTRACT         = 23,
    NB_INPLACE_TRUE_DIVIDE      = 24,
    NB_INPLACE_XOR              = 25,
};

/**
 * Comparison operation sub-opcodes for COMPARE_OP
 */
enum class CompareOpCode : uint8_t {
    LT = 0,   // <
    LE = 1,   // <=
    EQ = 2,   // ==
    NE = 3,   // !=
    GT = 4,   // >
    GE = 5,   // >=
};

// Forward declarations
inline const char* opcode_name(Opcode op);
inline bool opcode_has_arg(Opcode op);
inline bool opcode_is_jump(Opcode op);
inline bool opcode_is_relative_jump(Opcode op);
inline int opcode_stack_effect(Opcode op, int arg = 0);

/**
 * Instruction - represents a single bytecode instruction
 */
struct Instruction {
    Opcode opcode;
    int32_t arg;        // Argument (if any), -1 if no argument
    int lineno;         // Source line number
    int offset;         // Byte offset in bytecode
    
    Instruction(Opcode op, int32_t a = -1, int line = 0)
        : opcode(op), arg(a), lineno(line), offset(0) {}
    
    bool has_arg() const {
        return arg >= 0;
    }
    
    std::string to_string() const {
        std::ostringstream oss;
        oss << offset << "\t" << opcode_name(opcode);
        if (has_arg()) {
            oss << "\t" << arg;
        }
        return oss.str();
    }
};

/**
 * Get the name of an opcode
 */
inline const char* opcode_name(Opcode op) {
    switch (op) {
        // Stack Operations
        case Opcode::POP_TOP: return "POP_TOP";
        case Opcode::PUSH_NULL: return "PUSH_NULL";
        case Opcode::SWAP: return "SWAP";
        case Opcode::COPY: return "COPY";
        
        // Constants
        case Opcode::LOAD_CONST: return "LOAD_CONST";
        case Opcode::LOAD_SMALL_INT: return "LOAD_SMALL_INT";
        
        // Local Variables
        case Opcode::LOAD_FAST: return "LOAD_FAST";
        case Opcode::STORE_FAST: return "STORE_FAST";
        case Opcode::DELETE_FAST: return "DELETE_FAST";
        case Opcode::LOAD_FAST_CHECK: return "LOAD_FAST_CHECK";
        case Opcode::LOAD_FAST_AND_CLEAR: return "LOAD_FAST_AND_CLEAR";
        case Opcode::LOAD_FAST_LOAD_FAST: return "LOAD_FAST_LOAD_FAST";
        case Opcode::STORE_FAST_LOAD_FAST: return "STORE_FAST_LOAD_FAST";
        case Opcode::STORE_FAST_STORE_FAST: return "STORE_FAST_STORE_FAST";
        case Opcode::LOAD_FAST_BORROW: return "LOAD_FAST_BORROW";
        case Opcode::LOAD_FAST_BORROW_LOAD_FAST_BORROW: return "LOAD_FAST_BORROW_LOAD_FAST_BORROW";
        
        // Global/Name Variables
        case Opcode::LOAD_NAME: return "LOAD_NAME";
        case Opcode::STORE_NAME: return "STORE_NAME";
        case Opcode::DELETE_NAME: return "DELETE_NAME";
        case Opcode::LOAD_GLOBAL: return "LOAD_GLOBAL";
        case Opcode::STORE_GLOBAL: return "STORE_GLOBAL";
        case Opcode::DELETE_GLOBAL: return "DELETE_GLOBAL";
        
        // Closure Variables
        case Opcode::LOAD_DEREF: return "LOAD_DEREF";
        case Opcode::STORE_DEREF: return "STORE_DEREF";
        case Opcode::DELETE_DEREF: return "DELETE_DEREF";
        case Opcode::MAKE_CELL: return "MAKE_CELL";
        case Opcode::COPY_FREE_VARS: return "COPY_FREE_VARS";
        case Opcode::LOAD_FROM_DICT_OR_DEREF: return "LOAD_FROM_DICT_OR_DEREF";
        case Opcode::LOAD_FROM_DICT_OR_GLOBALS: return "LOAD_FROM_DICT_OR_GLOBALS";
        
        // Attributes
        case Opcode::LOAD_ATTR: return "LOAD_ATTR";
        case Opcode::STORE_ATTR: return "STORE_ATTR";
        case Opcode::DELETE_ATTR: return "DELETE_ATTR";
        case Opcode::LOAD_SUPER_ATTR: return "LOAD_SUPER_ATTR";
        
        // Subscript/Slice
        case Opcode::BINARY_SLICE: return "BINARY_SLICE";
        case Opcode::STORE_SLICE: return "STORE_SLICE";
        case Opcode::STORE_SUBSCR: return "STORE_SUBSCR";
        case Opcode::DELETE_SUBSCR: return "DELETE_SUBSCR";
        
        // Binary Operations
        case Opcode::BINARY_OP: return "BINARY_OP";
        
        // Unary Operations
        case Opcode::UNARY_NEGATIVE: return "UNARY_NEGATIVE";
        case Opcode::UNARY_NOT: return "UNARY_NOT";
        case Opcode::UNARY_INVERT: return "UNARY_INVERT";
        case Opcode::TO_BOOL: return "TO_BOOL";
        
        // Comparison
        case Opcode::COMPARE_OP: return "COMPARE_OP";
        case Opcode::IS_OP: return "IS_OP";
        case Opcode::CONTAINS_OP: return "CONTAINS_OP";
        
        // Control Flow - Jumps
        case Opcode::NOT_TAKEN: return "NOT_TAKEN";
        case Opcode::JUMP_FORWARD: return "JUMP_FORWARD";
        case Opcode::JUMP_BACKWARD: return "JUMP_BACKWARD";
        case Opcode::JUMP_BACKWARD_NO_INTERRUPT: return "JUMP_BACKWARD_NO_INTERRUPT";
        case Opcode::POP_JUMP_IF_FALSE: return "POP_JUMP_IF_FALSE";
        case Opcode::POP_JUMP_IF_TRUE: return "POP_JUMP_IF_TRUE";
        case Opcode::POP_JUMP_IF_NONE: return "POP_JUMP_IF_NONE";
        case Opcode::POP_JUMP_IF_NOT_NONE: return "POP_JUMP_IF_NOT_NONE";
        
        // Iteration
        case Opcode::GET_ITER: return "GET_ITER";
        case Opcode::GET_AITER: return "GET_AITER";
        case Opcode::GET_ANEXT: return "GET_ANEXT";
        case Opcode::FOR_ITER: return "FOR_ITER";
        case Opcode::END_FOR: return "END_FOR";
        case Opcode::POP_ITER: return "POP_ITER";
        case Opcode::GET_YIELD_FROM_ITER: return "GET_YIELD_FROM_ITER";
        
        // Function/Call
        case Opcode::CALL: return "CALL";
        case Opcode::CALL_FUNCTION_EX: return "CALL_FUNCTION_EX";
        case Opcode::CALL_KW: return "CALL_KW";
        case Opcode::RETURN_VALUE: return "RETURN_VALUE";
        case Opcode::RETURN_GENERATOR: return "RETURN_GENERATOR";
        case Opcode::MAKE_FUNCTION: return "MAKE_FUNCTION";
        case Opcode::SET_FUNCTION_ATTRIBUTE: return "SET_FUNCTION_ATTRIBUTE";
        case Opcode::LOAD_BUILD_CLASS: return "LOAD_BUILD_CLASS";
        
        // Building Objects
        case Opcode::BUILD_LIST: return "BUILD_LIST";
        case Opcode::BUILD_TUPLE: return "BUILD_TUPLE";
        case Opcode::BUILD_SET: return "BUILD_SET";
        case Opcode::BUILD_MAP: return "BUILD_MAP";
        case Opcode::BUILD_STRING: return "BUILD_STRING";
        case Opcode::BUILD_SLICE: return "BUILD_SLICE";
        case Opcode::BUILD_TEMPLATE: return "BUILD_TEMPLATE";
        case Opcode::BUILD_INTERPOLATION: return "BUILD_INTERPOLATION";
        
        // List/Set/Dict Operations
        case Opcode::LIST_APPEND: return "LIST_APPEND";
        case Opcode::LIST_EXTEND: return "LIST_EXTEND";
        case Opcode::SET_ADD: return "SET_ADD";
        case Opcode::SET_UPDATE: return "SET_UPDATE";
        case Opcode::MAP_ADD: return "MAP_ADD";
        case Opcode::DICT_MERGE: return "DICT_MERGE";
        case Opcode::DICT_UPDATE: return "DICT_UPDATE";
        
        // Exception Handling
        case Opcode::PUSH_EXC_INFO: return "PUSH_EXC_INFO";
        case Opcode::POP_EXCEPT: return "POP_EXCEPT";
        case Opcode::RAISE_VARARGS: return "RAISE_VARARGS";
        case Opcode::RERAISE: return "RERAISE";
        case Opcode::CHECK_EXC_MATCH: return "CHECK_EXC_MATCH";
        case Opcode::CHECK_EG_MATCH: return "CHECK_EG_MATCH";
        case Opcode::CLEANUP_THROW: return "CLEANUP_THROW";
        case Opcode::WITH_EXCEPT_START: return "WITH_EXCEPT_START";
        case Opcode::BEFORE_WITH: return "BEFORE_WITH";
        case Opcode::BEFORE_ASYNC_WITH: return "BEFORE_ASYNC_WITH";
        case Opcode::END_ASYNC_FOR: return "END_ASYNC_FOR";
        
        // Unpacking
        case Opcode::UNPACK_SEQUENCE: return "UNPACK_SEQUENCE";
        case Opcode::UNPACK_EX: return "UNPACK_EX";
        case Opcode::GET_LEN: return "GET_LEN";
        
        // Import
        case Opcode::IMPORT_NAME: return "IMPORT_NAME";
        case Opcode::IMPORT_FROM: return "IMPORT_FROM";
        
        // Pattern Matching
        case Opcode::MATCH_MAPPING: return "MATCH_MAPPING";
        case Opcode::MATCH_SEQUENCE: return "MATCH_SEQUENCE";
        case Opcode::MATCH_KEYS: return "MATCH_KEYS";
        case Opcode::MATCH_CLASS: return "MATCH_CLASS";
        
        // Async/Await
        case Opcode::GET_AWAITABLE: return "GET_AWAITABLE";
        case Opcode::SEND: return "SEND";
        case Opcode::YIELD_VALUE: return "YIELD_VALUE";
        case Opcode::END_SEND: return "END_SEND";
        
        // Format
        case Opcode::FORMAT_SIMPLE: return "FORMAT_SIMPLE";
        case Opcode::FORMAT_WITH_SPEC: return "FORMAT_WITH_SPEC";
        case Opcode::CONVERT_VALUE: return "CONVERT_VALUE";
        
        // Misc
        case Opcode::NOP: return "NOP";
        case Opcode::EXTENDED_ARG: return "EXTENDED_ARG";
        case Opcode::RESUME: return "RESUME";
        case Opcode::CACHE: return "CACHE";
        case Opcode::SETUP_ANNOTATIONS: return "SETUP_ANNOTATIONS";
        case Opcode::LOAD_LOCALS: return "LOAD_LOCALS";
        case Opcode::LOAD_SPECIAL: return "LOAD_SPECIAL";
        case Opcode::LOAD_COMMON_CONSTANT: return "LOAD_COMMON_CONSTANT";
        case Opcode::CALL_INTRINSIC_1: return "CALL_INTRINSIC_1";
        case Opcode::CALL_INTRINSIC_2: return "CALL_INTRINSIC_2";
        case Opcode::INTERPRETER_EXIT: return "INTERPRETER_EXIT";
        case Opcode::EXIT_INIT_CHECK: return "EXIT_INIT_CHECK";
        
        default: return "UNKNOWN";
    }
}

/**
 * Check if opcode has an argument
 */
inline bool opcode_has_arg(Opcode op) {
    switch (op) {
        case Opcode::POP_TOP:
        case Opcode::PUSH_NULL:
        case Opcode::UNARY_NEGATIVE:
        case Opcode::UNARY_NOT:
        case Opcode::UNARY_INVERT:
        case Opcode::RETURN_VALUE:
        case Opcode::GET_ITER:
        case Opcode::NOP:
        case Opcode::BINARY_SLICE:
        case Opcode::STORE_SLICE:
        case Opcode::STORE_SUBSCR:
        case Opcode::DELETE_SUBSCR:
        case Opcode::END_FOR:
        case Opcode::POP_ITER:
        case Opcode::PUSH_EXC_INFO:
        case Opcode::POP_EXCEPT:
        case Opcode::RERAISE:
        case Opcode::CHECK_EXC_MATCH:
        case Opcode::CHECK_EG_MATCH:
        case Opcode::TO_BOOL:
        case Opcode::END_SEND:
        case Opcode::NOT_TAKEN:
            return false;
        default:
            return true;
    }
}

/**
 * Check if opcode is a jump instruction
 */
inline bool opcode_is_jump(Opcode op) {
    switch (op) {
        case Opcode::JUMP_FORWARD:
        case Opcode::JUMP_BACKWARD:
        case Opcode::JUMP_BACKWARD_NO_INTERRUPT:
        case Opcode::POP_JUMP_IF_FALSE:
        case Opcode::POP_JUMP_IF_TRUE:
        case Opcode::POP_JUMP_IF_NONE:
        case Opcode::POP_JUMP_IF_NOT_NONE:
        case Opcode::FOR_ITER:
        case Opcode::SEND:
            return true;
        default:
            return false;
    }
}

/**
 * Check if opcode is a relative jump
 */
inline bool opcode_is_relative_jump(Opcode op) {
    switch (op) {
        case Opcode::JUMP_FORWARD:
        case Opcode::JUMP_BACKWARD:
        case Opcode::JUMP_BACKWARD_NO_INTERRUPT:
        case Opcode::FOR_ITER:
        case Opcode::SEND:
            return true;
        default:
            return false;
    }
}

/**
 * Get stack effect of an opcode
 */
inline int opcode_stack_effect(Opcode op, int arg) {
    switch (op) {
        // Stack Operations
        case Opcode::POP_TOP: return -1;
        case Opcode::PUSH_NULL: return 1;
        case Opcode::SWAP: return 0;
        case Opcode::COPY: return 1;
        
        // Constants
        case Opcode::LOAD_CONST: return 1;
        case Opcode::LOAD_SMALL_INT: return 1;
        
        // Local Variables
        case Opcode::LOAD_FAST: return 1;
        case Opcode::STORE_FAST: return -1;
        case Opcode::DELETE_FAST: return 0;
        case Opcode::LOAD_FAST_CHECK: return 1;
        case Opcode::LOAD_FAST_AND_CLEAR: return 1;
        case Opcode::LOAD_FAST_LOAD_FAST: return 2;
        case Opcode::STORE_FAST_LOAD_FAST: return 0;
        case Opcode::STORE_FAST_STORE_FAST: return -2;
        case Opcode::LOAD_FAST_BORROW: return 1;
        case Opcode::LOAD_FAST_BORROW_LOAD_FAST_BORROW: return 2;
        
        // Global/Name Variables
        case Opcode::LOAD_NAME: return 1;
        case Opcode::STORE_NAME: return -1;
        case Opcode::DELETE_NAME: return 0;
        case Opcode::LOAD_GLOBAL: return 1;
        case Opcode::STORE_GLOBAL: return -1;
        case Opcode::DELETE_GLOBAL: return 0;
        
        // Closure Variables
        case Opcode::LOAD_DEREF: return 1;
        case Opcode::STORE_DEREF: return -1;
        case Opcode::DELETE_DEREF: return 0;
        case Opcode::MAKE_CELL: return 0;
        case Opcode::COPY_FREE_VARS: return 0;
        
        // Attributes
        case Opcode::LOAD_ATTR: return 0;
        case Opcode::STORE_ATTR: return -2;
        case Opcode::DELETE_ATTR: return -1;
        
        // Subscript/Slice
        case Opcode::BINARY_SLICE: return -2;
        case Opcode::STORE_SLICE: return -4;
        case Opcode::STORE_SUBSCR: return -3;
        case Opcode::DELETE_SUBSCR: return -2;
        
        // Binary Operations
        case Opcode::BINARY_OP: return -1;
        
        // Unary Operations
        case Opcode::UNARY_NEGATIVE: return 0;
        case Opcode::UNARY_NOT: return 0;
        case Opcode::UNARY_INVERT: return 0;
        case Opcode::TO_BOOL: return 0;
        
        // Comparison
        case Opcode::COMPARE_OP: return -1;
        case Opcode::IS_OP: return -1;
        case Opcode::CONTAINS_OP: return -1;
        
        // Control Flow - Jumps
        case Opcode::NOT_TAKEN: return 0;
        case Opcode::JUMP_FORWARD: return 0;
        case Opcode::JUMP_BACKWARD: return 0;
        case Opcode::JUMP_BACKWARD_NO_INTERRUPT: return 0;
        case Opcode::POP_JUMP_IF_FALSE: return -1;
        case Opcode::POP_JUMP_IF_TRUE: return -1;
        case Opcode::POP_JUMP_IF_NONE: return -1;
        case Opcode::POP_JUMP_IF_NOT_NONE: return -1;
        
        // Iteration
        case Opcode::GET_ITER: return 0;
        case Opcode::FOR_ITER: return 1;
        case Opcode::END_FOR: return -1;
        case Opcode::POP_ITER: return -1;
        
        // Function/Call
        case Opcode::CALL: return -arg;
        case Opcode::CALL_FUNCTION_EX: return -1;
        case Opcode::RETURN_VALUE: return -1;
        case Opcode::MAKE_FUNCTION: return 0;
        
        // Building Objects
        case Opcode::BUILD_LIST: return 1 - arg;
        case Opcode::BUILD_TUPLE: return 1 - arg;
        case Opcode::BUILD_SET: return 1 - arg;
        case Opcode::BUILD_MAP: return 1 - 2 * arg;
        case Opcode::BUILD_STRING: return 1 - arg;
        case Opcode::BUILD_SLICE: return -1;
        
        // List/Set/Dict Operations
        case Opcode::LIST_APPEND: return -1;
        case Opcode::LIST_EXTEND: return -1;
        case Opcode::SET_ADD: return -1;
        case Opcode::MAP_ADD: return -2;
        
        // Exception Handling
        case Opcode::PUSH_EXC_INFO: return 1;
        case Opcode::POP_EXCEPT: return -1;
        case Opcode::RAISE_VARARGS: return -arg;
        case Opcode::RERAISE: return -1;
        case Opcode::CHECK_EXC_MATCH: return 0;
        case Opcode::WITH_EXCEPT_START: return 1;
        case Opcode::BEFORE_WITH: return 1;  // Pushes __exit__ method
        case Opcode::BEFORE_ASYNC_WITH: return 1;  // Pushes __aexit__ method
        
        // Unpacking
        case Opcode::UNPACK_SEQUENCE: return arg - 1;
        case Opcode::UNPACK_EX: return (arg & 0xFF) + (arg >> 8);
        
        // Import
        case Opcode::IMPORT_NAME: return -1;
        case Opcode::IMPORT_FROM: return 1;
        
        // Async/Await
        case Opcode::YIELD_VALUE: return 0;
        
        // Misc
        case Opcode::NOP: return 0;
        case Opcode::EXTENDED_ARG: return 0;
        case Opcode::RESUME: return 0;
        
        default: return 0;
    }
}

} // namespace compiler
} // namespace cpython_cpp

#endif // CPYTHON_CPP_COMPILER_OPCODE_HPP
