#include "opcode.hpp"
#include <sstream>

namespace cpython_cpp {
namespace compiler {

const char* opcode_name(Opcode op) {
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

bool opcode_has_arg(Opcode op) {
    // In Python 3.6+, all instructions are 2 bytes
    // Only a few opcodes don't use their argument
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
            return false;
        default:
            return true;
    }
}

bool opcode_is_jump(Opcode op) {
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

bool opcode_is_relative_jump(Opcode op) {
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

int opcode_stack_effect(Opcode op, int arg) {
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
        
        // Global/Name Variables
        case Opcode::LOAD_NAME: return 1;
        case Opcode::STORE_NAME: return -1;
        case Opcode::DELETE_NAME: return 0;
        case Opcode::LOAD_GLOBAL: return 1;  // Actually pushes 2 in 3.11+
        case Opcode::STORE_GLOBAL: return -1;
        case Opcode::DELETE_GLOBAL: return 0;
        
        // Closure Variables
        case Opcode::LOAD_DEREF: return 1;
        case Opcode::STORE_DEREF: return -1;
        case Opcode::DELETE_DEREF: return 0;
        case Opcode::MAKE_CELL: return 0;
        case Opcode::COPY_FREE_VARS: return 0;
        
        // Attributes
        case Opcode::LOAD_ATTR: return 0;  // Pops obj, pushes attr (or method+self)
        case Opcode::STORE_ATTR: return -2;  // Pops value and obj
        case Opcode::DELETE_ATTR: return -1;
        
        // Subscript/Slice
        case Opcode::BINARY_SLICE: return -2;  // Pops obj, start, stop; pushes result
        case Opcode::STORE_SLICE: return -4;
        case Opcode::STORE_SUBSCR: return -3;  // Pops value, obj, key
        case Opcode::DELETE_SUBSCR: return -2;
        
        // Binary Operations
        case Opcode::BINARY_OP: return -1;  // Pops 2, pushes 1
        
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
        case Opcode::JUMP_FORWARD: return 0;
        case Opcode::JUMP_BACKWARD: return 0;
        case Opcode::JUMP_BACKWARD_NO_INTERRUPT: return 0;
        case Opcode::POP_JUMP_IF_FALSE: return -1;
        case Opcode::POP_JUMP_IF_TRUE: return -1;
        case Opcode::POP_JUMP_IF_NONE: return -1;
        case Opcode::POP_JUMP_IF_NOT_NONE: return -1;
        
        // Iteration
        case Opcode::GET_ITER: return 0;
        case Opcode::FOR_ITER: return 1;  // Pushes next value (or jumps)
        case Opcode::END_FOR: return -1;
        case Opcode::POP_ITER: return -1;
        
        // Function/Call
        case Opcode::CALL: return -arg;  // Pops func + args, pushes result
        case Opcode::CALL_FUNCTION_EX: return -1;  // Complex
        case Opcode::RETURN_VALUE: return -1;
        case Opcode::MAKE_FUNCTION: return 0;
        
        // Building Objects
        case Opcode::BUILD_LIST: return 1 - arg;
        case Opcode::BUILD_TUPLE: return 1 - arg;
        case Opcode::BUILD_SET: return 1 - arg;
        case Opcode::BUILD_MAP: return 1 - 2 * arg;
        case Opcode::BUILD_STRING: return 1 - arg;
        case Opcode::BUILD_SLICE: return -1;  // 2 or 3 args -> 1 result
        
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
        
        // Unpacking
        case Opcode::UNPACK_SEQUENCE: return arg - 1;
        case Opcode::UNPACK_EX: return (arg & 0xFF) + (arg >> 8);
        
        // Import
        case Opcode::IMPORT_NAME: return -1;  // Pops 2, pushes 1
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

std::string Instruction::to_string() const {
    std::ostringstream oss;
    oss << offset << "\t" << opcode_name(opcode);
    if (has_arg()) {
        oss << "\t" << arg;
    }
    return oss.str();
}

} // namespace compiler
} // namespace cpython_cpp
