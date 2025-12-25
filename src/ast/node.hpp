#ifndef CPYTHON_CPP_AST_NODE_HPP
#define CPYTHON_CPP_AST_NODE_HPP

#include <string>
#include <vector>
#include <memory>
#include <cstdint>
#include <type_traits>
#include <concepts>

namespace cpython_cpp {
namespace ast {

/**
 * Template-based AST nodes using CRTP (Curiously Recurring Template Pattern)
 * Reference: Python/Python-ast.c, Include/internal/pycore_ast.h
 * 
 * Benefits of template-based design:
 * - No virtual function overhead (static polymorphism)
 * - Compile-time type checking
 * - Better inlining opportunities
 * - Zero-cost abstractions
 */

// Concept for AST node types
template<typename T>
concept ASTNodeConcept = requires(const T& t, int indent) {
    { t.lineno() } -> std::convertible_to<int>;
    { t.col_offset() } -> std::convertible_to<int>;
    { t.to_string(indent) } -> std::convertible_to<std::string>;
};

// CRTP base class for AST nodes (no virtual functions!)
template<typename Derived>
class ASTNode {
public:
    ASTNode(int lineno, int col_offset)
        : lineno_(lineno), col_offset_(col_offset),
          end_lineno_(-1), end_col_offset_(-1) {}

    // No virtual destructor needed - static polymorphism
    ~ASTNode() = default;

    // Location information
    int lineno() const { return lineno_; }
    int col_offset() const { return col_offset_; }
    int end_lineno() const { return end_lineno_; }
    int end_col_offset() const { return end_col_offset_; }
    
    void set_end_location(int end_lineno, int end_col_offset) {
        end_lineno_ = end_lineno;
        end_col_offset_ = end_col_offset;
    }

    // CRTP: Compile-time polymorphism (no virtual call overhead)
    std::string to_string(int indent = 0) const {
        return static_cast<const Derived*>(this)->to_string_impl(indent);
    }
    
    // Get the derived type at compile time
    const Derived& derived() const {
        return static_cast<const Derived&>(*this);
    }
    
    Derived& derived() {
        return static_cast<Derived&>(*this);
    }

protected:
    int lineno_;
    int col_offset_;
    int end_lineno_;
    int end_col_offset_;

    std::string indent_str(int level) const {
        return std::string(level * 2, ' ');
    }
};

// Type trait to check if a type is an AST node
template<typename T>
struct is_ast_node : std::false_type {};

template<typename Derived>
struct is_ast_node<ASTNode<Derived>> : std::true_type {};

template<typename T>
inline constexpr bool is_ast_node_v = is_ast_node<T>::value;

// Forward declarations removed - use templates directly

// Backward compatibility: Non-template base class for existing code
class ASTNodeBase {
public:
    ASTNodeBase(int lineno, int col_offset)
        : lineno_(lineno), col_offset_(col_offset),
          end_lineno_(-1), end_col_offset_(-1) {}

    virtual ~ASTNodeBase() = default;

    // Location information
    int lineno() const { return lineno_; }
    int col_offset() const { return col_offset_; }
    int end_lineno() const { return end_lineno_; }
    int end_col_offset() const { return end_col_offset_; }
    
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

// Type aliases for backward compatibility
using Expr = ASTNodeBase;
using Stmt = ASTNodeBase;

// Expression context
enum class ExprContext {
    Load,
    Store,
    Del
};

// Template-based operators with compile-time properties
enum class Operator {
    Add, Sub, Mult, MatMult, Div, Mod, Pow, FloorDiv,
    BitOr, BitAnd, BitXor, LShift, RShift
};

// Compile-time operator to string conversion
template<Operator Op>
constexpr const char* operator_name() {
    if constexpr (Op == Operator::Add) return "+";
    else if constexpr (Op == Operator::Sub) return "-";
    else if constexpr (Op == Operator::Mult) return "*";
    else if constexpr (Op == Operator::MatMult) return "@";
    else if constexpr (Op == Operator::Div) return "/";
    else if constexpr (Op == Operator::Mod) return "%";
    else if constexpr (Op == Operator::Pow) return "**";
    else if constexpr (Op == Operator::FloorDiv) return "//";
    else if constexpr (Op == Operator::BitOr) return "|";
    else if constexpr (Op == Operator::BitAnd) return "&";
    else if constexpr (Op == Operator::BitXor) return "^";
    else if constexpr (Op == Operator::LShift) return "<<";
    else if constexpr (Op == Operator::RShift) return ">>";
    else return "<unknown>";
}

// Runtime operator to string
inline const char* operator_to_string(Operator op) {
    switch (op) {
        case Operator::Add: return "+";
        case Operator::Sub: return "-";
        case Operator::Mult: return "*";
        case Operator::MatMult: return "@";
        case Operator::Div: return "/";
        case Operator::Mod: return "%";
        case Operator::Pow: return "**";
        case Operator::FloorDiv: return "//";
        case Operator::BitOr: return "|";
        case Operator::BitAnd: return "&";
        case Operator::BitXor: return "^";
        case Operator::LShift: return "<<";
        case Operator::RShift: return ">>";
        default: return "<unknown>";
    }
}

enum class BoolOp {
    And, Or
};

enum class CompareOp {
    Eq, NotEq, Lt, LtE, Gt, GtE, Is, IsNot, In, NotIn
};

} // namespace ast
} // namespace cpython_cpp

#endif // CPYTHON_CPP_AST_NODE_HPP


