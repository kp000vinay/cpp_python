#ifndef CPYTHON_CPP_COMPILER_BYTECODE_COMPILER_HPP
#define CPYTHON_CPP_COMPILER_BYTECODE_COMPILER_HPP

#include "opcode.hpp"
#include "code_object.hpp"
#include "../ast/node.hpp"
#include "../ast/expr.hpp"
#include "../ast/stmt.hpp"
#include "../ast/module.hpp"
#include <memory>
#include <string>
#include <vector>
#include <stack>
#include <unordered_map>
#include <sstream>
#include <algorithm>

namespace cpython_cpp {
namespace compiler {

/**
 * Scope types for variable resolution
 */
enum class ScopeType {
    Module,
    Function,
    Class,
    Comprehension
};

/**
 * Compiler scope - tracks variables and their locations
 */
struct CompilerScope {
    ScopeType type;
    std::string name;
    std::shared_ptr<CodeObject> code;
    
    // Variable tracking
    std::unordered_map<std::string, int> locals;   // Local variables
    std::unordered_map<std::string, int> globals;  // Global declarations
    std::unordered_map<std::string, int> nonlocals; // Nonlocal declarations
    
    // Loop tracking for break/continue
    struct LoopInfo {
        int start_offset;
        std::vector<int> break_patches;   // Instruction indices to patch for break
        std::vector<int> continue_patches; // Instruction indices to patch for continue
    };
    std::stack<LoopInfo> loops;
    
    CompilerScope(ScopeType t, const std::string& n)
        : type(t), name(n), code(std::make_shared<CodeObject>()) {
        code->co_name = n;
    }
};

/**
 * BytecodeCompiler - compiles AST to Python-compatible bytecode
 * 
 * This is the main compiler class that traverses the AST and generates
 * bytecode instructions matching CPython's format.
 */
class BytecodeCompiler {
public:
    BytecodeCompiler() : current_lineno_(1) {}
    ~BytecodeCompiler() = default;
    
    /**
     * Compile a module AST to a code object
     */
    std::shared_ptr<CodeObject> compile(const ast::Module& module,
                                        const std::string& filename = "<module>") {
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
    
    /**
     * Compile a single statement
     */
    void compile_stmt(ast::Stmt* stmt) {
        if (!stmt) return;
        
        current_lineno_ = stmt->lineno();
        
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
        } else if (auto* node = dynamic_cast<ast::AsyncFor*>(stmt)) {
            compile_async_for(node);
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
        } else if (auto* node = dynamic_cast<ast::With*>(stmt)) {
            compile_with(node);
        } else if (auto* node = dynamic_cast<ast::AsyncWith*>(stmt)) {
            compile_async_with(node);
        } else if (auto* node = dynamic_cast<ast::TypeAlias*>(stmt)) {
            compile_typealias(node);
        } else {
            add_error("Unknown statement type", stmt->lineno());
        }
    }
    
    /**
     * Compile an expression
     */
    void compile_expr(ast::Expr* expr) {
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
        } else if (auto* node = dynamic_cast<ast::SetComp*>(expr)) {
            compile_setcomp(node);
        } else if (auto* node = dynamic_cast<ast::DictComp*>(expr)) {
            compile_dictcomp(node);
        } else if (auto* node = dynamic_cast<ast::GeneratorExp*>(expr)) {
            compile_generatorexp(node);
        } else if (auto* node = dynamic_cast<ast::Await*>(expr)) {
            compile_await(node);
        } else if (auto* node = dynamic_cast<ast::Yield*>(expr)) {
            compile_yield(node);
        } else if (auto* node = dynamic_cast<ast::YieldFrom*>(expr)) {
            compile_yield_from(node);
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
    
    /**
     * Get compilation errors
     */
    const std::vector<std::string>& errors() const { return errors_; }
    
    /**
     * Check if compilation had errors
     */
    bool has_errors() const { return !errors_.empty(); }

private:
    // === Scope Management ===
    std::stack<CompilerScope> scopes_;
    
    CompilerScope& current_scope() { return scopes_.top(); }
    const CompilerScope& current_scope() const { return scopes_.top(); }
    CodeObject& code() { return *current_scope().code; }
    
    void push_scope(ScopeType type, const std::string& name) {
        scopes_.push(CompilerScope(type, name));
        
        if (type == ScopeType::Function) {
            code().co_flags |= CodeFlags::CO_OPTIMIZED | CodeFlags::CO_NEWLOCALS;
        }
    }
    
    std::shared_ptr<CodeObject> pop_scope() {
        auto code_obj = current_scope().code;
        scopes_.pop();
        return code_obj;
    }
    
    // === Code Generation ===
    void emit(Opcode op, int arg = -1) {
        code().emit(op, arg, current_lineno_);
    }
    
    void emit_with_lineno(Opcode op, int arg, int lineno) {
        code().emit(op, arg, lineno);
    }
    
    int emit_jump(Opcode op) {
        int index = static_cast<int>(code().instructions.size());
        emit(op, 0);
        return index;
    }
    
    void patch_jump(int instr_index) {
        int target = code().current_offset();
        code().instructions[instr_index].arg = target;
    }
    
    void patch_jump_to(int instr_index, int target) {
        code().instructions[instr_index].arg = target;
    }
    
    // === Statement Compilation ===
    void compile_function_def(ast::FunctionDef* node) {
        push_scope(ScopeType::Function, node->name());
        
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
    
    void compile_async_function_def(ast::AsyncFunctionDef* node) {
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
    
    void compile_class_def(ast::ClassDef* node) {
        emit(Opcode::LOAD_BUILD_CLASS);
        
        push_scope(ScopeType::Class, node->name());
        code().co_flags |= CodeFlags::CO_NEWLOCALS;
        
        emit(Opcode::LOAD_NAME, code().add_name("__name__"));
        emit(Opcode::STORE_NAME, code().add_name("__module__"));
        
        int qualname_idx = code().add_const(node->name());
        emit(Opcode::LOAD_CONST, qualname_idx);
        emit(Opcode::STORE_NAME, code().add_name("__qualname__"));
        
        for (const auto& stmt : node->body()) {
            compile_stmt(stmt.get());
        }
        
        emit(Opcode::LOAD_CONST, code().add_const(std::monostate{}));
        emit(Opcode::RETURN_VALUE);
        
        code().calculate_stacksize();
        code().assemble();
        
        auto class_code = pop_scope();
        
        int code_idx = code().add_const(class_code);
        emit(Opcode::LOAD_CONST, code_idx);
        emit(Opcode::MAKE_FUNCTION, 0);
        
        int name_idx = code().add_const(node->name());
        emit(Opcode::LOAD_CONST, name_idx);
        
        for (const auto& base : node->bases()) {
            compile_expr(base.get());
        }
        
        emit(Opcode::CALL, 2 + static_cast<int>(node->bases().size()));
        
        store_name(node->name());
    }
    
    void compile_return(ast::Return* node) {
        if (node->value()) {
            compile_expr(node->value().get());
        } else {
            emit(Opcode::LOAD_CONST, 0);
        }
        emit(Opcode::RETURN_VALUE);
    }
    
    void compile_assign(ast::Assign* node) {
        compile_expr(node->value().get());
        
        const auto& targets = node->targets();
        for (size_t i = 0; i < targets.size(); ++i) {
            if (i < targets.size() - 1) {
                emit(Opcode::COPY, 1);
            }
            compile_store_target(targets[i].get());
        }
    }
    
    void compile_ann_assign(ast::AnnAssign* node) {
        if (node->value()) {
            compile_expr(node->value().get());
            compile_store_target(node->target().get());
        }
    }
    
    void compile_aug_assign(ast::AugAssign* node) {
        compile_expr(node->target().get());
        compile_expr(node->value().get());
        
        BinaryOpCode op_code = get_inplace_binop_code(node->op());
        emit(Opcode::BINARY_OP, static_cast<int>(op_code));
        
        compile_store_target(node->target().get());
    }
    
    void compile_if(ast::If* node) {
        compile_expr(node->test().get());
        
        int jump_to_else = emit_jump(Opcode::POP_JUMP_IF_FALSE);
        
        for (const auto& stmt : node->body()) {
            compile_stmt(stmt.get());
        }
        
        if (!node->orelse().empty()) {
            int jump_to_end = emit_jump(Opcode::JUMP_FORWARD);
            
            patch_jump(jump_to_else);
            
            for (const auto& stmt : node->orelse()) {
                compile_stmt(stmt.get());
            }
            
            patch_jump(jump_to_end);
        } else {
            patch_jump(jump_to_else);
        }
    }
    
    void compile_while(ast::While* node) {
        int loop_start = code().current_offset();
        
        current_scope().loops.push({loop_start, {}, {}});
        
        compile_expr(node->test().get());
        
        int jump_to_end = emit_jump(Opcode::POP_JUMP_IF_FALSE);
        
        for (const auto& stmt : node->body()) {
            compile_stmt(stmt.get());
        }
        
        emit(Opcode::JUMP_BACKWARD, code().current_offset() - loop_start);
        
        patch_jump(jump_to_end);
        
        if (!node->orelse().empty()) {
            for (const auto& stmt : node->orelse()) {
                compile_stmt(stmt.get());
            }
        }
        
        auto& loop_info = current_scope().loops.top();
        for (int idx : loop_info.break_patches) {
            patch_jump(idx);
        }
        
        current_scope().loops.pop();
    }
    
    void compile_for(ast::For* node) {
        compile_expr(node->iter().get());
        
        emit(Opcode::GET_ITER);
        
        int loop_start = code().current_offset();
        
        current_scope().loops.push({loop_start, {}, {}});
        
        int for_iter = emit_jump(Opcode::FOR_ITER);
        
        compile_store_target(node->target().get());
        
        for (const auto& stmt : node->body()) {
            compile_stmt(stmt.get());
        }
        
        emit(Opcode::JUMP_BACKWARD, code().current_offset() - loop_start);
        
        patch_jump(for_iter);
        
        emit(Opcode::END_FOR);
        
        if (!node->orelse().empty()) {
            for (const auto& stmt : node->orelse()) {
                compile_stmt(stmt.get());
            }
        }
        
        auto& loop_info = current_scope().loops.top();
        for (int idx : loop_info.break_patches) {
            patch_jump(idx);
        }
        
        current_scope().loops.pop();
    }
    
    /**
     * Compile an async for loop
     * 
     * Python: async for TARGET in ITER:
     *             BODY
     *         else:
     *             ORELSE
     * 
     * Bytecode pattern (CPython 3.11+):
     *   ITER                    # Evaluate async iterable
     *   GET_AITER               # Get async iterator
     * loop_start:
     *   GET_ANEXT               # Get next awaitable
     *   LOAD_CONST None
     *   SEND to_store           # Await the next value
     *   YIELD_VALUE             # Yield control
     *   RESUME 3
     *   JUMP_BACKWARD to SEND
     * to_store:
     *   STORE TARGET            # Store the value
     *   BODY                    # Execute body
     *   JUMP_BACKWARD loop_start
     * end:
     *   END_ASYNC_FOR           # Cleanup async iterator
     *   ORELSE                  # Execute else clause (if any)
     */
    void compile_async_for(ast::AsyncFor* node) {
        // Evaluate the async iterable
        compile_expr(node->iter().get());
        
        // GET_AITER: Get async iterator from the iterable
        emit(Opcode::GET_AITER);
        
        // Mark the start of the loop
        int loop_start = code().current_offset();
        
        // Push loop context for break/continue handling
        current_scope().loops.push({loop_start, {}, {}});
        
        // GET_ANEXT: Get next awaitable from async iterator
        emit(Opcode::GET_ANEXT);
        
        // Load None for SEND
        emit(Opcode::LOAD_CONST, code().add_const(std::monostate{}));
        
        // SEND: Await the next value (jumps to store on completion)
        int send_jump = emit_jump(Opcode::SEND);
        
        // YIELD_VALUE: Yield control back to event loop
        emit(Opcode::YIELD_VALUE);
        
        // RESUME 3: Resume after yield (async for context)
        emit(Opcode::RESUME, 3);
        
        // Jump back to SEND to continue awaiting
        emit(Opcode::JUMP_BACKWARD_NO_INTERRUPT, code().current_offset() - (send_jump - 2));
        
        // Patch SEND to jump here when value is ready
        patch_jump(send_jump);
        
        // Store the yielded value in the target
        compile_store_target(node->target().get());
        
        // Compile the loop body
        for (const auto& stmt : node->body()) {
            compile_stmt(stmt.get());
        }
        
        // Jump back to GET_ANEXT to get next value
        emit(Opcode::JUMP_BACKWARD, code().current_offset() - loop_start);
        
        // END_ASYNC_FOR: Cleanup when StopAsyncIteration is raised
        emit(Opcode::END_ASYNC_FOR);
        
        // Compile else clause (executed if loop completes normally)
        if (!node->orelse().empty()) {
            for (const auto& stmt : node->orelse()) {
                compile_stmt(stmt.get());
            }
        }
        
        // Patch break statements to jump here
        auto& loop_info = current_scope().loops.top();
        for (int idx : loop_info.break_patches) {
            patch_jump(idx);
        }
        
        current_scope().loops.pop();
    }
    
    void compile_try(ast::Try* node) {
        emit(Opcode::PUSH_EXC_INFO);
        
        for (const auto& stmt : node->body()) {
            compile_stmt(stmt.get());
        }
        
        int jump_to_end = emit_jump(Opcode::JUMP_FORWARD);
        
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
                emit(Opcode::POP_TOP);
                for (const auto& stmt : handler->body()) {
                    compile_stmt(stmt.get());
                }
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
    
    void compile_try_star(ast::TryStar* node) {
        emit(Opcode::PUSH_EXC_INFO);
        
        for (const auto& stmt : node->body()) {
            compile_stmt(stmt.get());
        }
        
        int jump_to_end = emit_jump(Opcode::JUMP_FORWARD);
        
        for (const auto& handler : node->handlers()) {
            if (handler->type()) {
                compile_expr(handler->type().get());
                emit(Opcode::CHECK_EG_MATCH);
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
    
    void compile_import(ast::Import* node) {
        for (const auto& alias : node->names()) {
            emit(Opcode::LOAD_CONST, code().add_const(int64_t(0)));
            emit(Opcode::LOAD_CONST, 0);
            
            int name_idx = code().add_name(alias->name());
            emit(Opcode::IMPORT_NAME, name_idx);
            
            const std::string& store_name_str = alias->asname().empty() ? 
                alias->name() : alias->asname();
            store_name(store_name_str);
        }
    }
    
    void compile_import_from(ast::ImportFrom* node) {
        emit(Opcode::LOAD_CONST, code().add_const(int64_t(node->level())));
        emit(Opcode::LOAD_CONST, 0);
        
        int name_idx = code().add_name(node->module());
        emit(Opcode::IMPORT_NAME, name_idx);
        
        for (const auto& alias : node->names()) {
            int from_idx = code().add_name(alias->name());
            emit(Opcode::IMPORT_FROM, from_idx);
            
            const std::string& store_name_str = alias->asname().empty() ?
                alias->name() : alias->asname();
            store_name(store_name_str);
        }
        
        emit(Opcode::POP_TOP);
    }
    
    void compile_global(ast::Global* node) {
        for (const auto& name : node->names()) {
            current_scope().globals[name] = 1;
        }
    }
    
    void compile_nonlocal(ast::Nonlocal* node) {
        for (const auto& name : node->names()) {
            current_scope().nonlocals[name] = 1;
        }
    }
    
    void compile_expr_stmt(ast::ExprStmt* node) {
        compile_expr(node->value().get());
        emit(Opcode::POP_TOP);
    }
    
    void compile_pass(ast::Pass* node) {
        emit(Opcode::NOP);
    }
    
    void compile_break(ast::Break* node) {
        if (current_scope().loops.empty()) {
            add_error("'break' outside loop", node->lineno());
            return;
        }
        
        int jump_idx = emit_jump(Opcode::JUMP_FORWARD);
        current_scope().loops.top().break_patches.push_back(jump_idx);
    }
    
    void compile_continue(ast::Continue* node) {
        if (current_scope().loops.empty()) {
            add_error("'continue' outside loop", node->lineno());
            return;
        }
        
        auto& loop = current_scope().loops.top();
        emit(Opcode::JUMP_BACKWARD, code().current_offset() - loop.start_offset);
    }
    
    void compile_raise(ast::Raise* node) {
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
    
    void compile_assert(ast::Assert* node) {
        compile_expr(node->test().get());
        
        int jump_if_true = emit_jump(Opcode::POP_JUMP_IF_TRUE);
        
        emit(Opcode::LOAD_GLOBAL, code().add_name("AssertionError"));
        
        if (node->msg()) {
            compile_expr(node->msg().get());
            emit(Opcode::CALL, 1);
        }
        
        emit(Opcode::RAISE_VARARGS, 1);
        
        patch_jump(jump_if_true);
    }
    
    void compile_delete(ast::Delete* node) {
        for (const auto& target : node->targets()) {
            compile_delete_target(target.get());
        }
    }
    
    void compile_match(ast::Match* node) {
        compile_expr(node->subject().get());
        
        for (const auto& case_ : node->cases()) {
            emit(Opcode::COPY, 1);
            
            for (const auto& stmt : case_.body()) {
                compile_stmt(stmt.get());
            }
        }
        
        emit(Opcode::POP_TOP);
    }
    
    /**
     * Compile a with statement (context manager)
     * 
     * Python: with EXPR as VAR:
     *             BODY
     * 
     * Bytecode pattern:
     *   EXPR                    # Evaluate context expression
     *   BEFORE_WITH             # Call __enter__, push __exit__
     *   STORE VAR               # Store result of __enter__ (if 'as' clause)
     *   BODY                    # Execute body
     *   LOAD_CONST None         # Normal exit: call __exit__(None, None, None)
     *   LOAD_CONST None
     *   LOAD_CONST None
     *   CALL 2
     *   POP_TOP
     *   JUMP cleanup
     * exception_handler:
     *   PUSH_EXC_INFO           # Exception: call __exit__(exc_type, exc_val, exc_tb)
     *   WITH_EXCEPT_START
     *   POP_JUMP_IF_TRUE suppress
     *   RERAISE
     * suppress:
     *   POP_TOP
     *   POP_EXCEPT
     *   POP_TOP
     *   POP_TOP
     * cleanup:
     */
    void compile_with(ast::With* node) {
        // For simplicity, we compile a basic version without full exception handling
        // A complete implementation would need exception table support
        
        for (const auto& item : node->items()) {
            // Evaluate context expression
            compile_expr(item.context_expr.get());
            
            // BEFORE_WITH: calls __enter__ and pushes __exit__ for later
            emit(Opcode::BEFORE_WITH);
            
            // Store the result of __enter__ if there's an 'as' clause
            if (item.optional_vars) {
                compile_store_target(item.optional_vars.get());
            } else {
                emit(Opcode::POP_TOP);
            }
        }
        
        // Compile the body
        for (const auto& stmt : node->body()) {
            compile_stmt(stmt.get());
        }
        
        // Normal exit: call __exit__(None, None, None) for each context manager (reverse order)
        for (size_t i = node->items().size(); i > 0; --i) {
            emit(Opcode::LOAD_CONST, code().add_const(std::monostate{}));  // None
            emit(Opcode::LOAD_CONST, code().add_const(std::monostate{}));  // None
            emit(Opcode::LOAD_CONST, code().add_const(std::monostate{}));  // None
            emit(Opcode::CALL, 2);  // Call __exit__ with 3 args (2 on stack after callable)
            emit(Opcode::POP_TOP);  // Discard __exit__ return value
        }
    }
    
    /**
     * Compile an async with statement
     * 
     * Similar to with statement but uses __aenter__ and __aexit__
     * and requires await on both calls.
     */
    void compile_async_with(ast::AsyncWith* node) {
        for (const auto& item : node->items()) {
            // Evaluate context expression
            compile_expr(item.context_expr.get());
            
            // BEFORE_ASYNC_WITH: calls __aenter__ and pushes __aexit__
            emit(Opcode::BEFORE_ASYNC_WITH);
            emit(Opcode::GET_AWAITABLE, 1);
            emit(Opcode::LOAD_CONST, code().add_const(std::monostate{}));
            emit(Opcode::SEND, 0);  // Will be patched
            emit(Opcode::POP_TOP);
            
            // Store the result of __aenter__ if there's an 'as' clause
            if (item.optional_vars) {
                compile_store_target(item.optional_vars.get());
            } else {
                emit(Opcode::POP_TOP);
            }
        }
        
        // Compile the body
        for (const auto& stmt : node->body()) {
            compile_stmt(stmt.get());
        }
        
        // Normal exit: call __aexit__(None, None, None) for each context manager
        for (size_t i = node->items().size(); i > 0; --i) {
            emit(Opcode::LOAD_CONST, code().add_const(std::monostate{}));  // None
            emit(Opcode::LOAD_CONST, code().add_const(std::monostate{}));  // None
            emit(Opcode::LOAD_CONST, code().add_const(std::monostate{}));  // None
            emit(Opcode::CALL, 2);
            emit(Opcode::GET_AWAITABLE, 2);
            emit(Opcode::LOAD_CONST, code().add_const(std::monostate{}));
            emit(Opcode::SEND, 0);
            emit(Opcode::POP_TOP);
        }
    }
    
    /**
     * Compile type alias statement (PEP 695)
     * 
     * type Vector = list[int]
     * type Mapping[K, V] = dict[K, V]
     * 
     * Bytecode pattern:
     *   LOAD_CONST        <alias_name>
     *   BUILD_TUPLE       0  (or type params if generic)
     *   LOAD_CONST        <value_expression_code>
     *   MAKE_FUNCTION     0
     *   BUILD_TUPLE       3
     *   CALL_INTRINSIC_1  6  (INTRINSIC_TYPEALIAS)
     *   STORE_NAME        <alias_name>
     */
    void compile_typealias(ast::TypeAlias* node) {
        // Get the alias name
        std::string alias_name;
        if (auto* name_expr = dynamic_cast<ast::Name*>(node->name().get())) {
            alias_name = name_expr->id();
        } else {
            add_error("TypeAlias name must be a simple identifier");
            return;
        }
        
        // 1. Push the alias name
        emit(Opcode::LOAD_CONST, code().add_const(alias_name));
        
        // 2. Build type parameters tuple
        if (node->is_generic()) {
            // Compile each type parameter
            for (const auto& param : node->type_params()) {
                // Type parameters are TypeVar, ParamSpec, or TypeVarTuple
                // Get the name from the concrete type
                std::string param_name;
                if (auto* tv = dynamic_cast<ast::TypeVar*>(param.get())) {
                    param_name = tv->name();
                } else if (auto* ps = dynamic_cast<ast::ParamSpec*>(param.get())) {
                    param_name = ps->name();
                } else if (auto* tvt = dynamic_cast<ast::TypeVarTuple*>(param.get())) {
                    param_name = tvt->name();
                }
                emit(Opcode::LOAD_CONST, code().add_const(param_name));
            }
            emit(Opcode::BUILD_TUPLE, static_cast<int>(node->type_params().size()));
        } else {
            // Empty tuple for non-generic type alias
            emit(Opcode::BUILD_TUPLE, 0);
        }
        
        // 3. Create a function that returns the type value (for lazy evaluation)
        // Create a nested code object for the type alias value
        auto value_code = std::make_shared<CodeObject>(
            "<type alias value>",
            code().co_filename,
            node->lineno()
        );
        
        // Save current scope and push new scope for value code
        push_scope(ScopeType::Function, "<type alias value>");
        // Replace the code object with our custom one
        current_scope().code = value_code;
        
        // Compile the value expression
        compile_expr(node->value().get());
        emit(Opcode::RETURN_VALUE);
        
        // Pop the value scope
        pop_scope();
        
        // Push the code object as a constant in parent scope
        int code_idx = code().add_const(value_code);
        emit(Opcode::LOAD_CONST, code_idx);
        
        // Make function from the code object
        emit(Opcode::MAKE_FUNCTION, 0);
        
        // 4. Build tuple of (name, type_params, value_func)
        emit(Opcode::BUILD_TUPLE, 3);
        
        // 5. Call INTRINSIC_TYPEALIAS (intrinsic index 6)
        emit(Opcode::CALL_INTRINSIC_1, 6);
        
        // 6. Store the result
        int name_idx = code().add_name(alias_name);
        if (current_scope().type == ScopeType::Module) {
            emit(Opcode::STORE_NAME, name_idx);
        } else if (current_scope().type == ScopeType::Class) {
            emit(Opcode::STORE_NAME, name_idx);
        } else {
            // In function scope, store as fast local
            int local_idx = code().add_varname(alias_name);
            emit(Opcode::STORE_FAST, local_idx);
        }
    }
    
    // === Expression Compilation ===
    void compile_constant(ast::Constant* node) {
        const std::string& val = node->value();
        
        if (val.empty()) {
            emit(Opcode::LOAD_CONST, code().add_const(std::monostate{}));
        } else if (val == "None") {
            emit(Opcode::LOAD_CONST, code().add_const(std::monostate{}));
        } else if (val == "True") {
            emit(Opcode::LOAD_CONST, code().add_const(true));
        } else if (val == "False") {
            emit(Opcode::LOAD_CONST, code().add_const(false));
        } else if (val.front() == '\'' || val.front() == '"') {
            std::string str_val = val.substr(1, val.length() - 2);
            emit(Opcode::LOAD_CONST, code().add_const(str_val));
        } else {
            bool is_float = false;
            bool is_number = true;
            
            for (size_t i = 0; i < val.size(); ++i) {
                char c = val[i];
                if (c == '.' || c == 'e' || c == 'E') {
                    is_float = true;
                } else if (c == '-' || c == '+') {
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
                        int64_t i = std::stoll(val, nullptr, 0);
                        if (i >= 0 && i <= 255) {
                            emit(Opcode::LOAD_SMALL_INT, static_cast<int>(i));
                        } else {
                            emit(Opcode::LOAD_CONST, code().add_const(i));
                        }
                    }
                } catch (...) {
                    emit(Opcode::LOAD_CONST, code().add_const(val));
                }
            } else {
                emit(Opcode::LOAD_CONST, code().add_const(val));
            }
        }
    }
    
    void compile_name(ast::Name* node) {
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
    
    void compile_binop(ast::BinOp* node) {
        compile_expr(node->left().get());
        compile_expr(node->right().get());
        
        BinaryOpCode op_code = get_binop_code(node->op());
        emit(Opcode::BINARY_OP, static_cast<int>(op_code));
    }
    
    void compile_unaryop(ast::UnaryOp* node) {
        compile_expr(node->operand().get());
        
        switch (node->op()) {
            case ast::UnaryOp::UnaryOpType::Not:
                emit(Opcode::UNARY_NOT);
                break;
            case ast::UnaryOp::UnaryOpType::UAdd:
                break;
            case ast::UnaryOp::UnaryOpType::USub:
                emit(Opcode::UNARY_NEGATIVE);
                break;
            case ast::UnaryOp::UnaryOpType::Invert:
                emit(Opcode::UNARY_INVERT);
                break;
        }
    }
    
    void compile_boolop(ast::BoolOpExpr* node) {
        const auto& values = node->values();
        
        if (node->op() == ast::BoolOp::And) {
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
    
    void compile_compare(ast::Compare* node) {
        compile_expr(node->left().get());
        compile_expr(node->right().get());
        
        CompareOpCode op_code = get_compare_code(node->op());
        emit(Opcode::COMPARE_OP, static_cast<int>(op_code));
    }
    
    void compile_call(ast::Call* node) {
        compile_expr(node->func().get());
        
        for (const auto& arg : node->args()) {
            compile_expr(arg.get());
        }
        
        emit(Opcode::CALL, static_cast<int>(node->args().size()));
    }
    
    void compile_attribute(ast::Attribute* node) {
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
    
    void compile_subscript(ast::Subscript* node) {
        compile_expr(node->value().get());
        compile_expr(node->slice().get());
        
        switch (node->ctx()) {
            case ast::ExprContext::Load:
                emit(Opcode::BINARY_OP, static_cast<int>(BinaryOpCode::NB_ADD));
                break;
            case ast::ExprContext::Store:
                emit(Opcode::STORE_SUBSCR);
                break;
            case ast::ExprContext::Del:
                emit(Opcode::DELETE_SUBSCR);
                break;
        }
    }
    
    void compile_slice(ast::Slice* node) {
        if (node->lower()) {
            compile_expr(node->lower().get());
        } else {
            emit(Opcode::LOAD_CONST, 0);
        }
        
        if (node->upper()) {
            compile_expr(node->upper().get());
        } else {
            emit(Opcode::LOAD_CONST, 0);
        }
        
        if (node->step()) {
            compile_expr(node->step().get());
            emit(Opcode::BUILD_SLICE, 3);
        } else {
            emit(Opcode::BUILD_SLICE, 2);
        }
    }
    
    void compile_list(ast::List* node) {
        for (const auto& elt : node->elts()) {
            compile_expr(elt.get());
        }
        emit(Opcode::BUILD_LIST, static_cast<int>(node->elts().size()));
    }
    
    void compile_tuple(ast::Tuple* node) {
        for (const auto& elt : node->elts()) {
            compile_expr(elt.get());
        }
        emit(Opcode::BUILD_TUPLE, static_cast<int>(node->elts().size()));
    }
    
    void compile_dict(ast::Dict* node) {
        const auto& keys = node->keys();
        const auto& values = node->values();
        
        for (size_t i = 0; i < keys.size(); ++i) {
            compile_expr(keys[i].get());
            compile_expr(values[i].get());
        }
        emit(Opcode::BUILD_MAP, static_cast<int>(keys.size()));
    }
    
    void compile_set(ast::Set* node) {
        for (const auto& elt : node->elts()) {
            compile_expr(elt.get());
        }
        emit(Opcode::BUILD_SET, static_cast<int>(node->elts().size()));
    }
    
    void compile_ifexp(ast::IfExp* node) {
        compile_expr(node->test().get());
        
        int jump_to_else = emit_jump(Opcode::POP_JUMP_IF_FALSE);
        
        compile_expr(node->body().get());
        int jump_to_end = emit_jump(Opcode::JUMP_FORWARD);
        
        patch_jump(jump_to_else);
        compile_expr(node->orelse().get());
        
        patch_jump(jump_to_end);
    }
    
    void compile_lambda(ast::Lambda* node) {
        push_scope(ScopeType::Function, "<lambda>");
        
        for (const auto& arg : node->args()) {
            code().add_varname(arg);
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
    
    /**
     * Compile a list comprehension
     * 
     * Python: [expr for target in iter if cond]
     * 
     * CPython compiles comprehensions as nested functions that take
     * an iterator as argument. For simplicity, we inline the comprehension.
     * 
     * Bytecode pattern (inlined):
     *   BUILD_LIST 0           # Create empty list
     *   ITER                   # Evaluate iterable
     *   GET_ITER               # Get iterator
     * loop:
     *   FOR_ITER end           # Get next item or jump to end
     *   STORE TARGET           # Store in target variable
     *   [IF CONDITIONS]        # Optional: evaluate conditions
     *   [POP_JUMP_IF_FALSE loop] # Skip if condition false
     *   EXPR                   # Evaluate element expression
     *   LIST_APPEND 2          # Append to list (2 = stack depth)
     *   JUMP_BACKWARD loop
     * end:
     *   (list is on stack)
     */
    void compile_listcomp(ast::ListComp* node) {
        // Build empty list
        emit(Opcode::BUILD_LIST, 0);
        
        // Compile the comprehension generators
        compile_comprehension_generators(
            node->generators(),
            [this, node]() {
                // Compile the element expression
                compile_expr(node->elt().get());
                // Append to list (stack depth = 2: list, element)
                emit(Opcode::LIST_APPEND, 2);
            }
        );
    }
    
    /**
     * Compile a set comprehension
     * 
     * Python: {expr for target in iter if cond}
     * 
     * Same pattern as list comprehension but uses BUILD_SET and SET_ADD
     */
    void compile_setcomp(ast::SetComp* node) {
        // Build empty set
        emit(Opcode::BUILD_SET, 0);
        
        // Compile the comprehension generators
        compile_comprehension_generators(
            node->generators(),
            [this, node]() {
                // Compile the element expression
                compile_expr(node->elt().get());
                // Add to set (stack depth = 2: set, element)
                emit(Opcode::SET_ADD, 2);
            }
        );
    }
    
    /**
     * Compile a dictionary comprehension
     * 
     * Python: {key: value for target in iter if cond}
     * 
     * Same pattern but uses BUILD_MAP and MAP_ADD
     */
    void compile_dictcomp(ast::DictComp* node) {
        // Build empty dict
        emit(Opcode::BUILD_MAP, 0);
        
        // Compile the comprehension generators
        compile_comprehension_generators(
            node->generators(),
            [this, node]() {
                // Compile key and value expressions
                compile_expr(node->key().get());
                compile_expr(node->value().get());
                // Add to map (stack depth = 2: map, key, value)
                emit(Opcode::MAP_ADD, 2);
            }
        );
    }
    
    /**
     * Compile a generator expression
     * 
     * Python: (expr for target in iter if cond)
     * 
     * Generator expressions create a generator function that yields values.
     * We compile it as a nested code object.
     */
    void compile_generatorexp(ast::GeneratorExp* node) {
        // For now, compile as a simple inline generator
        // A full implementation would create a nested code object
        
        // Create a code object for the generator
        std::string gen_name = "<genexpr>";
        auto gen_code = std::make_shared<CodeObject>(gen_name, code().co_filename, node->lineno());
        gen_code->co_flags = 0x20;  // CO_GENERATOR
        
        // Push the generator code object
        int code_idx = code().add_const(gen_code);
        emit(Opcode::LOAD_CONST, code_idx);
        emit(Opcode::MAKE_FUNCTION, 0);
        
        // Evaluate the first iterator and pass it to the generator
        if (!node->generators().empty()) {
            compile_expr(node->generators()[0].iter.get());
            emit(Opcode::GET_ITER);
            emit(Opcode::CALL, 1);
        } else {
            emit(Opcode::CALL, 0);
        }
    }
    
    /**
     * Helper function to compile comprehension generators (for loops and conditions)
     * 
     * This handles nested for loops and if conditions in comprehensions.
     * The body_emitter is called when we're inside all the loops and conditions.
     */
    template<typename BodyEmitter>
    void compile_comprehension_generators(
        const std::vector<ast::Comprehension>& generators,
        BodyEmitter body_emitter,
        size_t gen_index = 0
    ) {
        if (gen_index >= generators.size()) {
            // All generators processed, emit the body
            body_emitter();
            return;
        }
        
        const auto& gen = generators[gen_index];
        
        // Compile the iterable (only for first generator, others use outer scope)
        if (gen_index == 0) {
            compile_expr(gen.iter.get());
            if (gen.is_async) {
                emit(Opcode::GET_AITER);
            } else {
                emit(Opcode::GET_ITER);
            }
        } else {
            // For nested generators, compile the iterable normally
            compile_expr(gen.iter.get());
            if (gen.is_async) {
                emit(Opcode::GET_AITER);
            } else {
                emit(Opcode::GET_ITER);
            }
        }
        
        // Mark loop start
        int loop_start = code().current_offset();
        
        // FOR_ITER: Get next item or jump to end
        int for_iter_jump;
        if (gen.is_async) {
            // Async for: GET_ANEXT, LOAD_CONST None, SEND
            emit(Opcode::GET_ANEXT);
            emit(Opcode::LOAD_CONST, code().add_const(std::monostate{}));
            for_iter_jump = emit_jump(Opcode::SEND);
            emit(Opcode::YIELD_VALUE);
            emit(Opcode::RESUME, 3);
            emit(Opcode::JUMP_BACKWARD_NO_INTERRUPT, code().current_offset() - loop_start - 2);
            patch_jump(for_iter_jump);
        } else {
            for_iter_jump = emit_jump(Opcode::FOR_ITER);
        }
        
        // Store the iteration variable
        compile_store_target(gen.target.get());
        
        // Compile if conditions
        std::vector<int> condition_jumps;
        for (const auto& cond : gen.ifs) {
            compile_expr(cond.get());
            // Jump back to loop start if condition is false
            int cond_jump = emit_jump(Opcode::POP_JUMP_IF_FALSE);
            condition_jumps.push_back(cond_jump);
        }
        
        // Recursively compile remaining generators or emit body
        compile_comprehension_generators(generators, body_emitter, gen_index + 1);
        
        // Patch condition jumps to continue the loop
        for (int jump : condition_jumps) {
            // Patch to jump back to loop start
            patch_jump_to(jump, loop_start);
        }
        
        // Jump back to loop start
        emit(Opcode::JUMP_BACKWARD, code().current_offset() - loop_start);
        
        // Patch FOR_ITER to jump here when exhausted
        if (!gen.is_async) {
            patch_jump(for_iter_jump);
        }
        
        // For async, emit END_ASYNC_FOR
        if (gen.is_async) {
            emit(Opcode::END_ASYNC_FOR);
        }
    }
    
    void compile_await(ast::Await* node) {
        compile_expr(node->value().get());
        emit(Opcode::GET_AWAITABLE);
        emit(Opcode::LOAD_CONST, 0);
        emit(Opcode::YIELD_VALUE);
    }
    
    /**
     * Compile a yield expression
     * 
     * Python: yield VALUE
     * 
     * Bytecode pattern:
     *   VALUE (or LOAD_CONST None if no value)
     *   YIELD_VALUE
     *   RESUME 1
     *   (result of send() is on stack)
     */
    void compile_yield(ast::Yield* node) {
        // Load the value to yield (or None if no value)
        if (node->value()) {
            compile_expr(node->value().get());
        } else {
            emit(Opcode::LOAD_CONST, code().add_const(std::monostate{}));
        }
        
        // YIELD_VALUE: Suspend and yield the value
        emit(Opcode::YIELD_VALUE);
        
        // RESUME 1: Resume point after yield (generator context)
        emit(Opcode::RESUME, 1);
        
        // The result of send() is now on the stack
        // If yield is used as a statement, caller will POP_TOP
    }
    
    /**
     * Compile a yield from expression (generator delegation)
     * 
     * Python: yield from ITERABLE
     * 
     * Bytecode pattern (CPython 3.11+):
     *   ITERABLE                    # Evaluate the iterable
     *   GET_YIELD_FROM_ITER         # Get iterator for delegation
     *   LOAD_CONST None             # Initial send value
     * loop:
     *   SEND end                    # Send value to sub-iterator
     *   YIELD_VALUE                 # Yield the value
     *   RESUME 2                    # Resume (yield from context)
     *   JUMP_BACKWARD_NO_INTERRUPT loop
     * end:
     *   POP_TOP                     # Pop the final result
     *   (result of sub-generator is on stack)
     */
    void compile_yield_from(ast::YieldFrom* node) {
        // Evaluate the iterable to delegate to
        compile_expr(node->value().get());
        
        // GET_YIELD_FROM_ITER: Get an iterator suitable for yield from
        // This handles both generators and other iterables
        emit(Opcode::GET_YIELD_FROM_ITER);
        
        // Load None as the initial value to send
        emit(Opcode::LOAD_CONST, code().add_const(std::monostate{}));
        
        // Mark the start of the send loop
        int send_start = code().current_offset();
        
        // SEND: Send value to sub-iterator, jump to end on StopIteration
        int send_jump = emit_jump(Opcode::SEND);
        
        // YIELD_VALUE: Yield the value from sub-iterator
        emit(Opcode::YIELD_VALUE);
        
        // RESUME 2: Resume point (yield from context)
        emit(Opcode::RESUME, 2);
        
        // Jump back to SEND to continue delegation
        emit(Opcode::JUMP_BACKWARD_NO_INTERRUPT, code().current_offset() - send_start);
        
        // Patch SEND to jump here when sub-iterator is exhausted
        patch_jump(send_jump);
        
        // The return value of the sub-generator is now on the stack
        // (StopIteration.value)
    }
    
    void compile_namedexpr(ast::NamedExpr* node) {
        compile_expr(node->value().get());
        emit(Opcode::COPY, 1);
        if (auto* name = dynamic_cast<ast::Name*>(node->target().get())) {
            store_name(name->id());
        } else {
            add_error("Invalid walrus operator target", node->lineno());
        }
    }
    
    void compile_starred(ast::Starred* node) {
        compile_expr(node->value().get());
    }
    
    void compile_joinedstr(ast::JoinedStr* node) {
        for (const auto& value : node->values()) {
            compile_expr(value.get());
        }
        emit(Opcode::BUILD_STRING, static_cast<int>(node->values().size()));
    }
    
    void compile_formattedvalue(ast::FormattedValue* node) {
        compile_expr(node->value().get());
        
        if (node->conversion() != -1) {
            emit(Opcode::CONVERT_VALUE, node->conversion());
        }
        
        if (node->format_spec()) {
            compile_expr(node->format_spec().get());
            emit(Opcode::FORMAT_WITH_SPEC);
        } else {
            emit(Opcode::FORMAT_SIMPLE);
        }
    }
    
    // === Target Compilation ===
    void compile_store_target(ast::Expr* target) {
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
            compile_store_target(starred->value().get());
        } else {
            add_error("Invalid assignment target", target->lineno());
        }
    }
    
    void compile_delete_target(ast::Expr* target) {
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
    BinaryOpCode get_binop_code(ast::Operator op) {
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
    
    BinaryOpCode get_inplace_binop_code(ast::Operator op) {
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
    
    CompareOpCode get_compare_code(ast::CompareOp op) {
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
    
    int lookup_name(const std::string& name) {
        if (current_scope().type == ScopeType::Function) {
            auto it = current_scope().locals.find(name);
            if (it != current_scope().locals.end()) {
                return it->second;
            }
        }
        return -1;
    }
    
    void store_name(const std::string& name) {
        if (current_scope().type == ScopeType::Function) {
            if (current_scope().globals.count(name)) {
                emit(Opcode::STORE_GLOBAL, code().add_name(name));
            } else {
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
    
    void delete_name(const std::string& name) {
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
    
    void add_error(const std::string& msg, int lineno = 0) {
        std::ostringstream oss;
        oss << filename_ << ":" << lineno << ": " << msg;
        errors_.push_back(oss.str());
    }
    
    // === State ===
    std::string filename_;
    int current_lineno_;
    std::vector<std::string> errors_;
};

} // namespace compiler
} // namespace cpython_cpp

#endif // CPYTHON_CPP_COMPILER_BYTECODE_COMPILER_HPP
