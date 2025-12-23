#ifndef CPYTHON_CPP_AST_EXPR_HPP
#define CPYTHON_CPP_AST_EXPR_HPP

#include "node.hpp"
#include <string>
#include <vector>
#include <memory>
#include <sstream>

namespace cpython_cpp {
namespace ast {

/**
 * Expression AST nodes
 * Reference: Parser/Python.asdl (expr definitions)
 */

// Constant expression
class Constant : public ASTNode {
public:
    Constant(const std::string& value, int lineno, int col_offset)
        : ASTNode(lineno, col_offset), value_(value) {}

    std::string value() const { return value_; }
    std::string to_string(int indent = 0) const override;

private:
    std::string value_;
};

// Ellipsis expression (...)
class Ellipsis : public ASTNode {
public:
    Ellipsis(int lineno, int col_offset)
        : ASTNode(lineno, col_offset) {}

    std::string to_string(int indent = 0) const override;
};

// Name (variable reference)
class Name : public ASTNode {
public:
    Name(const std::string& id, ExprContext ctx, int lineno, int col_offset)
        : ASTNode(lineno, col_offset), id_(id), ctx_(ctx) {}

    std::string id() const { return id_; }
    ExprContext ctx() const { return ctx_; }
    std::string to_string(int indent = 0) const override;

private:
    std::string id_;
    ExprContext ctx_;
};

// Binary operation
class BinOp : public ASTNode {
public:
    BinOp(std::shared_ptr<Expr> left, Operator op, std::shared_ptr<Expr> right,
          int lineno, int col_offset)
        : ASTNode(lineno, col_offset), left_(left), op_(op), right_(right) {}

    std::shared_ptr<Expr> left() const { return left_; }
    Operator op() const { return op_; }
    std::shared_ptr<Expr> right() const { return right_; }
    std::string to_string(int indent = 0) const override;

private:
    std::shared_ptr<Expr> left_;
    Operator op_;
    std::shared_ptr<Expr> right_;
};

// Function call
class Call : public ASTNode {
public:
    Call(std::shared_ptr<Expr> func, std::vector<std::shared_ptr<Expr>> args,
         int lineno, int col_offset)
        : ASTNode(lineno, col_offset), func_(func), args_(args) {}

    std::shared_ptr<Expr> func() const { return func_; }
    const std::vector<std::shared_ptr<Expr>>& args() const { return args_; }
    std::string to_string(int indent = 0) const override;

private:
    std::shared_ptr<Expr> func_;
    std::vector<std::shared_ptr<Expr>> args_;
};

// Boolean operation (and/or)
class BoolOpExpr : public ASTNode {
public:
    BoolOpExpr(ast::BoolOp op, std::vector<std::shared_ptr<Expr>> values,
               int lineno, int col_offset)
        : ASTNode(lineno, col_offset), op_(op), values_(values) {}

    ast::BoolOp op() const { return op_; }
    const std::vector<std::shared_ptr<Expr>>& values() const { return values_; }
    std::string to_string(int indent = 0) const override;

private:
    ast::BoolOp op_;
    std::vector<std::shared_ptr<Expr>> values_;
};

// Comparison operation
class Compare : public ASTNode {
public:
    Compare(std::shared_ptr<Expr> left, CompareOp op, std::shared_ptr<Expr> right,
            int lineno, int col_offset)
        : ASTNode(lineno, col_offset), left_(left), op_(op), right_(right) {}

    std::shared_ptr<Expr> left() const { return left_; }
    CompareOp op() const { return op_; }
    std::shared_ptr<Expr> right() const { return right_; }
    std::string to_string(int indent = 0) const override;

private:
    std::shared_ptr<Expr> left_;
    CompareOp op_;
    std::shared_ptr<Expr> right_;
};

// Unary operation
class UnaryOp : public ASTNode {
public:
    enum class UnaryOpType {
        Not, UAdd, USub, Invert
    };

    UnaryOp(UnaryOpType op, std::shared_ptr<Expr> operand,
            int lineno, int col_offset)
        : ASTNode(lineno, col_offset), op_(op), operand_(operand) {}

    UnaryOpType op() const { return op_; }
    std::shared_ptr<Expr> operand() const { return operand_; }
    std::string to_string(int indent = 0) const override;

private:
    UnaryOpType op_;
    std::shared_ptr<Expr> operand_;
};

// List literal
class List : public ASTNode {
public:
    List(std::vector<std::shared_ptr<Expr>> elts, ExprContext ctx,
         int lineno, int col_offset)
        : ASTNode(lineno, col_offset), elts_(elts), ctx_(ctx) {}

    const std::vector<std::shared_ptr<Expr>>& elts() const { return elts_; }
    ExprContext ctx() const { return ctx_; }
    std::string to_string(int indent = 0) const override;

private:
    std::vector<std::shared_ptr<Expr>> elts_;
    ExprContext ctx_;
};

// Dictionary literal
class Dict : public ASTNode {
public:
    Dict(std::vector<std::shared_ptr<Expr>> keys,
         std::vector<std::shared_ptr<Expr>> values,
         int lineno, int col_offset)
        : ASTNode(lineno, col_offset), keys_(keys), values_(values) {}

    const std::vector<std::shared_ptr<Expr>>& keys() const { return keys_; }
    const std::vector<std::shared_ptr<Expr>>& values() const { return values_; }
    std::string to_string(int indent = 0) const override;

private:
    std::vector<std::shared_ptr<Expr>> keys_;
    std::vector<std::shared_ptr<Expr>> values_;
};

// Tuple literal
class Tuple : public ASTNode {
public:
    Tuple(std::vector<std::shared_ptr<Expr>> elts, ExprContext ctx,
          int lineno, int col_offset)
        : ASTNode(lineno, col_offset), elts_(elts), ctx_(ctx) {}

    const std::vector<std::shared_ptr<Expr>>& elts() const { return elts_; }
    ExprContext ctx() const { return ctx_; }
    std::string to_string(int indent = 0) const override;

private:
    std::vector<std::shared_ptr<Expr>> elts_;
    ExprContext ctx_;
};

// Attribute access (obj.attr)
class Attribute : public ASTNode {
public:
    Attribute(std::shared_ptr<Expr> value, const std::string& attr, ExprContext ctx,
              int lineno, int col_offset)
        : ASTNode(lineno, col_offset), value_(value), attr_(attr), ctx_(ctx) {}

    std::shared_ptr<Expr> value() const { return value_; }
    std::string attr() const { return attr_; }
    ExprContext ctx() const { return ctx_; }
    std::string to_string(int indent = 0) const override;

private:
    std::shared_ptr<Expr> value_;
    std::string attr_;
    ExprContext ctx_;
};

// Slice (start:end:step) - can only appear in Subscript
class Slice : public ASTNode {
public:
    Slice(std::shared_ptr<Expr> lower, std::shared_ptr<Expr> upper, std::shared_ptr<Expr> step,
          int lineno, int col_offset)
        : ASTNode(lineno, col_offset), lower_(lower), upper_(upper), step_(step) {}

    std::shared_ptr<Expr> lower() const { return lower_; }
    std::shared_ptr<Expr> upper() const { return upper_; }
    std::shared_ptr<Expr> step() const { return step_; }
    std::string to_string(int indent = 0) const override;

private:
    std::shared_ptr<Expr> lower_;   // start (can be nullptr)
    std::shared_ptr<Expr> upper_;   // end (can be nullptr)
    std::shared_ptr<Expr> step_;    // step (can be nullptr)
};

// Subscript (obj[key] or obj[start:end:step])
class Subscript : public ASTNode {
public:
    Subscript(std::shared_ptr<Expr> value, std::shared_ptr<Expr> slice, ExprContext ctx,
              int lineno, int col_offset)
        : ASTNode(lineno, col_offset), value_(value), slice_(slice), ctx_(ctx) {}

    std::shared_ptr<Expr> value() const { return value_; }
    std::shared_ptr<Expr> slice() const { return slice_; }
    ExprContext ctx() const { return ctx_; }
    std::string to_string(int indent = 0) const override;

private:
    std::shared_ptr<Expr> value_;
    std::shared_ptr<Expr> slice_;  // Can be an expression (index) or Slice node
    ExprContext ctx_;
};

// Expr is already defined in node.hpp as ASTNode

// Helper functions
inline std::string operator_to_string(Operator op) {
    switch (op) {
        case Operator::Add: return "Add";
        case Operator::Sub: return "Sub";
        case Operator::Mult: return "Mult";
        case Operator::Div: return "Div";
        case Operator::Mod: return "Mod";
        case Operator::Pow: return "Pow";
        case Operator::BitOr: return "BitOr";
        case Operator::BitAnd: return "BitAnd";
        case Operator::BitXor: return "BitXor";
        case Operator::LShift: return "LShift";
        case Operator::RShift: return "RShift";
        default: return "Unknown";
    }
}

inline std::string boolop_to_string(BoolOp op) {
    switch (op) {
        case BoolOp::And: return "And";
        case BoolOp::Or: return "Or";
        default: return "Unknown";
    }
}

inline std::string compareop_to_string(CompareOp op) {
    switch (op) {
        case CompareOp::Eq: return "Eq";
        case CompareOp::NotEq: return "NotEq";
        case CompareOp::Lt: return "Lt";
        case CompareOp::LtE: return "LtE";
        case CompareOp::Gt: return "Gt";
        case CompareOp::GtE: return "GtE";
        case CompareOp::Is: return "Is";
        case CompareOp::IsNot: return "IsNot";
        case CompareOp::In: return "In";
        case CompareOp::NotIn: return "NotIn";
        default: return "Unknown";
    }
}

inline std::string unaryop_to_string(UnaryOp::UnaryOpType op) {
    switch (op) {
        case UnaryOp::UnaryOpType::Not: return "Not";
        case UnaryOp::UnaryOpType::UAdd: return "UAdd";
        case UnaryOp::UnaryOpType::USub: return "USub";
        case UnaryOp::UnaryOpType::Invert: return "Invert";
        default: return "Unknown";
    }
}

inline std::string context_to_string(ExprContext ctx) {
    switch (ctx) {
        case ExprContext::Load: return "Load";
        case ExprContext::Store: return "Store";
        case ExprContext::Del: return "Del";
        default: return "Unknown";
    }
}

// Implementations
inline std::string Constant::to_string(int indent) const {
    return indent_str(indent) + "Constant(value='" + value_ + "')";
}

inline std::string Ellipsis::to_string(int indent) const {
    return indent_str(indent) + "Ellipsis()";
}

inline std::string Name::to_string(int indent) const {
    return indent_str(indent) + "Name(id='" + id_ + "', ctx=" + context_to_string(ctx_) + ")";
}

inline std::string BinOp::to_string(int indent) const {
    std::ostringstream oss;
    oss << indent_str(indent) << "BinOp(\n";
    oss << left_->to_string(indent + 1) << ",\n";
    oss << indent_str(indent + 1) << "op=" << operator_to_string(op_) << ",\n";
    oss << right_->to_string(indent + 1) << "\n";
    oss << indent_str(indent) << ")";
    return oss.str();
}

inline std::string Call::to_string(int indent) const {
    std::ostringstream oss;
    oss << indent_str(indent) << "Call(\n";
    oss << func_->to_string(indent + 1) << ",\n";
    oss << indent_str(indent + 1) << "args=[\n";
    for (size_t i = 0; i < args_.size(); ++i) {
        oss << args_[i]->to_string(indent + 2);
        if (i < args_.size() - 1) oss << ",";
        oss << "\n";
    }
    oss << indent_str(indent + 1) << "]\n";
    oss << indent_str(indent) << ")";
    return oss.str();
}

inline std::string BoolOpExpr::to_string(int indent) const {
    std::ostringstream oss;
    oss << indent_str(indent) << "BoolOp(\n";
    oss << indent_str(indent + 1) << "op=" << boolop_to_string(op_) << ",\n";
    oss << indent_str(indent + 1) << "values=[\n";
    for (size_t i = 0; i < values_.size(); ++i) {
        oss << values_[i]->to_string(indent + 2);
        if (i < values_.size() - 1) oss << ",";
        oss << "\n";
    }
    oss << indent_str(indent + 1) << "]\n";
    oss << indent_str(indent) << ")";
    return oss.str();
}

inline std::string Compare::to_string(int indent) const {
    std::ostringstream oss;
    oss << indent_str(indent) << "Compare(\n";
    oss << left_->to_string(indent + 1) << ",\n";
    oss << indent_str(indent + 1) << "op=" << compareop_to_string(op_) << ",\n";
    oss << right_->to_string(indent + 1) << "\n";
    oss << indent_str(indent) << ")";
    return oss.str();
}

inline std::string UnaryOp::to_string(int indent) const {
    std::ostringstream oss;
    oss << indent_str(indent) << "UnaryOp(\n";
    oss << indent_str(indent + 1) << "op=" << unaryop_to_string(op_) << ",\n";
    oss << operand_->to_string(indent + 1) << "\n";
    oss << indent_str(indent) << ")";
    return oss.str();
}

inline std::string List::to_string(int indent) const {
    std::ostringstream oss;
    oss << indent_str(indent) << "List(\n";
    oss << indent_str(indent + 1) << "elts=[\n";
    for (size_t i = 0; i < elts_.size(); ++i) {
        oss << elts_[i]->to_string(indent + 2);
        if (i < elts_.size() - 1) oss << ",";
        oss << "\n";
    }
    oss << indent_str(indent + 1) << "],\n";
    oss << indent_str(indent + 1) << "ctx=" << context_to_string(ctx_) << "\n";
    oss << indent_str(indent) << ")";
    return oss.str();
}

inline std::string Dict::to_string(int indent) const {
    std::ostringstream oss;
    oss << indent_str(indent) << "Dict(\n";
    oss << indent_str(indent + 1) << "keys=[\n";
    for (size_t i = 0; i < keys_.size(); ++i) {
        if (keys_[i]) {
            oss << keys_[i]->to_string(indent + 2);
        } else {
            oss << indent_str(indent + 2) << "None";
        }
        if (i < keys_.size() - 1) oss << ",";
        oss << "\n";
    }
    oss << indent_str(indent + 1) << "],\n";
    oss << indent_str(indent + 1) << "values=[\n";
    for (size_t i = 0; i < values_.size(); ++i) {
        oss << values_[i]->to_string(indent + 2);
        if (i < values_.size() - 1) oss << ",";
        oss << "\n";
    }
    oss << indent_str(indent + 1) << "]\n";
    oss << indent_str(indent) << ")";
    return oss.str();
}

inline std::string Tuple::to_string(int indent) const {
    std::ostringstream oss;
    oss << indent_str(indent) << "Tuple(\n";
    oss << indent_str(indent + 1) << "elts=[\n";
    for (size_t i = 0; i < elts_.size(); ++i) {
        oss << elts_[i]->to_string(indent + 2);
        if (i < elts_.size() - 1) oss << ",";
        oss << "\n";
    }
    oss << indent_str(indent + 1) << "],\n";
    oss << indent_str(indent + 1) << "ctx=" << context_to_string(ctx_) << "\n";
    oss << indent_str(indent) << ")";
    return oss.str();
}

inline std::string Attribute::to_string(int indent) const {
    std::ostringstream oss;
    oss << indent_str(indent) << "Attribute(\n";
    oss << value_->to_string(indent + 1) << ",\n";
    oss << indent_str(indent + 1) << "attr='" << attr_ << "',\n";
    oss << indent_str(indent + 1) << "ctx=" << context_to_string(ctx_) << "\n";
    oss << indent_str(indent) << ")";
    return oss.str();
}

inline std::string Slice::to_string(int indent) const {
    std::ostringstream oss;
    oss << indent_str(indent) << "Slice(\n";
    if (lower_) {
        oss << indent_str(indent + 1) << "lower=\n";
        oss << lower_->to_string(indent + 2) << ",\n";
    } else {
        oss << indent_str(indent + 1) << "lower=None,\n";
    }
    if (upper_) {
        oss << indent_str(indent + 1) << "upper=\n";
        oss << upper_->to_string(indent + 2) << ",\n";
    } else {
        oss << indent_str(indent + 1) << "upper=None,\n";
    }
    if (step_) {
        oss << indent_str(indent + 1) << "step=\n";
        oss << step_->to_string(indent + 2) << "\n";
    } else {
        oss << indent_str(indent + 1) << "step=None\n";
    }
    oss << indent_str(indent) << ")";
    return oss.str();
}

inline std::string Subscript::to_string(int indent) const {
    std::ostringstream oss;
    oss << indent_str(indent) << "Subscript(\n";
    oss << value_->to_string(indent + 1) << ",\n";
    oss << slice_->to_string(indent + 1) << ",\n";
    oss << indent_str(indent + 1) << "ctx=" << context_to_string(ctx_) << "\n";
    oss << indent_str(indent) << ")";
    return oss.str();
}

// Lambda function (anonymous function)
class Lambda : public ASTNode {
public:
    Lambda(std::vector<std::string> args,
           std::shared_ptr<Expr> body,
           int lineno, int col_offset)
        : ASTNode(lineno, col_offset), args_(args), body_(body) {}

    const std::vector<std::string>& args() const { return args_; }
    std::shared_ptr<Expr> body() const { return body_; }
    std::string to_string(int indent = 0) const override;

private:
    std::vector<std::string> args_;
    std::shared_ptr<Expr> body_;
};

// Yield expression (generator)
class Yield : public ASTNode {
public:
    Yield(std::shared_ptr<Expr> value, int lineno, int col_offset)
        : ASTNode(lineno, col_offset), value_(value) {}

    std::shared_ptr<Expr> value() const { return value_; }  // nullptr if no value
    std::string to_string(int indent = 0) const override;

private:
    std::shared_ptr<Expr> value_;
};

// Yield from expression (generator delegation)
class YieldFrom : public ASTNode {
public:
    YieldFrom(std::shared_ptr<Expr> value, int lineno, int col_offset)
        : ASTNode(lineno, col_offset), value_(value) {}

    std::shared_ptr<Expr> value() const { return value_; }
    std::string to_string(int indent = 0) const override;

private:
    std::shared_ptr<Expr> value_;
};

// Conditional expression (ternary operator): x if condition else y
class IfExp : public ASTNode {
public:
    IfExp(std::shared_ptr<Expr> test, std::shared_ptr<Expr> body, std::shared_ptr<Expr> orelse,
          int lineno, int col_offset)
        : ASTNode(lineno, col_offset), test_(test), body_(body), orelse_(orelse) {}

    std::shared_ptr<Expr> test() const { return test_; }
    std::shared_ptr<Expr> body() const { return body_; }
    std::shared_ptr<Expr> orelse() const { return orelse_; }
    std::string to_string(int indent = 0) const override;

private:
    std::shared_ptr<Expr> test_;
    std::shared_ptr<Expr> body_;
    std::shared_ptr<Expr> orelse_;
};

// Comprehension helper struct (for comprehensions and generator expressions)
struct Comprehension {
    std::shared_ptr<Expr> target;      // star_targets
    std::shared_ptr<Expr> iter;        // disjunction (the iterable)
    std::vector<std::shared_ptr<Expr>> ifs;  // Optional if conditions

    Comprehension(std::shared_ptr<Expr> t, std::shared_ptr<Expr> i,
                  std::vector<std::shared_ptr<Expr>> conditions)
        : target(t), iter(i), ifs(conditions) {}
};

// List comprehension: [x for x in range(10)]
class ListComp : public ASTNode {
public:
    ListComp(std::shared_ptr<Expr> elt, std::vector<Comprehension> generators,
             int lineno, int col_offset)
        : ASTNode(lineno, col_offset), elt_(elt), generators_(generators) {}

    std::shared_ptr<Expr> elt() const { return elt_; }
    const std::vector<Comprehension>& generators() const { return generators_; }
    std::string to_string(int indent = 0) const override;

private:
    std::shared_ptr<Expr> elt_;
    std::vector<Comprehension> generators_;
};

// Set comprehension: {x for x in range(10)}
class SetComp : public ASTNode {
public:
    SetComp(std::shared_ptr<Expr> elt, std::vector<Comprehension> generators,
            int lineno, int col_offset)
        : ASTNode(lineno, col_offset), elt_(elt), generators_(generators) {}

    std::shared_ptr<Expr> elt() const { return elt_; }
    const std::vector<Comprehension>& generators() const { return generators_; }
    std::string to_string(int indent = 0) const override;

private:
    std::shared_ptr<Expr> elt_;
    std::vector<Comprehension> generators_;
};

// Dictionary comprehension: {k: v for k, v in items}
class DictComp : public ASTNode {
public:
    DictComp(std::shared_ptr<Expr> key, std::shared_ptr<Expr> value,
             std::vector<Comprehension> generators,
             int lineno, int col_offset)
        : ASTNode(lineno, col_offset), key_(key), value_(value), generators_(generators) {}

    std::shared_ptr<Expr> key() const { return key_; }
    std::shared_ptr<Expr> value() const { return value_; }
    const std::vector<Comprehension>& generators() const { return generators_; }
    std::string to_string(int indent = 0) const override;

private:
    std::shared_ptr<Expr> key_;
    std::shared_ptr<Expr> value_;
    std::vector<Comprehension> generators_;
};

// Generator expression: (x for x in range(10))
class GeneratorExp : public ASTNode {
public:
    GeneratorExp(std::shared_ptr<Expr> elt, std::vector<Comprehension> generators,
                int lineno, int col_offset)
        : ASTNode(lineno, col_offset), elt_(elt), generators_(generators) {}

    std::shared_ptr<Expr> elt() const { return elt_; }
    const std::vector<Comprehension>& generators() const { return generators_; }
    std::string to_string(int indent = 0) const override;

private:
    std::shared_ptr<Expr> elt_;
    std::vector<Comprehension> generators_;
};

inline std::string Lambda::to_string(int indent) const {
    std::ostringstream oss;
    oss << indent_str(indent) << "Lambda(\n";
    oss << indent_str(indent + 1) << "args=[";
    for (size_t i = 0; i < args_.size(); ++i) {
        oss << "\"" << args_[i] << "\"";
        if (i < args_.size() - 1) oss << ", ";
    }
    oss << "],\n";
    oss << indent_str(indent + 1) << "body=\n";
    oss << body_->to_string(indent + 2) << "\n";
    oss << indent_str(indent) << ")";
    return oss.str();
}

inline std::string Yield::to_string(int indent) const {
    std::ostringstream oss;
    oss << indent_str(indent) << "Yield(\n";
    if (value_) {
        oss << indent_str(indent + 1) << "value=\n";
        oss << value_->to_string(indent + 2) << "\n";
    } else {
        oss << indent_str(indent + 1) << "value=None\n";
    }
    oss << indent_str(indent) << ")";
    return oss.str();
}

inline std::string YieldFrom::to_string(int indent) const {
    std::ostringstream oss;
    oss << indent_str(indent) << "YieldFrom(\n";
    oss << indent_str(indent + 1) << "value=\n";
    oss << value_->to_string(indent + 2) << "\n";
    oss << indent_str(indent) << ")";
    return oss.str();
}

inline std::string IfExp::to_string(int indent) const {
    std::ostringstream oss;
    oss << indent_str(indent) << "IfExp(\n";
    oss << indent_str(indent + 1) << "test=\n";
    oss << test_->to_string(indent + 2) << ",\n";
    oss << indent_str(indent + 1) << "body=\n";
    oss << body_->to_string(indent + 2) << ",\n";
    oss << indent_str(indent + 1) << "orelse=\n";
    oss << orelse_->to_string(indent + 2) << "\n";
    oss << indent_str(indent) << ")";
    return oss.str();
}

inline std::string ListComp::to_string(int indent) const {
    std::ostringstream oss;
    oss << indent_str(indent) << "ListComp(\n";
    oss << indent_str(indent + 1) << "elt=\n";
    oss << elt_->to_string(indent + 2) << ",\n";
    oss << indent_str(indent + 1) << "generators=[...]\n";
    oss << indent_str(indent) << ")";
    return oss.str();
}

inline std::string SetComp::to_string(int indent) const {
    std::ostringstream oss;
    oss << indent_str(indent) << "SetComp(\n";
    oss << indent_str(indent + 1) << "elt=\n";
    oss << elt_->to_string(indent + 2) << ",\n";
    oss << indent_str(indent + 1) << "generators=[...]\n";
    oss << indent_str(indent) << ")";
    return oss.str();
}

inline std::string DictComp::to_string(int indent) const {
    std::ostringstream oss;
    oss << indent_str(indent) << "DictComp(\n";
    oss << indent_str(indent + 1) << "key=\n";
    oss << key_->to_string(indent + 2) << ",\n";
    oss << indent_str(indent + 1) << "value=\n";
    oss << value_->to_string(indent + 2) << ",\n";
    oss << indent_str(indent + 1) << "generators=[...]\n";
    oss << indent_str(indent) << ")";
    return oss.str();
}

inline std::string GeneratorExp::to_string(int indent) const {
    std::ostringstream oss;
    oss << indent_str(indent) << "GeneratorExp(\n";
    oss << indent_str(indent + 1) << "elt=\n";
    oss << elt_->to_string(indent + 2) << ",\n";
    oss << indent_str(indent + 1) << "generators=[...]\n";
    oss << indent_str(indent) << ")";
    return oss.str();
}

} // namespace ast
} // namespace cpython_cpp

#endif // CPYTHON_CPP_AST_EXPR_HPP


