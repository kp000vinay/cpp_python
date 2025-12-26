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
    BytecodeCompiler();
    ~BytecodeCompiler() = default;
    
    /**
     * Compile a module AST to a code object
     */
    std::shared_ptr<CodeObject> compile(const ast::Module& module,
                                        const std::string& filename = "<module>");
    
    /**
     * Compile a single statement
     */
    void compile_stmt(ast::Stmt* stmt);
    
    /**
     * Compile an expression
     */
    void compile_expr(ast::Expr* expr);
    
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
    
    void push_scope(ScopeType type, const std::string& name);
    std::shared_ptr<CodeObject> pop_scope();
    
    // === Code Generation ===
    void emit(Opcode op, int arg = -1);
    void emit_with_lineno(Opcode op, int arg, int lineno);
    int emit_jump(Opcode op);  // Returns instruction index for patching
    void patch_jump(int instr_index);
    void patch_jump_to(int instr_index, int target);
    
    // === Statement Compilation ===
    void compile_function_def(ast::FunctionDef* node);
    void compile_async_function_def(ast::AsyncFunctionDef* node);
    void compile_class_def(ast::ClassDef* node);
    void compile_return(ast::Return* node);
    void compile_assign(ast::Assign* node);
    void compile_ann_assign(ast::AnnAssign* node);
    void compile_aug_assign(ast::AugAssign* node);
    void compile_if(ast::If* node);
    void compile_while(ast::While* node);
    void compile_for(ast::For* node);
    void compile_try(ast::Try* node);
    void compile_try_star(ast::TryStar* node);
    void compile_import(ast::Import* node);
    void compile_import_from(ast::ImportFrom* node);
    void compile_global(ast::Global* node);
    void compile_nonlocal(ast::Nonlocal* node);
    void compile_expr_stmt(ast::ExprStmt* node);
    void compile_pass(ast::Pass* node);
    void compile_break(ast::Break* node);
    void compile_continue(ast::Continue* node);
    void compile_raise(ast::Raise* node);
    void compile_assert(ast::Assert* node);
    void compile_delete(ast::Delete* node);
    void compile_match(ast::Match* node);
    
    // === Expression Compilation ===
    void compile_constant(ast::Constant* node);
    void compile_name(ast::Name* node);
    void compile_binop(ast::BinOp* node);
    void compile_unaryop(ast::UnaryOp* node);
    void compile_boolop(ast::BoolOpExpr* node);
    void compile_compare(ast::Compare* node);
    void compile_call(ast::Call* node);
    void compile_attribute(ast::Attribute* node);
    void compile_subscript(ast::Subscript* node);
    void compile_slice(ast::Slice* node);
    void compile_list(ast::List* node);
    void compile_tuple(ast::Tuple* node);
    void compile_dict(ast::Dict* node);
    void compile_set(ast::Set* node);
    void compile_ifexp(ast::IfExp* node);
    void compile_lambda(ast::Lambda* node);
    void compile_listcomp(ast::ListComp* node);
    void compile_await(ast::Await* node);
    void compile_yield(ast::Yield* node);
    void compile_namedexpr(ast::NamedExpr* node);
    void compile_starred(ast::Starred* node);
    void compile_joinedstr(ast::JoinedStr* node);
    void compile_formattedvalue(ast::FormattedValue* node);
    
    // === Target Compilation (for assignments) ===
    void compile_store_target(ast::Expr* target);
    void compile_delete_target(ast::Expr* target);
    
    // === Helper Methods ===
    BinaryOpCode get_binop_code(ast::Operator op);
    BinaryOpCode get_inplace_binop_code(ast::Operator op);
    CompareOpCode get_compare_code(ast::CompareOp op);
    
    int lookup_name(const std::string& name);
    void store_name(const std::string& name);
    void delete_name(const std::string& name);
    
    void add_error(const std::string& msg, int lineno = 0);
    
    // === State ===
    std::string filename_;
    int current_lineno_;
    std::vector<std::string> errors_;
};

} // namespace compiler
} // namespace cpython_cpp

#endif // CPYTHON_CPP_COMPILER_BYTECODE_COMPILER_HPP
