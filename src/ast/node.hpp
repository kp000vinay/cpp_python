#ifndef CPYTHON_CPP_AST_NODE_HPP
#define CPYTHON_CPP_AST_NODE_HPP

#include <string>
#include <vector>
#include <memory>
#include <cstdint>

namespace cpython_cpp {
namespace ast {

/**
 * Base class for all AST nodes
 * Reference: Python/Python-ast.c, Include/internal/pycore_ast.h
 */
class ASTNode {
public:
    ASTNode(int lineno, int col_offset)
        : lineno_(lineno), col_offset_(col_offset),
          end_lineno_(-1), end_col_offset_(-1) {}

    virtual ~ASTNode() = default;

    // Location information
    int lineno() const { return lineno_; }
    int col_offset() const { return col_offset_; }
    void set_end_location(int end_lineno, int end_col_offset) {
        end_lineno_ = end_lineno;
        end_col_offset_ = end_col_offset;
    }

    // Virtual method for pretty printing
    virtual std::string to_string(int indent = 0) const = 0;

protected:
    int lineno_;
    int col_offset_;
    int end_lineno_;
    int end_col_offset_;

    std::string indent_str(int level) const {
        return std::string(level * 2, ' ');
    }
};

// Forward declarations
class ASTNode;
using Expr = ASTNode;
using Stmt = ASTNode;

// Expression context
enum class ExprContext {
    Load,
    Store,
    Del
};

// Operators
enum class Operator {
    Add, Sub, Mult, Div, Mod, Pow,
    BitOr, BitAnd, BitXor, LShift, RShift
};

enum class BoolOp {
    And, Or
};

enum class CompareOp {
    Eq, NotEq, Lt, LtE, Gt, GtE, Is, IsNot, In, NotIn
};

} // namespace ast
} // namespace cpython_cpp

#endif // CPYTHON_CPP_AST_NODE_HPP


