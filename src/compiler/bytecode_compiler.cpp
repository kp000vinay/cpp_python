#include "bytecode_compiler.hpp"
#include <sstream>
#include <algorithm>

namespace cpython_cpp {
namespace compiler {

BytecodeCompiler::BytecodeCompiler()
    : current_lineno_(1) {}

std::shared_ptr<CodeObject> BytecodeCompiler::compile(
    const ast::Module& module, const std::string& filename) {
    
    filename_ = filename;
    errors_.clear();
    
    // Create module scope
    push_scope(ScopeType::Module, "<module>");
    code().co_filename = filename;
    
    // Add None to constants (always at index 0)
    code().add_const(std::monostate{});
    
    // Compile all statements in the module
    for (const auto& stmt : module.body()) {
        compile_stmt(stmt.get());
    }
    
    // Return None at end of module
    emit(Opcode::LOAD_CONST, 0);  // None
    emit(Opcode::RETURN_VALUE);
    
    // Finalize code object
    code().calculate_stacksize();
    code().assemble();
    
    return pop_scope();
}

void BytecodeCompiler::push_scope(ScopeType type, const std::string& name) {
    scopes_.push(CompilerScope(type, name));
    
    // Set flags based on scope type
    if (type == ScopeType::Function) {
        code().co_flags |= CodeFlags::CO_OPTIMIZED | CodeFlags::CO_NEWLOCALS;
    }
}

std::shared_ptr<CodeObject> BytecodeCompiler::pop_scope() {
    auto code_obj = current_scope().code;
    scopes_.pop();
    return code_obj;
}

void BytecodeCompiler::emit(Opcode op, int arg) {
    code().emit(op, arg, current_lineno_);
}

void BytecodeCompiler::emit_with_lineno(Opcode op, int arg, int lineno) {
    code().emit(op, arg, lineno);
}

int BytecodeCompiler::emit_jump(Opcode op) {
    int index = static_cast<int>(code().instructions.size());
    emit(op, 0);  // Placeholder, will be patched
    return index;
}

void BytecodeCompiler::patch_jump(int instr_index) {
    // Patch to jump to current position
    int target = code().current_offset();
    code().instructions[instr_index].arg = target;
}

void BytecodeCompiler::patch_jump_to(int instr_index, int target) {
    code().instructions[instr_index].arg = target;
}

// === Statement Compilation ===

void BytecodeCompiler::compile_stmt(ast::Stmt* stmt) {
    if (!stmt) return;
    
    current_lineno_ = stmt->lineno();
    
    // Try each statement type
    if (auto* node = dynamic_cast<ast::FunctionDef*>(stmt)) {
        compile_function_def(node);
    } else if (auto* node = dynamic_cast<ast::AsyncFunctionDef*>(stmt)) {
        compile_async_function_def(node);
    } else if (auto* node = dynamic_cast<ast::ClassDef*>(stmt)) {
        compile_class_def(node);
    } else if (auto* node = dynamic_cast<ast::Return*>(stmt)) {
        compile_return(node);
    } else if (auto* node = dynamic_cast<ast::Assign*>(stmt)) {
        compile_assign(node);
    } else if (auto* node = dynamic_cast<ast::AnnAssign*>(stmt)) {
        compile_ann_assign(node);
    } else if (auto* node = dynamic_cast<ast::AugAssign*>(stmt)) {
        compile_aug_assign(node);
    } else if (auto* node = dynamic_cast<ast::If*>(stmt)) {
        compile_if(node);
    } else if (auto* node = dynamic_cast<ast::While*>(stmt)) {
        compile_while(node);
    } else if (auto* node = dynamic_cast<ast::For*>(stmt)) {
        compile_for(node);
    } else if (auto* node = dynamic_cast<ast::TryStar*>(stmt)) {
        compile_try_star(node);
    } else if (auto* node = dynamic_cast<ast::Try*>(stmt)) {
        compile_try(node);
    } else if (auto* node = dynamic_cast<ast::Import*>(stmt)) {
        compile_import(node);
    } else if (auto* node = dynamic_cast<ast::ImportFrom*>(stmt)) {
        compile_import_from(node);
    } else if (auto* node = dynamic_cast<ast::Global*>(stmt)) {
        compile_global(node);
    } else if (auto* node = dynamic_cast<ast::Nonlocal*>(stmt)) {
        compile_nonlocal(node);
    } else if (auto* node = dynamic_cast<ast::ExprStmt*>(stmt)) {
        compile_expr_stmt(node);
    } else if (auto* node = dynamic_cast<ast::Pass*>(stmt)) {
        compile_pass(node);
    } else if (auto* node = dynamic_cast<ast::Break*>(stmt)) {
        compile_break(node);
    } else if (auto* node = dynamic_cast<ast::Continue*>(stmt)) {
        compile_continue(node);
    } else if (auto* node = dynamic_cast<ast::Raise*>(stmt)) {
        compile_raise(node);
    } else if (auto* node = dynamic_cast<ast::Assert*>(stmt)) {
        compile_assert(node);
    } else if (auto* node = dynamic_cast<ast::Delete*>(stmt)) {
        compile_delete(node);
    } else if (auto* node = dynamic_cast<ast::Match*>(stmt)) {
        compile_match(node);
    } else {
        add_error("Unknown statement type", stmt->lineno());
    }
}

void BytecodeCompiler::compile_function_def(ast::FunctionDef* node) {
    // Push function scope
    push_scope(ScopeType::Function, node->name());
    
    // Add arguments as local variables
    for (const auto& arg : node->args()) {
        code().add_varname(arg.arg_name);
    }
    code().co_argcount = static_cast<int>(node->args().size());
    
    // Add None constant
    code().add_const(std::monostate{});
    
    // Compile function body
    for (const auto& stmt : node->body()) {
        compile_stmt(stmt.get());
    }
    
    // Implicit return None if no explicit return
    emit(Opcode::LOAD_CONST, 0);
    emit(Opcode::RETURN_VALUE);
    
    // Finalize function code object
    code().calculate_stacksize();
    code().assemble();
    
    auto func_code = pop_scope();
    
    // In parent scope: create function object
    int code_idx = code().add_const(func_code);
    emit(Opcode::LOAD_CONST, code_idx);
    emit(Opcode::MAKE_FUNCTION, 0);
    
    // Store function in namespace
    store_name(node->name());
}

void BytecodeCompiler::compile_async_function_def(ast::AsyncFunctionDef* node) {
    // Similar to function_def but with CO_COROUTINE flag
    push_scope(ScopeType::Function, node->name());
    code().co_flags |= CodeFlags::CO_COROUTINE;
    
    for (const auto& arg : node->args()) {
        code().add_varname(arg.arg_name);
    }
    code().co_argcount = static_cast<int>(node->args().size());
    
    code().add_const(std::monostate{});
    
    for (const auto& stmt : node->body()) {
        compile_stmt(stmt.get());
    }
    
    emit(Opcode::LOAD_CONST, 0);
    emit(Opcode::RETURN_VALUE);
    
    code().calculate_stacksize();
    code().assemble();
    
    auto func_code = pop_scope();
    
    int code_idx = code().add_const(func_code);
    emit(Opcode::LOAD_CONST, code_idx);
    emit(Opcode::MAKE_FUNCTION, 0);
    
    store_name(node->name());
}

void BytecodeCompiler::compile_class_def(ast::ClassDef* node) {
    // Load __build_class__
    emit(Opcode::LOAD_BUILD_CLASS);
    
    // Create class body function
    push_scope(ScopeType::Class, node->name());
    code().co_flags |= CodeFlags::CO_NEWLOCALS;
    
    // Load __name__ and store as __module__
    emit(Opcode::LOAD_NAME, code().add_name("__name__"));
    emit(Opcode::STORE_NAME, code().add_name("__module__"));
    
    // Store __qualname__
    int qualname_idx = code().add_const(node->name());
    emit(Opcode::LOAD_CONST, qualname_idx);
    emit(Opcode::STORE_NAME, code().add_name("__qualname__"));
    
    // Compile class body
    for (const auto& stmt : node->body()) {
        compile_stmt(stmt.get());
    }
    
    // Return None
    emit(Opcode::LOAD_CONST, code().add_const(std::monostate{}));
    emit(Opcode::RETURN_VALUE);
    
    code().calculate_stacksize();
    code().assemble();
    
    auto class_code = pop_scope();
    
    // In parent scope: call __build_class__
    int code_idx = code().add_const(class_code);
    emit(Opcode::LOAD_CONST, code_idx);
    emit(Opcode::MAKE_FUNCTION, 0);
    
    // Push class name
    int name_idx = code().add_const(node->name());
    emit(Opcode::LOAD_CONST, name_idx);
    
    // Push base classes
    for (const auto& base : node->bases()) {
        compile_expr(base.get());
    }
    
    // Call __build_class__(func, name, *bases)
    emit(Opcode::CALL, 2 + static_cast<int>(node->bases().size()));
    
    // Store class
    store_name(node->name());
}

void BytecodeCompiler::compile_return(ast::Return* node) {
    if (node->value()) {
        compile_expr(node->value().get());
    } else {
        emit(Opcode::LOAD_CONST, 0);  // None
    }
    emit(Opcode::RETURN_VALUE);
}

void BytecodeCompiler::compile_assign(ast::Assign* node) {
    // Compile the value
    compile_expr(node->value().get());
    
    // Store to each target (in reverse order for multiple targets)
    const auto& targets = node->targets();
    for (size_t i = 0; i < targets.size(); ++i) {
        if (i < targets.size() - 1) {
            emit(Opcode::COPY, 1);  // Duplicate value for next target
        }
        compile_store_target(targets[i].get());
    }
}

void BytecodeCompiler::compile_ann_assign(ast::AnnAssign* node) {
    // Annotated assignment: x: int = 5
    if (node->value()) {
        compile_expr(node->value().get());
        compile_store_target(node->target().get());
    }
    // Note: Annotation itself is handled at runtime via __annotations__
}

void BytecodeCompiler::compile_aug_assign(ast::AugAssign* node) {
    // Augmented assignment: x += 1
    // First load the target value
    compile_expr(node->target().get());
    
    // Compile the value to add/subtract/etc
    compile_expr(node->value().get());
    
    // Emit inplace binary operation
    BinaryOpCode op_code = get_inplace_binop_code(node->op());
    emit(Opcode::BINARY_OP, static_cast<int>(op_code));
    
    // Store back to target
    compile_store_target(node->target().get());
}

void BytecodeCompiler::compile_if(ast::If* node) {
    // Compile test
    compile_expr(node->test().get());
    
    // Jump to else if false
    int jump_to_else = emit_jump(Opcode::POP_JUMP_IF_FALSE);
    
    // Compile if body
    for (const auto& stmt : node->body()) {
        compile_stmt(stmt.get());
    }
    
    if (!node->orelse().empty()) {
        // Jump over else
        int jump_to_end = emit_jump(Opcode::JUMP_FORWARD);
        
        // Patch jump to else
        patch_jump(jump_to_else);
        
        // Compile else body
        for (const auto& stmt : node->orelse()) {
            compile_stmt(stmt.get());
        }
        
        // Patch jump to end
        patch_jump(jump_to_end);
    } else {
        // No else, just patch jump
        patch_jump(jump_to_else);
    }
}

void BytecodeCompiler::compile_while(ast::While* node) {
    // Start of loop
    int loop_start = code().current_offset();
    
    // Push loop info for break/continue
    current_scope().loops.push({loop_start, {}, {}});
    
    // Compile test
    compile_expr(node->test().get());
    
    // Jump to end if false
    int jump_to_end = emit_jump(Opcode::POP_JUMP_IF_FALSE);
    
    // Compile body
    for (const auto& stmt : node->body()) {
        compile_stmt(stmt.get());
    }
    
    // Jump back to start
    emit(Opcode::JUMP_BACKWARD, code().current_offset() - loop_start);
    
    // Patch jump to end
    patch_jump(jump_to_end);
    
    // Handle else clause
    if (!node->orelse().empty()) {
        for (const auto& stmt : node->orelse()) {
            compile_stmt(stmt.get());
        }
    }
    
    // Patch break jumps
    auto& loop_info = current_scope().loops.top();
    for (int idx : loop_info.break_patches) {
        patch_jump(idx);
    }
    
    current_scope().loops.pop();
}

void BytecodeCompiler::compile_for(ast::For* node) {
    // Compile iterable
    compile_expr(node->iter().get());
    
    // Get iterator
    emit(Opcode::GET_ITER);
    
    // Start of loop
    int loop_start = code().current_offset();
    
    // Push loop info
    current_scope().loops.push({loop_start, {}, {}});
    
    // FOR_ITER jumps to end when exhausted
    int for_iter = emit_jump(Opcode::FOR_ITER);
    
    // Store loop variable
    compile_store_target(node->target().get());
    
    // Compile body
    for (const auto& stmt : node->body()) {
        compile_stmt(stmt.get());
    }
    
    // Jump back to FOR_ITER
    emit(Opcode::JUMP_BACKWARD, code().current_offset() - loop_start);
    
    // Patch FOR_ITER jump
    patch_jump(for_iter);
    
    // End for cleanup
    emit(Opcode::END_FOR);
    
    // Handle else clause
    if (!node->orelse().empty()) {
        for (const auto& stmt : node->orelse()) {
            compile_stmt(stmt.get());
        }
    }
    
    // Patch break jumps
    auto& loop_info = current_scope().loops.top();
    for (int idx : loop_info.break_patches) {
        patch_jump(idx);
    }
    
    current_scope().loops.pop();
}

void BytecodeCompiler::compile_try(ast::Try* node) {
    // Simplified try/except compilation
    // Full implementation would need exception table
    
    // Push exception info
    emit(Opcode::PUSH_EXC_INFO);
    
    // Compile try body
    for (const auto& stmt : node->body()) {
        compile_stmt(stmt.get());
    }
    
    // Jump over handlers if no exception
    int jump_to_end = emit_jump(Opcode::JUMP_FORWARD);
    
    // Compile exception handlers
    for (const auto& handler : node->handlers()) {
        if (handler->type()) {
            compile_expr(handler->type().get());
            emit(Opcode::CHECK_EXC_MATCH);
            int skip_handler = emit_jump(Opcode::POP_JUMP_IF_FALSE);
            
            if (!handler->name().empty()) {
                store_name(handler->name());
            } else {
                emit(Opcode::POP_TOP);
            }
            
            for (const auto& stmt : handler->body()) {
                compile_stmt(stmt.get());
            }
            
            patch_jump(skip_handler);
        } else {
            // Bare except
            emit(Opcode::POP_TOP);
            for (const auto& stmt : handler->body()) {
                compile_stmt(stmt.get());
            }
        }
    }
    
    // Pop exception info
    emit(Opcode::POP_EXCEPT);
    
    patch_jump(jump_to_end);
    
    // Compile else clause
    for (const auto& stmt : node->orelse()) {
        compile_stmt(stmt.get());
    }
    
    // Compile finally clause
    for (const auto& stmt : node->finalbody()) {
        compile_stmt(stmt.get());
    }
}

void BytecodeCompiler::compile_try_star(ast::TryStar* node) {
    // Exception groups (PEP 654)
    emit(Opcode::PUSH_EXC_INFO);
    
    for (const auto& stmt : node->body()) {
        compile_stmt(stmt.get());
    }
    
    int jump_to_end = emit_jump(Opcode::JUMP_FORWARD);
    
    for (const auto& handler : node->handlers()) {
        if (handler->type()) {
            compile_expr(handler->type().get());
            emit(Opcode::CHECK_EG_MATCH);  // Exception group match
            int skip_handler = emit_jump(Opcode::POP_JUMP_IF_FALSE);
            
            if (!handler->name().empty()) {
                store_name(handler->name());
            } else {
                emit(Opcode::POP_TOP);
            }
            
            for (const auto& stmt : handler->body()) {
                compile_stmt(stmt.get());
            }
            
            patch_jump(skip_handler);
        }
    }
    
    emit(Opcode::POP_EXCEPT);
    patch_jump(jump_to_end);
    
    for (const auto& stmt : node->orelse()) {
        compile_stmt(stmt.get());
    }
    
    for (const auto& stmt : node->finalbody()) {
        compile_stmt(stmt.get());
    }
}

void BytecodeCompiler::compile_import(ast::Import* node) {
    for (const auto& alias : node->names()) {
        // Push import level (0 for absolute)
        emit(Opcode::LOAD_CONST, code().add_const(int64_t(0)));
        
        // Push None for fromlist
        emit(Opcode::LOAD_CONST, 0);
        
        // Import the module
        int name_idx = code().add_name(alias->name());
        emit(Opcode::IMPORT_NAME, name_idx);
        
        // Store with alias or original name
        const std::string& store_name_str = alias->asname().empty() ? 
            alias->name() : alias->asname();
        store_name(store_name_str);
    }
}

void BytecodeCompiler::compile_import_from(ast::ImportFrom* node) {
    // Push import level
    emit(Opcode::LOAD_CONST, code().add_const(int64_t(node->level())));
    
    // Build fromlist tuple
    std::vector<std::string> names;
    for (const auto& alias : node->names()) {
        names.push_back(alias->name());
    }
    // For simplicity, we'd need to build a tuple constant
    // This is a simplified version
    emit(Opcode::LOAD_CONST, 0);  // Placeholder
    
    // Import the module
    int name_idx = code().add_name(node->module());
    emit(Opcode::IMPORT_NAME, name_idx);
    
    // Import each name
    for (const auto& alias : node->names()) {
        int from_idx = code().add_name(alias->name());
        emit(Opcode::IMPORT_FROM, from_idx);
        
        const std::string& store_name_str = alias->asname().empty() ?
            alias->name() : alias->asname();
        store_name(store_name_str);
    }
    
    emit(Opcode::POP_TOP);  // Pop the module
}

void BytecodeCompiler::compile_global(ast::Global* node) {
    // Mark names as global in current scope
    for (const auto& name : node->names()) {
        current_scope().globals[name] = 1;
    }
}

void BytecodeCompiler::compile_nonlocal(ast::Nonlocal* node) {
    // Mark names as nonlocal in current scope
    for (const auto& name : node->names()) {
        current_scope().nonlocals[name] = 1;
    }
}

void BytecodeCompiler::compile_expr_stmt(ast::ExprStmt* node) {
    compile_expr(node->value().get());
    emit(Opcode::POP_TOP);  // Discard result
}

void BytecodeCompiler::compile_pass(ast::Pass* node) {
    // No-op
    emit(Opcode::NOP);
}

void BytecodeCompiler::compile_break(ast::Break* node) {
    if (current_scope().loops.empty()) {
        add_error("'break' outside loop", node->lineno());
        return;
    }
    
    int jump_idx = emit_jump(Opcode::JUMP_FORWARD);
    current_scope().loops.top().break_patches.push_back(jump_idx);
}

void BytecodeCompiler::compile_continue(ast::Continue* node) {
    if (current_scope().loops.empty()) {
        add_error("'continue' outside loop", node->lineno());
        return;
    }
    
    auto& loop = current_scope().loops.top();
    emit(Opcode::JUMP_BACKWARD, code().current_offset() - loop.start_offset);
}

void BytecodeCompiler::compile_raise(ast::Raise* node) {
    int arg = 0;
    if (node->exc()) {
        compile_expr(node->exc().get());
        arg = 1;
        if (node->cause()) {
            compile_expr(node->cause().get());
            arg = 2;
        }
    }
    emit(Opcode::RAISE_VARARGS, arg);
}

void BytecodeCompiler::compile_assert(ast::Assert* node) {
    // Compile test
    compile_expr(node->test().get());
    
    // Jump over raise if true
    int jump_if_true = emit_jump(Opcode::POP_JUMP_IF_TRUE);
    
    // Load AssertionError
    emit(Opcode::LOAD_GLOBAL, code().add_name("AssertionError"));
    
    // Load message if present
    if (node->msg()) {
        compile_expr(node->msg().get());
        emit(Opcode::CALL, 1);
    }
    
    emit(Opcode::RAISE_VARARGS, 1);
    
    patch_jump(jump_if_true);
}

void BytecodeCompiler::compile_delete(ast::Delete* node) {
    for (const auto& target : node->targets()) {
        compile_delete_target(target.get());
    }
}

void BytecodeCompiler::compile_match(ast::Match* node) {
    // Pattern matching (PEP 634) - simplified implementation
    compile_expr(node->subject().get());
    
    for (const auto& case_ : node->cases()) {
        // Each case needs pattern matching logic
        // This is a simplified version
        emit(Opcode::COPY, 1);  // Duplicate subject
        
        // Pattern matching would go here
        // For now, just compile the body
        for (const auto& stmt : case_.body()) {
            compile_stmt(stmt.get());
        }
    }
    
    emit(Opcode::POP_TOP);  // Pop subject
}

// === Expression Compilation ===

void BytecodeCompiler::compile_expr(ast::Expr* expr) {
    if (!expr) return;
    
    current_lineno_ = expr->lineno();
    
    if (auto* node = dynamic_cast<ast::Constant*>(expr)) {
        compile_constant(node);
    } else if (auto* node = dynamic_cast<ast::Name*>(expr)) {
        compile_name(node);
    } else if (auto* node = dynamic_cast<ast::BinOp*>(expr)) {
        compile_binop(node);
    } else if (auto* node = dynamic_cast<ast::UnaryOp*>(expr)) {
        compile_unaryop(node);
    } else if (auto* node = dynamic_cast<ast::BoolOpExpr*>(expr)) {
        compile_boolop(node);
    } else if (auto* node = dynamic_cast<ast::Compare*>(expr)) {
        compile_compare(node);
    } else if (auto* node = dynamic_cast<ast::Call*>(expr)) {
        compile_call(node);
    } else if (auto* node = dynamic_cast<ast::Attribute*>(expr)) {
        compile_attribute(node);
    } else if (auto* node = dynamic_cast<ast::Subscript*>(expr)) {
        compile_subscript(node);
    } else if (auto* node = dynamic_cast<ast::Slice*>(expr)) {
        compile_slice(node);
    } else if (auto* node = dynamic_cast<ast::List*>(expr)) {
        compile_list(node);
    } else if (auto* node = dynamic_cast<ast::Tuple*>(expr)) {
        compile_tuple(node);
    } else if (auto* node = dynamic_cast<ast::Dict*>(expr)) {
        compile_dict(node);
    } else if (auto* node = dynamic_cast<ast::Set*>(expr)) {
        compile_set(node);
    } else if (auto* node = dynamic_cast<ast::IfExp*>(expr)) {
        compile_ifexp(node);
    } else if (auto* node = dynamic_cast<ast::Lambda*>(expr)) {
        compile_lambda(node);
    } else if (auto* node = dynamic_cast<ast::ListComp*>(expr)) {
        compile_listcomp(node);
    } else if (auto* node = dynamic_cast<ast::Await*>(expr)) {
        compile_await(node);
    } else if (auto* node = dynamic_cast<ast::Yield*>(expr)) {
        compile_yield(node);
    } else if (auto* node = dynamic_cast<ast::NamedExpr*>(expr)) {
        compile_namedexpr(node);
    } else if (auto* node = dynamic_cast<ast::Starred*>(expr)) {
        compile_starred(node);
    } else if (auto* node = dynamic_cast<ast::JoinedStr*>(expr)) {
        compile_joinedstr(node);
    } else if (auto* node = dynamic_cast<ast::FormattedValue*>(expr)) {
        compile_formattedvalue(node);
    } else if (auto* node = dynamic_cast<ast::Ellipsis*>(expr)) {
        emit(Opcode::LOAD_CONST, code().add_const(std::string("...")));
    } else {
        add_error("Unknown expression type", expr->lineno());
    }
}

void BytecodeCompiler::compile_constant(ast::Constant* node) {
    const std::string& val = node->value();
    
    // Try to parse as different types
    if (val.empty()) {
        emit(Opcode::LOAD_CONST, code().add_const(std::monostate{}));
    } else if (val == "None") {
        emit(Opcode::LOAD_CONST, code().add_const(std::monostate{}));
    } else if (val == "True") {
        emit(Opcode::LOAD_CONST, code().add_const(true));
    } else if (val == "False") {
        emit(Opcode::LOAD_CONST, code().add_const(false));
    } else if (val.front() == '\'' || val.front() == '"') {
        // String literal - remove quotes
        std::string str_val = val.substr(1, val.length() - 2);
        emit(Opcode::LOAD_CONST, code().add_const(str_val));
    } else {
        // Try to parse as number
        bool is_float = false;
        bool is_number = true;
        
        // Check if it looks like a number
        for (size_t i = 0; i < val.size(); ++i) {
            char c = val[i];
            if (c == '.' || c == 'e' || c == 'E') {
                is_float = true;
            } else if (c == '-' || c == '+') {
                // Sign is only valid at start or after e/E
                if (i > 0 && val[i-1] != 'e' && val[i-1] != 'E') {
                    is_number = false;
                    break;
                }
            } else if (!std::isdigit(c) && c != 'x' && c != 'X' && c != 'b' && c != 'B' && c != 'o' && c != 'O' &&
                       !(c >= 'a' && c <= 'f') && !(c >= 'A' && c <= 'F')) {
                is_number = false;
                break;
            }
        }
        
        if (is_number) {
            try {
                if (is_float) {
                    double d = std::stod(val);
                    emit(Opcode::LOAD_CONST, code().add_const(d));
                } else {
                    int64_t i = std::stoll(val, nullptr, 0);  // Auto-detect base
                    if (i >= 0 && i <= 255) {
                        emit(Opcode::LOAD_SMALL_INT, static_cast<int>(i));
                    } else {
                        emit(Opcode::LOAD_CONST, code().add_const(i));
                    }
                }
            } catch (...) {
                // Fallback to string
                emit(Opcode::LOAD_CONST, code().add_const(val));
            }
        } else {
            // Not a number, treat as string
            emit(Opcode::LOAD_CONST, code().add_const(val));
        }
    }
}

void BytecodeCompiler::compile_name(ast::Name* node) {
    int idx = lookup_name(node->id());
    
    switch (node->ctx()) {
        case ast::ExprContext::Load:
            if (current_scope().type == ScopeType::Function &&
                current_scope().locals.count(node->id())) {
                emit(Opcode::LOAD_FAST, current_scope().locals[node->id()]);
            } else if (current_scope().globals.count(node->id())) {
                emit(Opcode::LOAD_GLOBAL, code().add_name(node->id()));
            } else {
                emit(Opcode::LOAD_NAME, code().add_name(node->id()));
            }
            break;
        case ast::ExprContext::Store:
            store_name(node->id());
            break;
        case ast::ExprContext::Del:
            delete_name(node->id());
            break;
    }
}

void BytecodeCompiler::compile_binop(ast::BinOp* node) {
    compile_expr(node->left().get());
    compile_expr(node->right().get());
    
    BinaryOpCode op_code = get_binop_code(node->op());
    emit(Opcode::BINARY_OP, static_cast<int>(op_code));
}

void BytecodeCompiler::compile_unaryop(ast::UnaryOp* node) {
    compile_expr(node->operand().get());
    
    switch (node->op()) {
        case ast::UnaryOp::UnaryOpType::Not:
            emit(Opcode::UNARY_NOT);
            break;
        case ast::UnaryOp::UnaryOpType::UAdd:
            // Unary plus is essentially a no-op for numbers
            break;
        case ast::UnaryOp::UnaryOpType::USub:
            emit(Opcode::UNARY_NEGATIVE);
            break;
        case ast::UnaryOp::UnaryOpType::Invert:
            emit(Opcode::UNARY_INVERT);
            break;
    }
}

void BytecodeCompiler::compile_boolop(ast::BoolOpExpr* node) {
    const auto& values = node->values();
    
    if (node->op() == ast::BoolOp::And) {
        // Short-circuit AND
        std::vector<int> jump_indices;
        
        for (size_t i = 0; i < values.size() - 1; ++i) {
            compile_expr(values[i].get());
            emit(Opcode::COPY, 1);
            jump_indices.push_back(emit_jump(Opcode::POP_JUMP_IF_FALSE));
            emit(Opcode::POP_TOP);
        }
        
        compile_expr(values.back().get());
        
        for (int idx : jump_indices) {
            patch_jump(idx);
        }
    } else {
        // Short-circuit OR
        std::vector<int> jump_indices;
        
        for (size_t i = 0; i < values.size() - 1; ++i) {
            compile_expr(values[i].get());
            emit(Opcode::COPY, 1);
            jump_indices.push_back(emit_jump(Opcode::POP_JUMP_IF_TRUE));
            emit(Opcode::POP_TOP);
        }
        
        compile_expr(values.back().get());
        
        for (int idx : jump_indices) {
            patch_jump(idx);
        }
    }
}

void BytecodeCompiler::compile_compare(ast::Compare* node) {
    compile_expr(node->left().get());
    compile_expr(node->right().get());
    
    CompareOpCode op_code = get_compare_code(node->op());
    emit(Opcode::COMPARE_OP, static_cast<int>(op_code));
}

void BytecodeCompiler::compile_call(ast::Call* node) {
    // Compile function
    compile_expr(node->func().get());
    
    // Compile arguments
    for (const auto& arg : node->args()) {
        compile_expr(arg.get());
    }
    
    // Call function
    emit(Opcode::CALL, static_cast<int>(node->args().size()));
}

void BytecodeCompiler::compile_attribute(ast::Attribute* node) {
    compile_expr(node->value().get());
    int attr_idx = code().add_name(node->attr());
    
    switch (node->ctx()) {
        case ast::ExprContext::Load:
            emit(Opcode::LOAD_ATTR, attr_idx);
            break;
        case ast::ExprContext::Store:
            emit(Opcode::STORE_ATTR, attr_idx);
            break;
        case ast::ExprContext::Del:
            emit(Opcode::DELETE_ATTR, attr_idx);
            break;
    }
}

void BytecodeCompiler::compile_subscript(ast::Subscript* node) {
    compile_expr(node->value().get());
    compile_expr(node->slice().get());
    
    switch (node->ctx()) {
        case ast::ExprContext::Load:
            emit(Opcode::BINARY_OP, static_cast<int>(BinaryOpCode::NB_ADD));  // Placeholder
            // Actually should use BINARY_SUBSCR or similar
            break;
        case ast::ExprContext::Store:
            emit(Opcode::STORE_SUBSCR);
            break;
        case ast::ExprContext::Del:
            emit(Opcode::DELETE_SUBSCR);
            break;
    }
}

void BytecodeCompiler::compile_slice(ast::Slice* node) {
    // Compile slice components
    if (node->lower()) {
        compile_expr(node->lower().get());
    } else {
        emit(Opcode::LOAD_CONST, 0);  // None
    }
    
    if (node->upper()) {
        compile_expr(node->upper().get());
    } else {
        emit(Opcode::LOAD_CONST, 0);  // None
    }
    
    if (node->step()) {
        compile_expr(node->step().get());
        emit(Opcode::BUILD_SLICE, 3);
    } else {
        emit(Opcode::BUILD_SLICE, 2);
    }
}

void BytecodeCompiler::compile_list(ast::List* node) {
    for (const auto& elt : node->elts()) {
        compile_expr(elt.get());
    }
    emit(Opcode::BUILD_LIST, static_cast<int>(node->elts().size()));
}

void BytecodeCompiler::compile_tuple(ast::Tuple* node) {
    for (const auto& elt : node->elts()) {
        compile_expr(elt.get());
    }
    emit(Opcode::BUILD_TUPLE, static_cast<int>(node->elts().size()));
}

void BytecodeCompiler::compile_dict(ast::Dict* node) {
    const auto& keys = node->keys();
    const auto& values = node->values();
    
    for (size_t i = 0; i < keys.size(); ++i) {
        compile_expr(keys[i].get());
        compile_expr(values[i].get());
    }
    emit(Opcode::BUILD_MAP, static_cast<int>(keys.size()));
}

void BytecodeCompiler::compile_set(ast::Set* node) {
    for (const auto& elt : node->elts()) {
        compile_expr(elt.get());
    }
    emit(Opcode::BUILD_SET, static_cast<int>(node->elts().size()));
}

void BytecodeCompiler::compile_ifexp(ast::IfExp* node) {
    // Conditional expression: a if b else c
    compile_expr(node->test().get());
    
    int jump_to_else = emit_jump(Opcode::POP_JUMP_IF_FALSE);
    
    compile_expr(node->body().get());
    int jump_to_end = emit_jump(Opcode::JUMP_FORWARD);
    
    patch_jump(jump_to_else);
    compile_expr(node->orelse().get());
    
    patch_jump(jump_to_end);
}

void BytecodeCompiler::compile_lambda(ast::Lambda* node) {
    // Create lambda function
    push_scope(ScopeType::Function, "<lambda>");
    
    for (const auto& arg : node->args()) {
        code().add_varname(arg);  // Lambda args are just strings
    }
    code().co_argcount = static_cast<int>(node->args().size());
    
    compile_expr(node->body().get());
    emit(Opcode::RETURN_VALUE);
    
    code().calculate_stacksize();
    code().assemble();
    
    auto lambda_code = pop_scope();
    
    int code_idx = code().add_const(lambda_code);
    emit(Opcode::LOAD_CONST, code_idx);
    emit(Opcode::MAKE_FUNCTION, 0);
}

void BytecodeCompiler::compile_listcomp(ast::ListComp* node) {
    // Simplified list comprehension
    // Full implementation would create a nested function
    emit(Opcode::BUILD_LIST, 0);
    
    // For each generator, create a loop
    // This is a simplified version
    add_error("List comprehension compilation not fully implemented", node->lineno());
}

void BytecodeCompiler::compile_await(ast::Await* node) {
    compile_expr(node->value().get());
    emit(Opcode::GET_AWAITABLE);
    emit(Opcode::LOAD_CONST, 0);  // None
    emit(Opcode::YIELD_VALUE);
}

void BytecodeCompiler::compile_yield(ast::Yield* node) {
    if (node->value()) {
        compile_expr(node->value().get());
    } else {
        emit(Opcode::LOAD_CONST, 0);  // None
    }
    emit(Opcode::YIELD_VALUE);
}

void BytecodeCompiler::compile_namedexpr(ast::NamedExpr* node) {
    // Walrus operator: (x := value)
    compile_expr(node->value().get());
    emit(Opcode::COPY, 1);  // Duplicate value
    // Target should be a Name node
    if (auto* name = dynamic_cast<ast::Name*>(node->target().get())) {
        store_name(name->id());
    } else {
        add_error("Invalid walrus operator target", node->lineno());
    }
}

void BytecodeCompiler::compile_starred(ast::Starred* node) {
    compile_expr(node->value().get());
    // Starred is handled by the context (unpacking, etc.)
}

void BytecodeCompiler::compile_joinedstr(ast::JoinedStr* node) {
    // f-string
    for (const auto& value : node->values()) {
        compile_expr(value.get());
    }
    emit(Opcode::BUILD_STRING, static_cast<int>(node->values().size()));
}

void BytecodeCompiler::compile_formattedvalue(ast::FormattedValue* node) {
    compile_expr(node->value().get());
    
    // Handle conversion
    if (node->conversion() != -1) {
        emit(Opcode::CONVERT_VALUE, node->conversion());
    }
    
    // Handle format spec
    if (node->format_spec()) {
        compile_expr(node->format_spec().get());
        emit(Opcode::FORMAT_WITH_SPEC);
    } else {
        emit(Opcode::FORMAT_SIMPLE);
    }
}

// === Target Compilation ===

void BytecodeCompiler::compile_store_target(ast::Expr* target) {
    if (auto* name = dynamic_cast<ast::Name*>(target)) {
        store_name(name->id());
    } else if (auto* attr = dynamic_cast<ast::Attribute*>(target)) {
        compile_expr(attr->value().get());
        emit(Opcode::STORE_ATTR, code().add_name(attr->attr()));
    } else if (auto* subscr = dynamic_cast<ast::Subscript*>(target)) {
        compile_expr(subscr->value().get());
        compile_expr(subscr->slice().get());
        emit(Opcode::STORE_SUBSCR);
    } else if (auto* tuple = dynamic_cast<ast::Tuple*>(target)) {
        emit(Opcode::UNPACK_SEQUENCE, static_cast<int>(tuple->elts().size()));
        for (const auto& elt : tuple->elts()) {
            compile_store_target(elt.get());
        }
    } else if (auto* list = dynamic_cast<ast::List*>(target)) {
        emit(Opcode::UNPACK_SEQUENCE, static_cast<int>(list->elts().size()));
        for (const auto& elt : list->elts()) {
            compile_store_target(elt.get());
        }
    } else if (auto* starred = dynamic_cast<ast::Starred*>(target)) {
        // Handle starred in unpacking
        compile_store_target(starred->value().get());
    } else {
        add_error("Invalid assignment target", target->lineno());
    }
}

void BytecodeCompiler::compile_delete_target(ast::Expr* target) {
    if (auto* name = dynamic_cast<ast::Name*>(target)) {
        delete_name(name->id());
    } else if (auto* attr = dynamic_cast<ast::Attribute*>(target)) {
        compile_expr(attr->value().get());
        emit(Opcode::DELETE_ATTR, code().add_name(attr->attr()));
    } else if (auto* subscr = dynamic_cast<ast::Subscript*>(target)) {
        compile_expr(subscr->value().get());
        compile_expr(subscr->slice().get());
        emit(Opcode::DELETE_SUBSCR);
    } else {
        add_error("Invalid delete target", target->lineno());
    }
}

// === Helper Methods ===

BinaryOpCode BytecodeCompiler::get_binop_code(ast::Operator op) {
    switch (op) {
        case ast::Operator::Add: return BinaryOpCode::NB_ADD;
        case ast::Operator::Sub: return BinaryOpCode::NB_SUBTRACT;
        case ast::Operator::Mult: return BinaryOpCode::NB_MULTIPLY;
        case ast::Operator::MatMult: return BinaryOpCode::NB_MATRIX_MULTIPLY;
        case ast::Operator::Div: return BinaryOpCode::NB_TRUE_DIVIDE;
        case ast::Operator::FloorDiv: return BinaryOpCode::NB_FLOOR_DIVIDE;
        case ast::Operator::Mod: return BinaryOpCode::NB_REMAINDER;
        case ast::Operator::Pow: return BinaryOpCode::NB_POWER;
        case ast::Operator::LShift: return BinaryOpCode::NB_LSHIFT;
        case ast::Operator::RShift: return BinaryOpCode::NB_RSHIFT;
        case ast::Operator::BitOr: return BinaryOpCode::NB_OR;
        case ast::Operator::BitXor: return BinaryOpCode::NB_XOR;
        case ast::Operator::BitAnd: return BinaryOpCode::NB_AND;
        default: return BinaryOpCode::NB_ADD;
    }
}

BinaryOpCode BytecodeCompiler::get_inplace_binop_code(ast::Operator op) {
    switch (op) {
        case ast::Operator::Add: return BinaryOpCode::NB_INPLACE_ADD;
        case ast::Operator::Sub: return BinaryOpCode::NB_INPLACE_SUBTRACT;
        case ast::Operator::Mult: return BinaryOpCode::NB_INPLACE_MULTIPLY;
        case ast::Operator::MatMult: return BinaryOpCode::NB_INPLACE_MATRIX_MULTIPLY;
        case ast::Operator::Div: return BinaryOpCode::NB_INPLACE_TRUE_DIVIDE;
        case ast::Operator::FloorDiv: return BinaryOpCode::NB_INPLACE_FLOOR_DIVIDE;
        case ast::Operator::Mod: return BinaryOpCode::NB_INPLACE_REMAINDER;
        case ast::Operator::Pow: return BinaryOpCode::NB_INPLACE_POWER;
        case ast::Operator::LShift: return BinaryOpCode::NB_INPLACE_LSHIFT;
        case ast::Operator::RShift: return BinaryOpCode::NB_INPLACE_RSHIFT;
        case ast::Operator::BitOr: return BinaryOpCode::NB_INPLACE_OR;
        case ast::Operator::BitXor: return BinaryOpCode::NB_INPLACE_XOR;
        case ast::Operator::BitAnd: return BinaryOpCode::NB_INPLACE_AND;
        default: return BinaryOpCode::NB_INPLACE_ADD;
    }
}

CompareOpCode BytecodeCompiler::get_compare_code(ast::CompareOp op) {
    switch (op) {
        case ast::CompareOp::Lt: return CompareOpCode::LT;
        case ast::CompareOp::LtE: return CompareOpCode::LE;
        case ast::CompareOp::Eq: return CompareOpCode::EQ;
        case ast::CompareOp::NotEq: return CompareOpCode::NE;
        case ast::CompareOp::Gt: return CompareOpCode::GT;
        case ast::CompareOp::GtE: return CompareOpCode::GE;
        default: return CompareOpCode::EQ;
    }
}

int BytecodeCompiler::lookup_name(const std::string& name) {
    // Check locals first (for functions)
    if (current_scope().type == ScopeType::Function) {
        auto it = current_scope().locals.find(name);
        if (it != current_scope().locals.end()) {
            return it->second;
        }
    }
    return -1;
}

void BytecodeCompiler::store_name(const std::string& name) {
    if (current_scope().type == ScopeType::Function) {
        if (current_scope().globals.count(name)) {
            emit(Opcode::STORE_GLOBAL, code().add_name(name));
        } else {
            // Add to locals if not already there
            int idx = code().find_varname(name);
            if (idx < 0) {
                idx = code().add_varname(name);
                current_scope().locals[name] = idx;
            }
            emit(Opcode::STORE_FAST, idx);
        }
    } else {
        emit(Opcode::STORE_NAME, code().add_name(name));
    }
}

void BytecodeCompiler::delete_name(const std::string& name) {
    if (current_scope().type == ScopeType::Function) {
        if (current_scope().globals.count(name)) {
            emit(Opcode::DELETE_GLOBAL, code().add_name(name));
        } else {
            int idx = code().find_varname(name);
            if (idx >= 0) {
                emit(Opcode::DELETE_FAST, idx);
            } else {
                emit(Opcode::DELETE_NAME, code().add_name(name));
            }
        }
    } else {
        emit(Opcode::DELETE_NAME, code().add_name(name));
    }
}

void BytecodeCompiler::add_error(const std::string& msg, int lineno) {
    std::ostringstream oss;
    oss << filename_ << ":" << lineno << ": " << msg;
    errors_.push_back(oss.str());
}

} // namespace compiler
} // namespace cpython_cpp
