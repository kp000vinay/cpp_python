#pragma once

#include "pyobject.hpp"
#include "../compiler/code_object.hpp"
#include "../compiler/opcode.hpp"
#include <stack>
#include <vector>
#include <string>
#include <memory>
#include <stdexcept>
#include <iostream>

namespace cpython_cpp {
namespace vm {

/**
 * Frame - Execution frame for a code object
 * 
 * Represents the execution state of a function/module.
 * Contains the value stack, local variables, and instruction pointer.
 */
struct Frame {
    std::shared_ptr<compiler::CodeObject> code;  // Code object being executed
    std::shared_ptr<PyDict> globals;             // Global namespace
    std::shared_ptr<PyDict> locals;              // Local namespace
    std::vector<PyObject> value_stack;           // Value stack
    size_t ip;                                   // Instruction pointer
    
    Frame(std::shared_ptr<compiler::CodeObject> code,
          std::shared_ptr<PyDict> globals,
          std::shared_ptr<PyDict> locals = nullptr)
        : code(std::move(code))
        , globals(std::move(globals))
        , locals(locals ? locals : std::make_shared<PyDict>())
        , ip(0) {}
    
    // Stack operations
    void push(const PyObject& obj) {
        value_stack.push_back(obj);
    }
    
    PyObject pop() {
        if (value_stack.empty()) {
            throw std::runtime_error("Stack underflow");
        }
        PyObject obj = value_stack.back();
        value_stack.pop_back();
        return obj;
    }
    
    PyObject& top() {
        if (value_stack.empty()) {
            throw std::runtime_error("Stack is empty");
        }
        return value_stack.back();
    }
    
    size_t stack_size() const {
        return value_stack.size();
    }
    
    // Instruction reading (CPython 3.11+ format: 2-byte instructions)
    uint8_t read_byte() {
        if (ip >= code->co_code.size()) {
            throw std::runtime_error("Instruction pointer out of bounds");
        }
        return code->co_code[ip++];
    }
    
    int read_arg() {
        // In CPython 3.11+, the argument is in the same byte as the opcode
        // Instructions are 2 bytes: [opcode, arg]
        // For extended args, EXTENDED_ARG opcode is used
        return read_byte();
    }
};

/**
 * VirtualMachine - Stack-based Python bytecode interpreter
 * 
 * Executes compiled Python bytecode using a stack-based execution model.
 * Implements the core opcode dispatch loop.
 */
class VirtualMachine {
public:
    VirtualMachine()
        : globals_(std::make_shared<PyDict>())
        , builtins_(std::make_shared<PyDict>()) {
        setup_builtins();
    }
    
    /**
     * Execute a code object
     */
    PyObject execute(std::shared_ptr<compiler::CodeObject> code) {
        // Create a new frame for this code object
        Frame frame(code, globals_);
        
        // Execute the frame
        return run_frame(frame);
    }
    
    /**
     * Get the global namespace
     */
    std::shared_ptr<PyDict> globals() { return globals_; }
    
    /**
     * Get the builtins namespace
     */
    std::shared_ptr<PyDict> builtins() { return builtins_; }
    
private:
    std::shared_ptr<PyDict> globals_;   // Global namespace
    std::shared_ptr<PyDict> builtins_;  // Built-in functions
    
    /**
     * Setup built-in functions
     */
    void setup_builtins() {
        // Built-ins will be added in the next phase
        // For now, just create an empty namespace
    }
    
    /**
     * Run a frame until it returns
     */
    PyObject run_frame(Frame& frame) {
        using compiler::Opcode;
        
        while (frame.ip < frame.code->co_code.size()) {
            // Read the opcode
            uint8_t opcode_byte = frame.read_byte();
            Opcode opcode = static_cast<Opcode>(opcode_byte);
            
            // In CPython 3.11+, all instructions are 2 bytes (word-aligned)
            // Always read the argument byte, even if unused
            int arg = frame.read_arg();
            
            // Ignore argument if opcode doesn't use it
            if (!compiler::opcode_has_arg(opcode)) {
                arg = -1;
            }
            
            // Debug output (optional)
            #ifdef VM_DEBUG
            std::cout << "IP=" << (frame.ip - 3) << " "
                      << compiler::opcode_name(opcode) << " ";
            if (arg != -1) std::cout << arg;
            std::cout << " (stack size: " << frame.stack_size() << ")\n";
            #endif
            
            // Dispatch opcode
            try {
                dispatch_opcode(frame, opcode, arg);
            } catch (const std::exception& e) {
                std::cerr << "Runtime error at IP " << frame.ip << ": " << e.what() << "\n";
                throw;
            }
            
            // Check for return
            if (opcode == Opcode::RETURN_VALUE) {
                if (frame.stack_size() > 0) {
                    return frame.pop();
                } else {
                    return std::monostate{};  // None
                }
            }
        }
        
        // If we reach here, the code didn't return explicitly
        return std::monostate{};  // None
    }
    
    /**
     * Dispatch a single opcode
     */
    void dispatch_opcode(Frame& frame, compiler::Opcode opcode, int arg) {
        using compiler::Opcode;
        
        switch (opcode) {
            // === Load/Store Operations ===
            case Opcode::LOAD_CONST:
                op_load_const(frame, arg);
                break;
                
            case Opcode::LOAD_NAME:
                op_load_name(frame, arg);
                break;
                
            case Opcode::STORE_NAME:
                op_store_name(frame, arg);
                break;
                
            case Opcode::LOAD_FAST:
                op_load_fast(frame, arg);
                break;
                
            case Opcode::STORE_FAST:
                op_store_fast(frame, arg);
                break;
                
            case Opcode::LOAD_GLOBAL:
                op_load_global(frame, arg);
                break;
                
            case Opcode::STORE_GLOBAL:
                op_store_global(frame, arg);
                break;
                
            // === Stack Operations ===
            case Opcode::POP_TOP:
                frame.pop();
                break;
                
            // DUP_TOP and ROT_TWO not in CPython 3.11+ opcodes
            // Use COPY and SWAP instead if needed
                
            // === Arithmetic Operations ===
            case Opcode::BINARY_OP:
                op_binary_op(frame, arg);
                break;
                
            case Opcode::UNARY_NOT:
                op_unary_not(frame);
                break;
                
            case Opcode::UNARY_NEGATIVE:
                op_unary_negative(frame);
                break;
                
            case Opcode::UNARY_INVERT:
                op_unary_invert(frame);
                break;
                
            // === Comparison Operations ===
            case Opcode::COMPARE_OP:
                op_compare_op(frame, arg);
                break;
                
            // === Control Flow ===
            case Opcode::RETURN_VALUE:
                // Handled in run_frame()
                break;
                
            case Opcode::JUMP_FORWARD:
                frame.ip += arg;
                break;
                
            case Opcode::JUMP_BACKWARD:
                frame.ip -= arg;
                break;
                
            case Opcode::POP_JUMP_IF_FALSE: {
                PyObject cond = frame.pop();
                if (!to_bool(cond)) {
                    frame.ip = arg;
                }
                break;
            }
                
            case Opcode::POP_JUMP_IF_TRUE: {
                PyObject cond = frame.pop();
                if (to_bool(cond)) {
                    frame.ip = arg;
                }
                break;
            }
                
            // === Function Calls ===
            case Opcode::CALL:
                op_call(frame, arg);
                break;
                
            // === Build Operations ===
            case Opcode::BUILD_LIST:
                op_build_list(frame, arg);
                break;
                
            case Opcode::BUILD_TUPLE:
                op_build_tuple(frame, arg);
                break;
                
            case Opcode::BUILD_MAP:
                op_build_map(frame, arg);
                break;
                
            case Opcode::BUILD_SET:
                op_build_set(frame, arg);
                break;
                
            // === Small Integer Optimization ===
            case Opcode::LOAD_SMALL_INT:
                frame.push(static_cast<int64_t>(arg));
                break;
                
            // === T-string Operations (temporary stub) ===
            case Opcode::BUILD_TEMPLATE:
            case Opcode::BUILD_INTERPOLATION:
                // For now, just pass through (t-strings not fully implemented)
                // This prevents crashes on simple code that shouldn't use t-strings
                break;
                
            // === Slicing Operations ===
            case Opcode::BINARY_SLICE:
                op_binary_slice(frame);
                break;
                
            // === Context Manager Operations ===
            case Opcode::BEFORE_WITH:
                op_before_with(frame);
                break;
                
            case Opcode::BEFORE_ASYNC_WITH:
                op_before_async_with(frame);
                break;
                
            // === Cache Operations (CPython 3.11+) ===
            case Opcode::CACHE:
                // CACHE is a no-op placeholder for inline caching
                // Just skip it
                break;
                
            default:
                throw std::runtime_error("Unimplemented opcode: " + 
                    std::string(compiler::opcode_name(opcode)) + 
                    " (" + std::to_string(static_cast<int>(opcode)) + ")");
        }
    }
    
    // === Opcode Implementations ===
    
    void op_load_const(Frame& frame, int arg) {
        if (arg < 0 || arg >= static_cast<int>(frame.code->co_consts.size())) {
            throw std::runtime_error("Invalid constant index: " + std::to_string(arg));
        }
        
        const auto& constant = frame.code->co_consts[arg];
        
        // Convert PyConstant to PyObject
        PyObject obj = std::visit([](const auto& val) -> PyObject {
            using T = std::decay_t<decltype(val)>;
            if constexpr (std::is_same_v<T, std::monostate>) {
                return std::monostate{};
            } else if constexpr (std::is_same_v<T, bool>) {
                return val;
            } else if constexpr (std::is_same_v<T, int64_t>) {
                return val;
            } else if constexpr (std::is_same_v<T, double>) {
                return val;
            } else if constexpr (std::is_same_v<T, std::string>) {
                return val;
            } else {
                return std::monostate{};
            }
        }, constant);
        
        frame.push(obj);
    }
    
    void op_load_name(Frame& frame, int arg) {
        if (arg < 0 || arg >= static_cast<int>(frame.code->co_names.size())) {
            throw std::runtime_error("Invalid name index: " + std::to_string(arg));
        }
        
        const std::string& name = frame.code->co_names[arg];
        
        // Try locals first, then globals, then builtins
        if (frame.locals->contains(name)) {
            frame.push(frame.locals->get(name));
        } else if (frame.globals->contains(name)) {
            frame.push(frame.globals->get(name));
        } else if (builtins_->contains(name)) {
            frame.push(builtins_->get(name));
        } else {
            throw std::runtime_error("NameError: name '" + name + "' is not defined");
        }
    }
    
    void op_store_name(Frame& frame, int arg) {
        if (arg < 0 || arg >= static_cast<int>(frame.code->co_names.size())) {
            throw std::runtime_error("Invalid name index: " + std::to_string(arg));
        }
        
        const std::string& name = frame.code->co_names[arg];
        PyObject value = frame.pop();
        frame.locals->set(name, value);
    }
    
    void op_load_fast(Frame& frame, int arg) {
        if (arg < 0 || arg >= static_cast<int>(frame.code->co_varnames.size())) {
            throw std::runtime_error("Invalid varname index: " + std::to_string(arg));
        }
        
        const std::string& name = frame.code->co_varnames[arg];
        
        if (frame.locals->contains(name)) {
            frame.push(frame.locals->get(name));
        } else {
            throw std::runtime_error("UnboundLocalError: local variable '" + name + "' referenced before assignment");
        }
    }
    
    void op_store_fast(Frame& frame, int arg) {
        if (arg < 0 || arg >= static_cast<int>(frame.code->co_varnames.size())) {
            throw std::runtime_error("Invalid varname index: " + std::to_string(arg));
        }
        
        const std::string& name = frame.code->co_varnames[arg];
        PyObject value = frame.pop();
        frame.locals->set(name, value);
    }
    
    void op_load_global(Frame& frame, int arg) {
        if (arg < 0 || arg >= static_cast<int>(frame.code->co_names.size())) {
            throw std::runtime_error("Invalid name index: " + std::to_string(arg));
        }
        
        const std::string& name = frame.code->co_names[arg];
        
        // Try globals first, then builtins
        if (frame.globals->contains(name)) {
            frame.push(frame.globals->get(name));
        } else if (builtins_->contains(name)) {
            frame.push(builtins_->get(name));
        } else {
            throw std::runtime_error("NameError: name '" + name + "' is not defined");
        }
    }
    
    void op_store_global(Frame& frame, int arg) {
        if (arg < 0 || arg >= static_cast<int>(frame.code->co_names.size())) {
            throw std::runtime_error("Invalid name index: " + std::to_string(arg));
        }
        
        const std::string& name = frame.code->co_names[arg];
        PyObject value = frame.pop();
        frame.globals->set(name, value);
    }
    
    void op_binary_op(Frame& frame, int op) {
        PyObject right = frame.pop();
        PyObject left = frame.pop();
        
        PyObject result;
        
        // Handle int operations
        if (is_int(left) && is_int(right)) {
            int64_t l = std::get<int64_t>(left);
            int64_t r = std::get<int64_t>(right);
            
            switch (op) {
                case 0: result = l + r; break;  // ADD
                case 1: result = l & r; break;  // AND
                case 2: result = l / r; break;  // FLOOR_DIV
                case 3: result = l << r; break; // LSHIFT
                case 4: result = l % r; break;  // MODULO
                case 5: result = l * r; break;  // MULTIPLY
                case 6: result = l | r; break;  // OR
                case 7: result = static_cast<int64_t>(std::pow(l, r)); break;  // POWER
                case 8: result = l >> r; break; // RSHIFT
                case 9: result = l - r; break;  // SUBTRACT
                case 10: result = l / r; break; // TRUE_DIV (simplified)
                case 11: result = l ^ r; break; // XOR
                default:
                    throw std::runtime_error("Unknown binary operation: " + std::to_string(op));
            }
        }
        // Handle float operations
        else if ((is_int(left) || is_float(left)) && (is_int(right) || is_float(right))) {
            double l = to_float(left);
            double r = to_float(right);
            
            switch (op) {
                case 0: result = l + r; break;  // ADD
                case 5: result = l * r; break;  // MULTIPLY
                case 7: result = std::pow(l, r); break;  // POWER
                case 9: result = l - r; break;  // SUBTRACT
                case 10: result = l / r; break; // TRUE_DIV
                default:
                    throw std::runtime_error("Unsupported float operation: " + std::to_string(op));
            }
        }
        // Handle string operations
        else if (is_string(left) && is_string(right)) {
            std::string l = std::get<std::string>(left);
            std::string r = std::get<std::string>(right);
            
            switch (op) {
                case 0: result = l + r; break;  // ADD (concatenation)
                default:
                    throw std::runtime_error("Unsupported string operation: " + std::to_string(op));
            }
        }
        else {
            throw std::runtime_error("Unsupported operand types for binary operation");
        }
        
        frame.push(result);
    }
    
    void op_unary_not(Frame& frame) {
        PyObject value = frame.pop();
        frame.push(!to_bool(value));
    }
    
    void op_unary_negative(Frame& frame) {
        PyObject value = frame.pop();
        if (is_int(value)) {
            frame.push(-std::get<int64_t>(value));
        } else if (is_float(value)) {
            frame.push(-std::get<double>(value));
        } else {
            throw std::runtime_error("Unsupported operand type for unary -");
        }
    }
    
    void op_unary_positive(Frame& frame) {
        PyObject value = frame.pop();
        if (is_int(value) || is_float(value)) {
            frame.push(value);  // No-op for numbers
        } else {
            throw std::runtime_error("Unsupported operand type for unary +");
        }
    }
    
    void op_unary_invert(Frame& frame) {
        PyObject value = frame.pop();
        if (is_int(value)) {
            frame.push(~std::get<int64_t>(value));
        } else {
            throw std::runtime_error("Unsupported operand type for unary ~");
        }
    }
    
    void op_compare_op(Frame& frame, int op) {
        PyObject right = frame.pop();
        PyObject left = frame.pop();
        
        bool result = false;
        
        // Handle int comparisons
        if (is_int(left) && is_int(right)) {
            int64_t l = std::get<int64_t>(left);
            int64_t r = std::get<int64_t>(right);
            
            switch (op) {
                case 0: result = (l < r); break;   // <
                case 1: result = (l <= r); break;  // <=
                case 2: result = (l == r); break;  // ==
                case 3: result = (l != r); break;  // !=
                case 4: result = (l > r); break;   // >
                case 5: result = (l >= r); break;  // >=
                default:
                    throw std::runtime_error("Unknown comparison operation: " + std::to_string(op));
            }
        }
        // Handle float comparisons
        else if ((is_int(left) || is_float(left)) && (is_int(right) || is_float(right))) {
            double l = to_float(left);
            double r = to_float(right);
            
            switch (op) {
                case 0: result = (l < r); break;   // <
                case 1: result = (l <= r); break;  // <=
                case 2: result = (l == r); break;  // ==
                case 3: result = (l != r); break;  // !=
                case 4: result = (l > r); break;   // >
                case 5: result = (l >= r); break;  // >=
                default:
                    throw std::runtime_error("Unknown comparison operation: " + std::to_string(op));
            }
        }
        // Handle string comparisons
        else if (is_string(left) && is_string(right)) {
            std::string l = std::get<std::string>(left);
            std::string r = std::get<std::string>(right);
            
            switch (op) {
                case 0: result = (l < r); break;   // <
                case 1: result = (l <= r); break;  // <=
                case 2: result = (l == r); break;  // ==
                case 3: result = (l != r); break;  // !=
                case 4: result = (l > r); break;   // >
                case 5: result = (l >= r); break;  // >=
                default:
                    throw std::runtime_error("Unknown comparison operation: " + std::to_string(op));
            }
        }
        else {
            throw std::runtime_error("Unsupported operand types for comparison");
        }
        
        frame.push(result);
    }
    
    void op_call(Frame& frame, int argc) {
        // Pop arguments
        std::vector<PyObject> args;
        for (int i = 0; i < argc; ++i) {
            args.insert(args.begin(), frame.pop());
        }
        
        // Pop callable
        PyObject callable = frame.pop();
        
        // For now, only handle built-in print function
        // Full function call support will be added in Phase 3
        if (is_string(callable)) {
            std::string func_name = std::get<std::string>(callable);
            if (func_name == "<builtin print>") {
                // Call print with arguments
                for (size_t i = 0; i < args.size(); ++i) {
                    if (i > 0) std::cout << " ";
                    print_object(args[i]);
                }
                std::cout << "\n";
                frame.push(std::monostate{});  // print returns None
                return;
            }
        }
        
        throw std::runtime_error("Unsupported callable type");
    }
    
    void op_build_list(Frame& frame, int count) {
        auto list = std::make_shared<PyList>();
        for (int i = 0; i < count; ++i) {
            list->items.insert(list->items.begin(), frame.pop());
        }
        frame.push(list);
    }
    
    void op_build_tuple(Frame& frame, int count) {
        std::vector<PyObject> items;
        for (int i = 0; i < count; ++i) {
            items.insert(items.begin(), frame.pop());
        }
        frame.push(std::make_shared<PyTuple>(items));
    }
    
    void op_binary_slice(Frame& frame) {
        // TOS = end, TOS1 = start, TOS2 = container
        PyObject end = frame.pop();
        PyObject start = frame.pop();
        PyObject container = frame.pop();
        
        // For Phase 1, implement basic list/string slicing
        if (is_list(container)) {
            auto list = std::get<std::shared_ptr<PyList>>(container);
            int start_idx = is_int(start) ? std::get<int64_t>(start) : 0;
            int end_idx = is_int(end) ? std::get<int64_t>(end) : list->items.size();
            
            auto result = std::make_shared<PyList>();
            for (int i = start_idx; i < end_idx && i < static_cast<int>(list->items.size()); ++i) {
                result->items.push_back(list->items[i]);
            }
            frame.push(result);
        } else if (is_string(container)) {
            std::string str = std::get<std::string>(container);
            int start_idx = is_int(start) ? std::get<int64_t>(start) : 0;
            int end_idx = is_int(end) ? std::get<int64_t>(end) : str.size();
            
            frame.push(str.substr(start_idx, end_idx - start_idx));
        } else {
            throw std::runtime_error("BINARY_SLICE: unsupported type");
        }
    }
    
    void op_before_with(Frame& frame) {
        // TOS = context manager
        // Call __enter__() and push __exit__ for later
        // For Phase 1, simplified implementation
        PyObject cm = frame.pop();
        
        // Push __exit__ (simplified - just push None)
        frame.push(std::monostate{});
        
        // Push result of __enter__() (simplified - just push the CM itself)
        frame.push(cm);
    }
    
    void op_before_async_with(Frame& frame) {
        // Similar to BEFORE_WITH but for async context managers
        // For Phase 1, same simplified implementation
        op_before_with(frame);
    }
    
    void op_build_map(Frame& frame, int count) {
        auto dict = std::make_shared<PyDict>();
        for (int i = 0; i < count; ++i) {
            PyObject value = frame.pop();
            PyObject key = frame.pop();
            if (!is_string(key)) {
                throw std::runtime_error("Dictionary keys must be strings");
            }
            dict->set(std::get<std::string>(key), value);
        }
        frame.push(dict);
    }
    
    void op_build_set(Frame& frame, int count) {
        auto set = std::make_shared<PySet>();
        for (int i = 0; i < count; ++i) {
            set->add(frame.pop());
        }
        frame.push(set);
    }
};

} // namespace vm
} // namespace cpython_cpp
