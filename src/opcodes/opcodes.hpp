#ifndef CPYTHON_CPP_OPCODES_HPP
#define CPYTHON_CPP_OPCODES_HPP

#include <cstdint>

namespace cpython_cpp {
namespace opcodes {

/**
 * Opcode definitions matching CPython
 * Reference: Include/opcode.h, Include/opcode_ids.h
 *
 * This module can be developed independently to understand
 * Python's bytecode instruction set.
 */

// Opcode IDs (subset - see Include/opcode_ids.h for full list)
enum class Opcode : uint8_t {
    // Stack manipulation
    POP_TOP = 1,
    ROT_TWO = 2,
    ROT_THREE = 3,

    // Loading
    LOAD_CONST = 100,
    LOAD_NAME = 101,
    LOAD_FAST = 124,
    LOAD_GLOBAL = 116,

    // Storing
    STORE_NAME = 90,
    STORE_FAST = 125,

    // Operations
    BINARY_ADD = 23,
    BINARY_SUBTRACT = 24,
    BINARY_MULTIPLY = 20,

    // Function calls
    CALL_FUNCTION = 131,
    RETURN_VALUE = 83,

    // Control flow
    JUMP_FORWARD = 110,
    JUMP_IF_FALSE_OR_POP = 111,
    POP_JUMP_IF_FALSE = 114,

    // Comparison
    COMPARE_OP = 107,

    // Special
    NOP = 9,
    END_OF_CODE = 0
};

/**
 * Opcode metadata
 */
struct OpcodeInfo {
    Opcode code;
    const char* name;
    int arg_count;  // 0, 1, or 2
    bool has_arg;

    OpcodeInfo(Opcode c, const char* n, int ac, bool ha)
        : code(c), name(n), arg_count(ac), has_arg(ha) {}
};

// Get opcode information
OpcodeInfo get_opcode_info(Opcode opcode);

// Get opcode name as string
const char* opcode_name(Opcode opcode);

} // namespace opcodes
} // namespace cpython_cpp

#endif // CPYTHON_CPP_OPCODES_HPP







