#ifndef CPYTHON_CPP_COMPILER_HPP
#define CPYTHON_CPP_COMPILER_HPP

#include "../ast/module.hpp"
#include "../ast/stmt.hpp"
#include "../ast/expr.hpp"
#include <string>
#include <vector>
#include <memory>
#include <sstream>
#include <iostream>

namespace cpython_cpp {
namespace compiler {

/**
 * Compiler - converts AST to bytecode (or shows structure)
 * Reference: Python/compile.c, Python/assemble.c
 *
 * This is a simplified compiler that shows the breakdown of Python code.
 * A full compiler would generate actual bytecode.
 */
class Compiler {
public:
    Compiler();

    // Compile an AST module and return a breakdown
    std::string compile(std::shared_ptr<ast::Module> module);

    // Get breakdown statistics
    struct Breakdown {
        int function_count;
        int statement_count;
        int expression_count;
        std::vector<std::string> function_names;
    };

    Breakdown get_breakdown(std::shared_ptr<ast::Module> module);

private:
    void visit_module(std::shared_ptr<ast::Module> module, int indent, std::ostringstream& oss);
    void visit_stmt(std::shared_ptr<ast::Stmt> stmt, int indent, std::ostringstream& oss);
    void visit_expr(std::shared_ptr<ast::Expr> expr, int indent, std::ostringstream& oss);

    Breakdown breakdown_;
    void collect_stats(std::shared_ptr<ast::Module> module);
    void collect_stmt_stats(std::shared_ptr<ast::Stmt> stmt);
};

// Implementations
inline Compiler::Compiler() {
    breakdown_ = {0, 0, 0, {}};
}

inline std::string Compiler::compile(std::shared_ptr<ast::Module> module) {
    std::ostringstream oss;
    oss << "=== Python Code Breakdown ===\n\n";

    collect_stats(module);

    oss << "Statistics:\n";
    oss << "  Functions: " << breakdown_.function_count << "\n";
    oss << "  Statements: " << breakdown_.statement_count << "\n";
    oss << "  Expressions: " << breakdown_.expression_count << "\n";
    oss << "  Function names: ";
    for (size_t i = 0; i < breakdown_.function_names.size(); ++i) {
        oss << breakdown_.function_names[i];
        if (i < breakdown_.function_names.size() - 1) oss << ", ";
    }
    oss << "\n\n";

    oss << "AST Structure:\n";
    visit_module(module, 0, oss);

    return oss.str();
}

inline Compiler::Breakdown Compiler::get_breakdown(std::shared_ptr<ast::Module> module) {
    collect_stats(module);
    return breakdown_;
}

inline void Compiler::collect_stats(std::shared_ptr<ast::Module> module) {
    breakdown_ = {0, 0, 0, {}};

    for (const auto& stmt : module->body()) {
        collect_stmt_stats(stmt);
    }
}

inline void Compiler::collect_stmt_stats(std::shared_ptr<ast::Stmt> stmt) {
    breakdown_.statement_count++;

    // Check if it's a FunctionDef
    auto func_def = std::dynamic_pointer_cast<ast::FunctionDef>(stmt);
    if (func_def) {
        breakdown_.function_count++;
        breakdown_.function_names.push_back(func_def->name());

        // Count statements in function body
        for (const auto& body_stmt : func_def->body()) {
            collect_stmt_stats(body_stmt);
        }
    }

    // Check if it's an If statement
    auto if_stmt = std::dynamic_pointer_cast<ast::If>(stmt);
    if (if_stmt) {
        for (const auto& body_stmt : if_stmt->body()) {
            collect_stmt_stats(body_stmt);
        }
        for (const auto& else_stmt : if_stmt->orelse()) {
            collect_stmt_stats(else_stmt);
        }
    }
}

inline void Compiler::visit_module(std::shared_ptr<ast::Module> module, int indent, std::ostringstream& oss) {
    oss << std::string(indent * 2, ' ') << "Module\n";
    for (const auto& stmt : module->body()) {
        visit_stmt(stmt, indent + 1, oss);
    }
}

inline void Compiler::visit_stmt(std::shared_ptr<ast::Stmt> stmt, int indent, std::ostringstream& oss) {
    auto func_def = std::dynamic_pointer_cast<ast::FunctionDef>(stmt);
    if (func_def) {
        oss << std::string(indent * 2, ' ') << "FunctionDef: " << func_def->name() << "\n";
        oss << std::string(indent * 2, ' ') << "  Args: ";
        for (size_t i = 0; i < func_def->args().size(); ++i) {
            oss << func_def->args()[i].arg_name;
            if (i < func_def->args().size() - 1) oss << ", ";
        }
        oss << "\n";
        oss << std::string(indent * 2, ' ') << "  Body:\n";
        for (const auto& body_stmt : func_def->body()) {
            visit_stmt(body_stmt, indent + 2, oss);
        }
        return;
    }

    auto return_stmt = std::dynamic_pointer_cast<ast::Return>(stmt);
    if (return_stmt) {
        oss << std::string(indent * 2, ' ') << "Return\n";
        if (return_stmt->value()) {
            visit_expr(return_stmt->value(), indent + 1, oss);
        }
        return;
    }

    auto assign = std::dynamic_pointer_cast<ast::Assign>(stmt);
    if (assign) {
        oss << std::string(indent * 2, ' ') << "Assign\n";
        oss << std::string(indent * 2, ' ') << "  Targets:\n";
        for (const auto& target : assign->targets()) {
            visit_expr(target, indent + 2, oss);
        }
        oss << std::string(indent * 2, ' ') << "  Value:\n";
        visit_expr(assign->value(), indent + 2, oss);
        return;
    }

    auto if_stmt = std::dynamic_pointer_cast<ast::If>(stmt);
    if (if_stmt) {
        oss << std::string(indent * 2, ' ') << "If\n";
        oss << std::string(indent * 2, ' ') << "  Test:\n";
        visit_expr(if_stmt->test(), indent + 2, oss);
        oss << std::string(indent * 2, ' ') << "  Body:\n";
        for (const auto& body_stmt : if_stmt->body()) {
            visit_stmt(body_stmt, indent + 2, oss);
        }
        if (!if_stmt->orelse().empty()) {
            oss << std::string(indent * 2, ' ') << "  Else:\n";
            for (const auto& else_stmt : if_stmt->orelse()) {
                visit_stmt(else_stmt, indent + 2, oss);
            }
        }
        return;
    }

    auto expr_stmt = std::dynamic_pointer_cast<ast::ExprStmt>(stmt);
    if (expr_stmt) {
        oss << std::string(indent * 2, ' ') << "Expr\n";
        visit_expr(expr_stmt->value(), indent + 1, oss);
        return;
    }

    oss << std::string(indent * 2, ' ') << "Statement\n";
}

inline void Compiler::visit_expr(std::shared_ptr<ast::Expr> expr, int indent, std::ostringstream& oss) {
    auto constant = std::dynamic_pointer_cast<ast::Constant>(expr);
    if (constant) {
        oss << std::string(indent * 2, ' ') << "Constant: " << constant->value() << "\n";
        return;
    }

    auto name = std::dynamic_pointer_cast<ast::Name>(expr);
    if (name) {
        oss << std::string(indent * 2, ' ') << "Name: " << name->id() << "\n";
        return;
    }

    auto binop = std::dynamic_pointer_cast<ast::BinOp>(expr);
    if (binop) {
        oss << std::string(indent * 2, ' ') << "BinOp\n";
        visit_expr(binop->left(), indent + 1, oss);
        oss << std::string(indent * 2, ' ') << "  Op: " << ast::operator_to_string(binop->op()) << "\n";
        visit_expr(binop->right(), indent + 1, oss);
        return;
    }

    auto call = std::dynamic_pointer_cast<ast::Call>(expr);
    if (call) {
        oss << std::string(indent * 2, ' ') << "Call\n";
        visit_expr(call->func(), indent + 1, oss);
        oss << std::string(indent * 2, ' ') << "  Args:\n";
        for (const auto& arg : call->args()) {
            visit_expr(arg, indent + 2, oss);
        }
        return;
    }

    oss << std::string(indent * 2, ' ') << "Expression\n";
}

} // namespace compiler
} // namespace cpython_cpp

#endif // CPYTHON_CPP_COMPILER_HPP




