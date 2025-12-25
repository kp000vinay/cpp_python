#ifndef CPYTHON_CPP_AST_STMT_HPP
#define CPYTHON_CPP_AST_STMT_HPP

#include "node.hpp"
#include "expr.hpp"
#include <string>
#include <vector>
#include <memory>
#include <sstream>

namespace cpython_cpp {
namespace ast {

/**
 * Statement AST nodes
 * Reference: Parser/Python.asdl (stmt definitions)
 */

// Forward declaration for TypeParam (defined later in this file)
class TypeParam;

// Function argument with optional annotation (Python 3.5+)
struct arg {
    std::string arg_name;                      // Parameter name
    std::shared_ptr<Expr> annotation;          // Type annotation (optional, can be nullptr)
    
    arg(const std::string& name, std::shared_ptr<Expr> ann = nullptr)
        : arg_name(name), annotation(ann) {}
};

// Function definition (with PEP 695 generic type support)
class FunctionDef : public ASTNodeBase {
public:
    FunctionDef(const std::string& name,
                std::vector<arg> args,
                std::vector<std::shared_ptr<Stmt>> body,
                std::vector<std::shared_ptr<Expr>> decorator_list,
                std::shared_ptr<Expr> returns,  // Return type annotation (optional)
                std::vector<std::shared_ptr<TypeParam>> type_params,  // PEP 695: Generic type parameters
                int lineno, int col_offset)
        : ASTNodeBase(lineno, col_offset), name_(name), args_(args), body_(body),
          decorator_list_(decorator_list), returns_(returns), type_params_(type_params) {}

    std::string name() const { return name_; }
    const std::vector<arg>& args() const { return args_; }
    const std::vector<std::shared_ptr<Stmt>>& body() const { return body_; }
    const std::vector<std::shared_ptr<Expr>>& decorator_list() const { return decorator_list_; }
    std::shared_ptr<Expr> returns() const { return returns_; }
    const std::vector<std::shared_ptr<TypeParam>>& type_params() const { return type_params_; }  // PEP 695
    std::string to_string(int indent = 0) const override;

private:
    std::string name_;
    std::vector<arg> args_;
    std::vector<std::shared_ptr<Stmt>> body_;
    std::vector<std::shared_ptr<Expr>> decorator_list_;
    std::shared_ptr<Expr> returns_;  // Return type annotation
    std::vector<std::shared_ptr<TypeParam>> type_params_;  // PEP 695: Generic type parameters
};

// Async function definition (Python 3.5+)
class AsyncFunctionDef : public ASTNodeBase {
public:
    AsyncFunctionDef(const std::string& name,
                     std::vector<arg> args,
                     std::vector<std::shared_ptr<Stmt>> body,
                     std::vector<std::shared_ptr<Expr>> decorator_list,
                     std::shared_ptr<Expr> returns,  // Return type annotation (optional)
                     int lineno, int col_offset)
        : ASTNodeBase(lineno, col_offset), name_(name), args_(args), body_(body),
          decorator_list_(decorator_list), returns_(returns) {}

    std::string name() const { return name_; }
    const std::vector<arg>& args() const { return args_; }
    const std::vector<std::shared_ptr<Stmt>>& body() const { return body_; }
    const std::vector<std::shared_ptr<Expr>>& decorator_list() const { return decorator_list_; }
    std::shared_ptr<Expr> returns() const { return returns_; }
    std::string to_string(int indent = 0) const override;

private:
    std::string name_;
    std::vector<arg> args_;
    std::vector<std::shared_ptr<Stmt>> body_;
    std::vector<std::shared_ptr<Expr>> decorator_list_;
    std::shared_ptr<Expr> returns_;  // Return type annotation
};

// Return statement
class Return : public ASTNodeBase {
public:
    Return(std::shared_ptr<Expr> value, int lineno, int col_offset)
        : ASTNodeBase(lineno, col_offset), value_(value) {}

    std::shared_ptr<Expr> value() const { return value_; }
    std::string to_string(int indent = 0) const override;

private:
    std::shared_ptr<Expr> value_;
};

// Assignment
class Assign : public ASTNodeBase {
public:
    Assign(std::vector<std::shared_ptr<Expr>> targets,
           std::shared_ptr<Expr> value,
           int lineno, int col_offset)
        : ASTNodeBase(lineno, col_offset), targets_(targets), value_(value) {}

    const std::vector<std::shared_ptr<Expr>>& targets() const { return targets_; }
    std::shared_ptr<Expr> value() const { return value_; }
    std::string to_string(int indent = 0) const override;

private:
    std::vector<std::shared_ptr<Expr>> targets_;
    std::shared_ptr<Expr> value_;
};

// Annotated assignment: x: int = 5 or y: str (Python 3.6+)
class AnnAssign : public ASTNodeBase {
public:
    AnnAssign(std::shared_ptr<Expr> target,
              std::shared_ptr<Expr> annotation,
              std::shared_ptr<Expr> value,  // optional, can be nullptr
              bool simple,
              int lineno, int col_offset)
        : ASTNodeBase(lineno, col_offset),
          target_(target),
          annotation_(annotation),
          value_(value),
          simple_(simple) {}

    std::shared_ptr<Expr> target() const { return target_; }
    std::shared_ptr<Expr> annotation() const { return annotation_; }
    std::shared_ptr<Expr> value() const { return value_; }
    bool simple() const { return simple_; }
    std::string to_string(int indent = 0) const override;

private:
    std::shared_ptr<Expr> target_;
    std::shared_ptr<Expr> annotation_;
    std::shared_ptr<Expr> value_;  // nullptr if no value (e.g., "y: str")
    bool simple_;  // True if target is a simple name
};

// Augmented assignment (+=, -=, etc.)
class AugAssign : public ASTNodeBase {
public:
    AugAssign(std::shared_ptr<Expr> target,
              Operator op,
              std::shared_ptr<Expr> value,
              int lineno, int col_offset)
        : ASTNodeBase(lineno, col_offset), target_(target), op_(op), value_(value) {}

    std::shared_ptr<Expr> target() const { return target_; }
    Operator op() const { return op_; }
    std::shared_ptr<Expr> value() const { return value_; }
    std::string to_string(int indent = 0) const override;

private:
    std::shared_ptr<Expr> target_;
    Operator op_;
    std::shared_ptr<Expr> value_;
};

// Expression statement
class ExprStmt : public ASTNodeBase {
public:
    ExprStmt(std::shared_ptr<Expr> value, int lineno, int col_offset)
        : ASTNodeBase(lineno, col_offset), value_(value) {}

    std::shared_ptr<Expr> value() const { return value_; }
    std::string to_string(int indent = 0) const override;

private:
    std::shared_ptr<Expr> value_;
};

// If statement
class If : public ASTNodeBase {
public:
    If(std::shared_ptr<Expr> test,
       std::vector<std::shared_ptr<Stmt>> body,
       std::vector<std::shared_ptr<Stmt>> orelse,
       int lineno, int col_offset)
        : ASTNodeBase(lineno, col_offset), test_(test), body_(body), orelse_(orelse) {}

    std::shared_ptr<Expr> test() const { return test_; }
    const std::vector<std::shared_ptr<Stmt>>& body() const { return body_; }
    const std::vector<std::shared_ptr<Stmt>>& orelse() const { return orelse_; }
    std::string to_string(int indent = 0) const override;

private:
    std::shared_ptr<Expr> test_;
    std::vector<std::shared_ptr<Stmt>> body_;
    std::vector<std::shared_ptr<Stmt>> orelse_;
};

// While loop
class While : public ASTNodeBase {
public:
    While(std::shared_ptr<Expr> test,
          std::vector<std::shared_ptr<Stmt>> body,
          std::vector<std::shared_ptr<Stmt>> orelse,
          int lineno, int col_offset)
        : ASTNodeBase(lineno, col_offset), test_(test), body_(body), orelse_(orelse) {}

    std::shared_ptr<Expr> test() const { return test_; }
    const std::vector<std::shared_ptr<Stmt>>& body() const { return body_; }
    const std::vector<std::shared_ptr<Stmt>>& orelse() const { return orelse_; }
    std::string to_string(int indent = 0) const override;

private:
    std::shared_ptr<Expr> test_;
    std::vector<std::shared_ptr<Stmt>> body_;
    std::vector<std::shared_ptr<Stmt>> orelse_;
};

// Break statement
class Break : public ASTNodeBase {
public:
    Break(int lineno, int col_offset)
        : ASTNodeBase(lineno, col_offset) {}

    std::string to_string(int indent = 0) const override;
};

// Continue statement
class Continue : public ASTNodeBase {
public:
    Continue(int lineno, int col_offset)
        : ASTNodeBase(lineno, col_offset) {}

    std::string to_string(int indent = 0) const override;
};

// For loop
class For : public ASTNodeBase {
public:
    For(std::shared_ptr<Expr> target,
        std::shared_ptr<Expr> iter,
        std::vector<std::shared_ptr<Stmt>> body,
        std::vector<std::shared_ptr<Stmt>> orelse,
        int lineno, int col_offset)
        : ASTNodeBase(lineno, col_offset), target_(target), iter_(iter), body_(body), orelse_(orelse) {}

    std::shared_ptr<Expr> target() const { return target_; }
    std::shared_ptr<Expr> iter() const { return iter_; }
    const std::vector<std::shared_ptr<Stmt>>& body() const { return body_; }
    const std::vector<std::shared_ptr<Stmt>>& orelse() const { return orelse_; }
    std::string to_string(int indent = 0) const override;

private:
    std::shared_ptr<Expr> target_;
    std::shared_ptr<Expr> iter_;
    std::vector<std::shared_ptr<Stmt>> body_;
    std::vector<std::shared_ptr<Stmt>> orelse_;
};

// Async for loop (Python 3.5+)
class AsyncFor : public ASTNodeBase {
public:
    AsyncFor(std::shared_ptr<Expr> target,
             std::shared_ptr<Expr> iter,
             std::vector<std::shared_ptr<Stmt>> body,
             std::vector<std::shared_ptr<Stmt>> orelse,
             int lineno, int col_offset)
        : ASTNodeBase(lineno, col_offset), target_(target), iter_(iter), body_(body), orelse_(orelse) {}

    std::shared_ptr<Expr> target() const { return target_; }
    std::shared_ptr<Expr> iter() const { return iter_; }
    const std::vector<std::shared_ptr<Stmt>>& body() const { return body_; }
    const std::vector<std::shared_ptr<Stmt>>& orelse() const { return orelse_; }
    std::string to_string(int indent = 0) const override;

private:
    std::shared_ptr<Expr> target_;
    std::shared_ptr<Expr> iter_;
    std::vector<std::shared_ptr<Stmt>> body_;
    std::vector<std::shared_ptr<Stmt>> orelse_;
};

// Stmt is already defined in node.hpp as ASTNode

// Implementations
inline std::string FunctionDef::to_string(int indent) const {
    std::ostringstream oss;
    oss << indent_str(indent) << "FunctionDef(\n";
    oss << indent_str(indent + 1) << "name='" << name_ << "',\n";
    // PEP 695: Print type_params count (detailed output is at end of file)
    if (!type_params_.empty()) {
        oss << indent_str(indent + 1) << "type_params=[" << type_params_.size() << " params],\n";
    }
    if (!decorator_list_.empty()) {
        oss << indent_str(indent + 1) << "decorator_list=[\n";
        for (const auto& deco : decorator_list_) {
            oss << deco->to_string(indent + 2) << ",\n";
        }
        oss << indent_str(indent + 1) << "],\n";
    }
    oss << indent_str(indent + 1) << "args=[\n";
    for (size_t i = 0; i < args_.size(); ++i) {
        oss << indent_str(indent + 2) << "arg(name='" << args_[i].arg_name << "'";
        if (args_[i].annotation) {
            oss << ", annotation=" << args_[i].annotation->to_string(0);
        }
        oss << ")";
        if (i < args_.size() - 1) oss << ",";
        oss << "\n";
    }
    oss << indent_str(indent + 1) << "],\n";
    oss << indent_str(indent + 1) << "body=[\n";
    for (const auto& stmt : body_) {
        oss << stmt->to_string(indent + 2) << ",\n";
    }
    oss << indent_str(indent + 1) << "],\n";
    if (returns_) {
        oss << indent_str(indent + 1) << "returns=" << returns_->to_string(0) << "\n";
    }
    oss << indent_str(indent) << ")";
    return oss.str();
}

inline std::string AsyncFunctionDef::to_string(int indent) const {
    std::ostringstream oss;
    oss << indent_str(indent) << "AsyncFunctionDef(\n";
    oss << indent_str(indent + 1) << "name='" << name_ << "',\n";
    if (!decorator_list_.empty()) {
        oss << indent_str(indent + 1) << "decorator_list=[\n";
        for (const auto& deco : decorator_list_) {
            oss << deco->to_string(indent + 2) << ",\n";
        }
        oss << indent_str(indent + 1) << "],\n";
    }
    oss << indent_str(indent + 1) << "args=[\n";
    for (size_t i = 0; i < args_.size(); ++i) {
        oss << indent_str(indent + 2) << "arg(name='" << args_[i].arg_name << "'";
        if (args_[i].annotation) {
            oss << ", annotation=" << args_[i].annotation->to_string(0);
        }
        oss << ")";
        if (i < args_.size() - 1) oss << ",";
        oss << "\n";
    }
    oss << indent_str(indent + 1) << "],\n";
    oss << indent_str(indent + 1) << "body=[\n";
    for (const auto& stmt : body_) {
        oss << stmt->to_string(indent + 2) << ",\n";
    }
    oss << indent_str(indent + 1) << "],\n";
    if (returns_) {
        oss << indent_str(indent + 1) << "returns=" << returns_->to_string(0) << "\n";
    }
    oss << indent_str(indent) << ")";
    return oss.str();
}

inline std::string Return::to_string(int indent) const {
    std::ostringstream oss;
    oss << indent_str(indent) << "Return(\n";
    if (value_) {
        oss << value_->to_string(indent + 1) << "\n";
    } else {
        oss << indent_str(indent + 1) << "None\n";
    }
    oss << indent_str(indent) << ")";
    return oss.str();
}

inline std::string Assign::to_string(int indent) const {
    std::ostringstream oss;
    oss << indent_str(indent) << "Assign(\n";
    oss << indent_str(indent + 1) << "targets=[\n";
    for (const auto& target : targets_) {
        oss << target->to_string(indent + 2) << ",\n";
    }
    oss << indent_str(indent + 1) << "],\n";
    oss << value_->to_string(indent + 1) << "\n";
    oss << indent_str(indent) << ")";
    return oss.str();
}

inline std::string AnnAssign::to_string(int indent) const {
    std::ostringstream oss;
    oss << indent_str(indent) << "AnnAssign(\n";
    oss << indent_str(indent + 1) << "target=\n";
    oss << target_->to_string(indent + 2) << ",\n";
    oss << indent_str(indent + 1) << "annotation=\n";
    oss << annotation_->to_string(indent + 2) << ",\n";
    if (value_) {
        oss << indent_str(indent + 1) << "value=\n";
        oss << value_->to_string(indent + 2) << ",\n";
    } else {
        oss << indent_str(indent + 1) << "value=None,\n";
    }
    oss << indent_str(indent + 1) << "simple=" << (simple_ ? "True" : "False") << "\n";
    oss << indent_str(indent) << ")";
    return oss.str();
}

inline std::string AugAssign::to_string(int indent) const {
    std::ostringstream oss;
    oss << indent_str(indent) << "AugAssign(\n";
    oss << indent_str(indent + 1) << "target=\n";
    oss << target_->to_string(indent + 2) << ",\n";
    oss << indent_str(indent + 1) << "op=" << operator_to_string(op_) << ",\n";
    oss << indent_str(indent + 1) << "value=\n";
    oss << value_->to_string(indent + 2) << "\n";
    oss << indent_str(indent) << ")";
    return oss.str();
}

inline std::string ExprStmt::to_string(int indent) const {
    std::ostringstream oss;
    oss << indent_str(indent) << "Expr(\n";
    oss << value_->to_string(indent + 1) << "\n";
    oss << indent_str(indent) << ")";
    return oss.str();
}

inline std::string If::to_string(int indent) const {
    std::ostringstream oss;
    oss << indent_str(indent) << "If(\n";
    oss << indent_str(indent + 1) << "test=\n";
    oss << test_->to_string(indent + 2) << ",\n";
    oss << indent_str(indent + 1) << "body=[\n";
    for (const auto& stmt : body_) {
        oss << stmt->to_string(indent + 2) << ",\n";
    }
    oss << indent_str(indent + 1) << "],\n";
    if (!orelse_.empty()) {
        oss << indent_str(indent + 1) << "orelse=[\n";
        for (const auto& stmt : orelse_) {
            oss << stmt->to_string(indent + 2) << ",\n";
        }
        oss << indent_str(indent + 1) << "]\n";
    }
    oss << indent_str(indent) << ")";
    return oss.str();
}

inline std::string While::to_string(int indent) const {
    std::ostringstream oss;
    oss << indent_str(indent) << "While(\n";
    oss << indent_str(indent + 1) << "test=\n";
    oss << test_->to_string(indent + 2) << ",\n";
    oss << indent_str(indent + 1) << "body=[\n";
    for (const auto& stmt : body_) {
        oss << stmt->to_string(indent + 2) << ",\n";
    }
    oss << indent_str(indent + 1) << "],\n";
    if (!orelse_.empty()) {
        oss << indent_str(indent + 1) << "orelse=[\n";
        for (const auto& stmt : orelse_) {
            oss << stmt->to_string(indent + 2) << ",\n";
        }
        oss << indent_str(indent + 1) << "]\n";
    }
    oss << indent_str(indent) << ")";
    return oss.str();
}

inline std::string For::to_string(int indent) const {
    std::ostringstream oss;
    oss << indent_str(indent) << "For(\n";
    oss << indent_str(indent + 1) << "target=\n";
    oss << target_->to_string(indent + 2) << ",\n";
    oss << indent_str(indent + 1) << "iter=\n";
    oss << iter_->to_string(indent + 2) << ",\n";
    oss << indent_str(indent + 1) << "body=[\n";
    for (const auto& stmt : body_) {
        oss << stmt->to_string(indent + 2) << ",\n";
    }
    oss << indent_str(indent + 1) << "],\n";
    if (!orelse_.empty()) {
        oss << indent_str(indent + 1) << "orelse=[\n";
        for (const auto& stmt : orelse_) {
            oss << stmt->to_string(indent + 2) << ",\n";
        }
        oss << indent_str(indent + 1) << "]\n";
    }
    oss << indent_str(indent) << ")";
    return oss.str();
}

inline std::string AsyncFor::to_string(int indent) const {
    std::ostringstream oss;
    oss << indent_str(indent) << "AsyncFor(\n";
    oss << indent_str(indent + 1) << "target=\n";
    oss << target_->to_string(indent + 2) << ",\n";
    oss << indent_str(indent + 1) << "iter=\n";
    oss << iter_->to_string(indent + 2) << ",\n";
    oss << indent_str(indent + 1) << "body=[\n";
    for (const auto& stmt : body_) {
        oss << stmt->to_string(indent + 2) << ",\n";
    }
    oss << indent_str(indent + 1) << "],\n";
    if (!orelse_.empty()) {
        oss << indent_str(indent + 1) << "orelse=[\n";
        for (const auto& stmt : orelse_) {
            oss << stmt->to_string(indent + 2) << ",\n";
        }
        oss << indent_str(indent + 1) << "]\n";
    }
    oss << indent_str(indent) << ")";
    return oss.str();
}

inline std::string Break::to_string(int indent) const {
    return indent_str(indent) + "Break()";
}

inline std::string Continue::to_string(int indent) const {
    return indent_str(indent) + "Continue()";
}

// Pass statement (no-op)
class Pass : public ASTNodeBase {
public:
    Pass(int lineno, int col_offset)
        : ASTNodeBase(lineno, col_offset) {}

    std::string to_string(int indent = 0) const override;
};

// Raise statement
class Raise : public ASTNodeBase {
public:
    Raise(std::shared_ptr<Expr> exc,
          std::shared_ptr<Expr> cause,
          int lineno, int col_offset)
        : ASTNodeBase(lineno, col_offset), exc_(exc), cause_(cause) {}

    std::shared_ptr<Expr> exc() const { return exc_; }
    std::shared_ptr<Expr> cause() const { return cause_; }
    std::string to_string(int indent = 0) const override;

private:
    std::shared_ptr<Expr> exc_;  // Exception to raise (null if bare raise)
    std::shared_ptr<Expr> cause_;  // Exception cause (null if no 'from')
};

// Delete statement
class Delete : public ASTNodeBase {
public:
    Delete(std::vector<std::shared_ptr<Expr>> targets, int lineno, int col_offset)
        : ASTNodeBase(lineno, col_offset), targets_(targets) {}

    const std::vector<std::shared_ptr<Expr>>& targets() const { return targets_; }
    std::string to_string(int indent = 0) const override;

private:
    std::vector<std::shared_ptr<Expr>> targets_;
};

// Assert statement
class Assert : public ASTNodeBase {
public:
    Assert(std::shared_ptr<Expr> test,
           std::shared_ptr<Expr> msg,
           int lineno, int col_offset)
        : ASTNodeBase(lineno, col_offset), test_(test), msg_(msg) {}

    std::shared_ptr<Expr> test() const { return test_; }
    std::shared_ptr<Expr> msg() const { return msg_; }
    std::string to_string(int indent = 0) const override;

private:
    std::shared_ptr<Expr> test_;
    std::shared_ptr<Expr> msg_;  // Optional message
};

// Global statement
class Global : public ASTNodeBase {
public:
    Global(std::vector<std::string> names, int lineno, int col_offset)
        : ASTNodeBase(lineno, col_offset), names_(names) {}

    const std::vector<std::string>& names() const { return names_; }
    std::string to_string(int indent = 0) const override;

private:
    std::vector<std::string> names_;
};

// Nonlocal statement
class Nonlocal : public ASTNodeBase {
public:
    Nonlocal(std::vector<std::string> names, int lineno, int col_offset)
        : ASTNodeBase(lineno, col_offset), names_(names) {}

    const std::vector<std::string>& names() const { return names_; }
    std::string to_string(int indent = 0) const override;

private:
    std::vector<std::string> names_;
};

// Exception handler (for try/except)
class ExceptHandler : public ASTNodeBase {
public:
    ExceptHandler(std::shared_ptr<Expr> type,
                  const std::string& name,
                  std::vector<std::shared_ptr<Stmt>> body,
                  int lineno, int col_offset)
        : ASTNodeBase(lineno, col_offset), type_(type), name_(name), body_(body) {}

    std::shared_ptr<Expr> type() const { return type_; }
    std::string name() const { return name_; }
    const std::vector<std::shared_ptr<Stmt>>& body() const { return body_; }
    std::string to_string(int indent = 0) const override;

private:
    std::shared_ptr<Expr> type_;
    std::string name_;  // Empty if no 'as name'
    std::vector<std::shared_ptr<Stmt>> body_;
};

// Try statement
class Try : public ASTNodeBase {
public:
    Try(std::vector<std::shared_ptr<Stmt>> body,
        std::vector<std::shared_ptr<ExceptHandler>> handlers,
        std::vector<std::shared_ptr<Stmt>> orelse,
        std::vector<std::shared_ptr<Stmt>> finalbody,
        int lineno, int col_offset)
        : ASTNodeBase(lineno, col_offset), body_(body), handlers_(handlers),
          orelse_(orelse), finalbody_(finalbody) {}

    const std::vector<std::shared_ptr<Stmt>>& body() const { return body_; }
    const std::vector<std::shared_ptr<ExceptHandler>>& handlers() const { return handlers_; }
    const std::vector<std::shared_ptr<Stmt>>& orelse() const { return orelse_; }
    const std::vector<std::shared_ptr<Stmt>>& finalbody() const { return finalbody_; }
    std::string to_string(int indent = 0) const override;

private:
    std::vector<std::shared_ptr<Stmt>> body_;
    std::vector<std::shared_ptr<ExceptHandler>> handlers_;
    std::vector<std::shared_ptr<Stmt>> orelse_;
    std::vector<std::shared_ptr<Stmt>> finalbody_;
};

// TryStar statement - try/except* for exception groups (Python 3.11+, PEP 654)
// Reference: Python.asdl - TryStar(stmt* body, excepthandler* handlers, stmt* orelse, stmt* finalbody)
class TryStar : public ASTNodeBase {
public:
    TryStar(std::vector<std::shared_ptr<Stmt>> body,
            std::vector<std::shared_ptr<ExceptHandler>> handlers,
            std::vector<std::shared_ptr<Stmt>> orelse,
            std::vector<std::shared_ptr<Stmt>> finalbody,
            int lineno, int col_offset)
        : ASTNodeBase(lineno, col_offset), body_(std::move(body)), handlers_(std::move(handlers)),
          orelse_(std::move(orelse)), finalbody_(std::move(finalbody)) {}

    const std::vector<std::shared_ptr<Stmt>>& body() const { return body_; }
    const std::vector<std::shared_ptr<ExceptHandler>>& handlers() const { return handlers_; }
    const std::vector<std::shared_ptr<Stmt>>& orelse() const { return orelse_; }
    const std::vector<std::shared_ptr<Stmt>>& finalbody() const { return finalbody_; }
    std::string to_string(int indent = 0) const override;

private:
    std::vector<std::shared_ptr<Stmt>> body_;
    std::vector<std::shared_ptr<ExceptHandler>> handlers_;
    std::vector<std::shared_ptr<Stmt>> orelse_;
    std::vector<std::shared_ptr<Stmt>> finalbody_;
};

// Class definition
class ClassDef : public ASTNodeBase {
public:
    ClassDef(const std::string& name,
             std::vector<std::shared_ptr<Expr>> bases,
             std::vector<std::shared_ptr<Stmt>> body,
             std::vector<std::shared_ptr<Expr>> decorator_list,
             std::vector<std::shared_ptr<TypeParam>> type_params,  // PEP 695: Generic type parameters
             int lineno, int col_offset)
        : ASTNodeBase(lineno, col_offset), name_(name), bases_(bases), body_(body),
          decorator_list_(decorator_list), type_params_(type_params) {}

    std::string name() const { return name_; }
    const std::vector<std::shared_ptr<Expr>>& bases() const { return bases_; }
    const std::vector<std::shared_ptr<Stmt>>& body() const { return body_; }
    const std::vector<std::shared_ptr<Expr>>& decorator_list() const { return decorator_list_; }
    const std::vector<std::shared_ptr<TypeParam>>& type_params() const { return type_params_; }  // PEP 695
    std::string to_string(int indent = 0) const override;

private:
    std::string name_;
    std::vector<std::shared_ptr<Expr>> bases_;  // Empty if no inheritance
    std::vector<std::shared_ptr<Stmt>> body_;
    std::vector<std::shared_ptr<Expr>> decorator_list_;
    std::vector<std::shared_ptr<TypeParam>> type_params_;  // PEP 695: Generic type parameters
};

// Import alias (name [as asname])
class Alias : public ASTNodeBase {
public:
    Alias(const std::string& name, const std::string& asname, int lineno, int col_offset)
        : ASTNodeBase(lineno, col_offset), name_(name), asname_(asname) {}

    std::string name() const { return name_; }
    std::string asname() const { return asname_; }  // Empty if no 'as'
    std::string to_string(int indent = 0) const override;

private:
    std::string name_;
    std::string asname_;
};

// Import statement
class Import : public ASTNodeBase {
public:
    Import(std::vector<std::shared_ptr<Alias>> names, int lineno, int col_offset)
        : ASTNodeBase(lineno, col_offset), names_(names) {}

    const std::vector<std::shared_ptr<Alias>>& names() const { return names_; }
    std::string to_string(int indent = 0) const override;

private:
    std::vector<std::shared_ptr<Alias>> names_;
};

// Import from statement
class ImportFrom : public ASTNodeBase {
public:
    ImportFrom(const std::string& module,
               std::vector<std::shared_ptr<Alias>> names,
               int level,  // 0 = absolute import, >0 = relative import
               int lineno, int col_offset)
        : ASTNodeBase(lineno, col_offset), module_(module), names_(names), level_(level) {}

    std::string module() const { return module_; }  // Empty if relative import
    const std::vector<std::shared_ptr<Alias>>& names() const { return names_; }
    int level() const { return level_; }
    std::string to_string(int indent = 0) const override;

private:
    std::string module_;
    std::vector<std::shared_ptr<Alias>> names_;
    int level_;
};

// With item (context_expr, optional_vars)
struct WithItem {
    std::shared_ptr<Expr> context_expr;
    std::shared_ptr<Expr> optional_vars;  // nullptr if no 'as' clause

    WithItem(std::shared_ptr<Expr> context_expr, std::shared_ptr<Expr> optional_vars)
        : context_expr(context_expr), optional_vars(optional_vars) {}
};

// With statement (context managers)
class With : public ASTNodeBase {
public:
    With(std::vector<WithItem> items,
         std::vector<std::shared_ptr<Stmt>> body,
         int lineno, int col_offset)
        : ASTNodeBase(lineno, col_offset), items_(items), body_(body) {}

    const std::vector<WithItem>& items() const { return items_; }
    const std::vector<std::shared_ptr<Stmt>>& body() const { return body_; }
    std::string to_string(int indent = 0) const override;

private:
    std::vector<WithItem> items_;
    std::vector<std::shared_ptr<Stmt>> body_;
};

// Async with statement (Python 3.5+)
class AsyncWith : public ASTNodeBase {
public:
    AsyncWith(std::vector<WithItem> items,
              std::vector<std::shared_ptr<Stmt>> body,
              int lineno, int col_offset)
        : ASTNodeBase(lineno, col_offset), items_(items), body_(body) {}

    const std::vector<WithItem>& items() const { return items_; }
    const std::vector<std::shared_ptr<Stmt>>& body() const { return body_; }
    std::string to_string(int indent = 0) const override;

private:
    std::vector<WithItem> items_;
    std::vector<std::shared_ptr<Stmt>> body_;
};

// Match/Case Pattern Matching (Python 3.10+)
// Simplified implementation for basic pattern matching

// match_case represents a single case block in a match statement
class match_case {
public:
    match_case(std::shared_ptr<Expr> pattern,
               std::shared_ptr<Expr> guard,
               std::vector<std::shared_ptr<Stmt>> body)
        : pattern_(pattern), guard_(guard), body_(body) {}
    
    std::shared_ptr<Expr> pattern() const { return pattern_; }
    std::shared_ptr<Expr> guard() const { return guard_; }
    const std::vector<std::shared_ptr<Stmt>>& body() const { return body_; }
    
private:
    std::shared_ptr<Expr> pattern_;  // Pattern to match (simplified as Expr)
    std::shared_ptr<Expr> guard_;    // Optional guard condition (if clause)
    std::vector<std::shared_ptr<Stmt>> body_;  // Statements to execute if matched
};

// Match statement
class Match : public ASTNodeBase {
public:
    Match(std::shared_ptr<Expr> subject,
          std::vector<match_case> cases,
          int lineno, int col_offset)
        : ASTNodeBase(lineno, col_offset), subject_(subject), cases_(cases) {}
    
    std::shared_ptr<Expr> subject() const { return subject_; }
    const std::vector<match_case>& cases() const { return cases_; }
    std::string to_string(int indent = 0) const override;
    
private:
    std::shared_ptr<Expr> subject_;  // Expression to match against
    std::vector<match_case> cases_;  // List of case blocks
};

// ============================================================================
// Type Parameter AST Nodes (Python 3.12+ PEP 695)
// ============================================================================

/**
 * Base class for type parameters
 * Reference: Python.asdl - type_param
 */
class TypeParam : public ASTNodeBase {
public:
    TypeParam(int lineno, int col_offset)
        : ASTNodeBase(lineno, col_offset) {}
    
    virtual ~TypeParam() = default;
    virtual std::string to_string(int indent = 0) const override = 0;
};

/**
 * TypeVar - A type variable parameter
 * Reference: Python.asdl - TypeVar(identifier name, expr? bound, expr? default_value)
 * 
 * Examples:
 *   T           - simple type variable
 *   T: int      - type variable with bound
 *   T = int     - type variable with default (Python 3.13+)
 */
class TypeVar : public TypeParam {
public:
    TypeVar(const std::string& name,
            std::shared_ptr<Expr> bound,          // Optional bound constraint
            std::shared_ptr<Expr> default_value,  // Optional default (Python 3.13+)
            int lineno, int col_offset)
        : TypeParam(lineno, col_offset),
          name_(name),
          bound_(bound),
          default_value_(default_value) {}

    const std::string& name() const { return name_; }
    std::shared_ptr<Expr> bound() const { return bound_; }
    std::shared_ptr<Expr> default_value() const { return default_value_; }
    
    std::string to_string(int indent = 0) const override;

private:
    std::string name_;
    std::shared_ptr<Expr> bound_;
    std::shared_ptr<Expr> default_value_;
};

/**
 * ParamSpec - A parameter specification variable
 * Reference: Python.asdl - ParamSpec(identifier name, expr? default_value)
 * 
 * Examples:
 *   **P         - parameter specification
 *   **P = ...   - with default (Python 3.13+)
 */
class ParamSpec : public TypeParam {
public:
    ParamSpec(const std::string& name,
              std::shared_ptr<Expr> default_value,  // Optional default (Python 3.13+)
              int lineno, int col_offset)
        : TypeParam(lineno, col_offset),
          name_(name),
          default_value_(default_value) {}

    const std::string& name() const { return name_; }
    std::shared_ptr<Expr> default_value() const { return default_value_; }
    
    std::string to_string(int indent = 0) const override;

private:
    std::string name_;
    std::shared_ptr<Expr> default_value_;
};

/**
 * TypeVarTuple - A type variable tuple (variadic type parameter)
 * Reference: Python.asdl - TypeVarTuple(identifier name, expr? default_value)
 * 
 * Examples:
 *   *Ts         - type variable tuple
 *   *Ts = ...   - with default (Python 3.13+)
 */
class TypeVarTuple : public TypeParam {
public:
    TypeVarTuple(const std::string& name,
                 std::shared_ptr<Expr> default_value,  // Optional default (Python 3.13+)
                 int lineno, int col_offset)
        : TypeParam(lineno, col_offset),
          name_(name),
          default_value_(default_value) {}

    const std::string& name() const { return name_; }
    std::shared_ptr<Expr> default_value() const { return default_value_; }
    
    std::string to_string(int indent = 0) const override;

private:
    std::string name_;
    std::shared_ptr<Expr> default_value_;
};

// ============================================================================
// TypeAlias Statement Node (Python 3.12+ PEP 695)
// ============================================================================

/**
 * TypeAlias - A type alias statement
 * Reference: Python.asdl - TypeAlias(expr name, type_param* type_params, expr value)
 * 
 * Represents the Python 3.12+ type alias syntax:
 *   type Point = tuple[float, float]
 *   type Vector[T] = list[T]
 *   type Callback[**P, R] = Callable[P, R]
 * 
 * Grammar (from python.gram):
 *   type_alias: "type" NAME [type_params] '=' expression
 */
class TypeAlias : public ASTNodeBase {
public:
    /**
     * Construct a TypeAlias node
     * 
     * @param name        The alias name as a Name expression
     * @param type_params List of type parameters (empty if non-generic)
     * @param value       The type expression being aliased
     * @param lineno      Source line number
     * @param col_offset  Source column offset
     */
    TypeAlias(std::shared_ptr<Expr> name,
              std::vector<std::shared_ptr<TypeParam>> type_params,
              std::shared_ptr<Expr> value,
              int lineno, int col_offset)
        : ASTNodeBase(lineno, col_offset),
          name_(name),
          type_params_(std::move(type_params)),
          value_(value) {}

    // Accessors
    std::shared_ptr<Expr> name() const { return name_; }
    const std::vector<std::shared_ptr<TypeParam>>& type_params() const { return type_params_; }
    std::shared_ptr<Expr> value() const { return value_; }
    
    // Check if this is a generic type alias
    bool is_generic() const { return !type_params_.empty(); }
    
    std::string to_string(int indent = 0) const override;

private:
    std::shared_ptr<Expr> name_;                          // The alias name (as Name expr)
    std::vector<std::shared_ptr<TypeParam>> type_params_; // Generic type parameters
    std::shared_ptr<Expr> value_;                         // The aliased type expression
};

// Implementations
inline std::string ExceptHandler::to_string(int indent) const {
    std::ostringstream oss;
    oss << indent_str(indent) << "ExceptHandler(\n";
    if (type_) {
        oss << indent_str(indent + 1) << "type=\n";
        oss << type_->to_string(indent + 2) << ",\n";
    } else {
        oss << indent_str(indent + 1) << "type=None,\n";
    }
    if (!name_.empty()) {
        oss << indent_str(indent + 1) << "name=\"" << name_ << "\",\n";
    }
    oss << indent_str(indent + 1) << "body=[\n";
    for (const auto& stmt : body_) {
        oss << stmt->to_string(indent + 2) << ",\n";
    }
    oss << indent_str(indent + 1) << "]\n";
    oss << indent_str(indent) << ")";
    return oss.str();
}

inline std::string Try::to_string(int indent) const {
    std::ostringstream oss;
    oss << indent_str(indent) << "Try(\n";
    oss << indent_str(indent + 1) << "body=[\n";
    for (const auto& stmt : body_) {
        oss << stmt->to_string(indent + 2) << ",\n";
    }
    oss << indent_str(indent + 1) << "],\n";
    if (!handlers_.empty()) {
        oss << indent_str(indent + 1) << "handlers=[\n";
        for (const auto& handler : handlers_) {
            oss << handler->to_string(indent + 2) << ",\n";
        }
        oss << indent_str(indent + 1) << "],\n";
    }
    if (!orelse_.empty()) {
        oss << indent_str(indent + 1) << "orelse=[\n";
        for (const auto& stmt : orelse_) {
            oss << stmt->to_string(indent + 2) << ",\n";
        }
        oss << indent_str(indent + 1) << "],\n";
    }
    if (!finalbody_.empty()) {
        oss << indent_str(indent + 1) << "finalbody=[\n";
        for (const auto& stmt : finalbody_) {
            oss << stmt->to_string(indent + 2) << ",\n";
        }
        oss << indent_str(indent + 1) << "],\n";
    }
    oss << indent_str(indent) << ")";
    return oss.str();
}

inline std::string TryStar::to_string(int indent) const {
    std::ostringstream oss;
    oss << indent_str(indent) << "TryStar(\n";
    oss << indent_str(indent + 1) << "body=[\n";
    for (const auto& stmt : body_) {
        oss << stmt->to_string(indent + 2) << ",\n";
    }
    oss << indent_str(indent + 1) << "],\n";
    oss << indent_str(indent + 1) << "handlers=[\n";
    for (const auto& handler : handlers_) {
        oss << handler->to_string(indent + 2) << ",\n";
    }
    oss << indent_str(indent + 1) << "],\n";
    if (!orelse_.empty()) {
        oss << indent_str(indent + 1) << "orelse=[\n";
        for (const auto& stmt : orelse_) {
            oss << stmt->to_string(indent + 2) << ",\n";
        }
        oss << indent_str(indent + 1) << "],\n";
    }
    if (!finalbody_.empty()) {
        oss << indent_str(indent + 1) << "finalbody=[\n";
        for (const auto& stmt : finalbody_) {
            oss << stmt->to_string(indent + 2) << ",\n";
        }
        oss << indent_str(indent + 1) << "],\n";
    }
    oss << indent_str(indent) << ")";
    return oss.str();
}

inline std::string ClassDef::to_string(int indent) const {
    std::ostringstream oss;
    oss << indent_str(indent) << "ClassDef(\n";
    oss << indent_str(indent + 1) << "name=\"" << name_ << "\",\n";
    // PEP 695: Print type_params count (detailed output is at end of file)
    if (!type_params_.empty()) {
        oss << indent_str(indent + 1) << "type_params=[" << type_params_.size() << " params],\n";
    }
    if (!decorator_list_.empty()) {
        oss << indent_str(indent + 1) << "decorator_list=[\n";
        for (const auto& deco : decorator_list_) {
            oss << deco->to_string(indent + 2) << ",\n";
        }
        oss << indent_str(indent + 1) << "],\n";
    }
    if (!bases_.empty()) {
        oss << indent_str(indent + 1) << "bases=[\n";
        for (const auto& base : bases_) {
            oss << base->to_string(indent + 2) << ",\n";
        }
        oss << indent_str(indent + 1) << "],\n";
    }
    oss << indent_str(indent + 1) << "body=[\n";
    for (const auto& stmt : body_) {
        oss << stmt->to_string(indent + 2) << ",\n";
    }
    oss << indent_str(indent + 1) << "]\n";
    oss << indent_str(indent) << ")";
    return oss.str();
}

inline std::string Alias::to_string(int indent) const {
    std::ostringstream oss;
    oss << indent_str(indent) << "Alias(\n";
    oss << indent_str(indent + 1) << "name=\"" << name_ << "\"";
    if (!asname_.empty()) {
        oss << ",\n";
        oss << indent_str(indent + 1) << "asname=\"" << asname_ << "\"";
    }
    oss << "\n";
    oss << indent_str(indent) << ")";
    return oss.str();
}

inline std::string Import::to_string(int indent) const {
    std::ostringstream oss;
    oss << indent_str(indent) << "Import(\n";
    oss << indent_str(indent + 1) << "names=[\n";
    for (const auto& alias : names_) {
        oss << alias->to_string(indent + 2) << ",\n";
    }
    oss << indent_str(indent + 1) << "]\n";
    oss << indent_str(indent) << ")";
    return oss.str();
}

inline std::string ImportFrom::to_string(int indent) const {
    std::ostringstream oss;
    oss << indent_str(indent) << "ImportFrom(\n";
    if (!module_.empty()) {
        oss << indent_str(indent + 1) << "module=\"" << module_ << "\",\n";
    }
    oss << indent_str(indent + 1) << "level=" << level_ << ",\n";
    oss << indent_str(indent + 1) << "names=[\n";
    for (const auto& alias : names_) {
        oss << alias->to_string(indent + 2) << ",\n";
    }
    oss << indent_str(indent + 1) << "]\n";
    oss << indent_str(indent) << ")";
    return oss.str();
}

inline std::string With::to_string(int indent) const {
    std::ostringstream oss;
    oss << indent_str(indent) << "With(\n";
    oss << indent_str(indent + 1) << "items=[\n";
    for (const auto& item : items_) {
        oss << indent_str(indent + 2) << "WithItem(\n";
        oss << indent_str(indent + 3) << "context_expr=\n";
        oss << item.context_expr->to_string(indent + 4) << ",\n";
        if (item.optional_vars) {
            oss << indent_str(indent + 3) << "optional_vars=\n";
            oss << item.optional_vars->to_string(indent + 4) << "\n";
        } else {
            oss << indent_str(indent + 3) << "optional_vars=None\n";
        }
        oss << indent_str(indent + 2) << "),\n";
    }
    oss << indent_str(indent + 1) << "],\n";
    oss << indent_str(indent + 1) << "body=[\n";
    for (const auto& stmt : body_) {
        oss << stmt->to_string(indent + 2) << ",\n";
    }
    oss << indent_str(indent + 1) << "]\n";
    oss << indent_str(indent) << ")";
    return oss.str();
}

inline std::string AsyncWith::to_string(int indent) const {
    std::ostringstream oss;
    oss << indent_str(indent) << "AsyncWith(\n";
    oss << indent_str(indent + 1) << "items=[\n";
    for (const auto& item : items_) {
        oss << indent_str(indent + 2) << "WithItem(\n";
        oss << indent_str(indent + 3) << "context_expr=\n";
        oss << item.context_expr->to_string(indent + 4) << ",\n";
        if (item.optional_vars) {
            oss << indent_str(indent + 3) << "optional_vars=\n";
            oss << item.optional_vars->to_string(indent + 4) << "\n";
        } else {
            oss << indent_str(indent + 3) << "optional_vars=None\n";
        }
        oss << indent_str(indent + 2) << "),\n";
    }
    oss << indent_str(indent + 1) << "],\n";
    oss << indent_str(indent + 1) << "body=[\n";
    for (const auto& stmt : body_) {
        oss << stmt->to_string(indent + 2) << ",\n";
    }
    oss << indent_str(indent + 1) << "]\n";
    oss << indent_str(indent) << ")";
    return oss.str();
}

// Implementations for new statements
inline std::string Pass::to_string(int indent) const {
    return indent_str(indent) + "Pass()";
}

inline std::string Raise::to_string(int indent) const {
    std::ostringstream oss;
    oss << indent_str(indent) << "Raise(\n";
    if (exc_) {
        oss << indent_str(indent + 1) << "exc=\n";
        oss << exc_->to_string(indent + 2) << ",\n";
    } else {
        oss << indent_str(indent + 1) << "exc=None,\n";
    }
    if (cause_) {
        oss << indent_str(indent + 1) << "cause=\n";
        oss << cause_->to_string(indent + 2) << "\n";
    } else {
        oss << indent_str(indent + 1) << "cause=None\n";
    }
    oss << indent_str(indent) << ")";
    return oss.str();
}

inline std::string Delete::to_string(int indent) const {
    std::ostringstream oss;
    oss << indent_str(indent) << "Delete(\n";
    oss << indent_str(indent + 1) << "targets=[\n";
    for (const auto& target : targets_) {
        oss << target->to_string(indent + 2) << ",\n";
    }
    oss << indent_str(indent + 1) << "]\n";
    oss << indent_str(indent) << ")";
    return oss.str();
}

inline std::string Assert::to_string(int indent) const {
    std::ostringstream oss;

    oss << indent_str(indent) << "Assert(\n";
    oss << indent_str(indent + 1) << "test=\n";
    oss << test_->to_string(indent + 2) << ",\n";
    if (msg_) {
        oss << indent_str(indent + 1) << "msg=\n";
        oss << msg_->to_string(indent + 2) << "\n";
    } else {
        oss << indent_str(indent + 1) << "msg=None\n";
    }
    oss << indent_str(indent) << ")";
    return oss.str();
}

inline std::string Global::to_string(int indent) const {
    std::ostringstream oss;
    oss << indent_str(indent) << "Global(\n";
    oss << indent_str(indent + 1) << "names=[";
    for (size_t i = 0; i < names_.size(); ++i) {
        oss << "\"" << names_[i] << "\"";
        if (i < names_.size() - 1) oss << ", ";
    }
    oss << "]\n";
    oss << indent_str(indent) << ")";
    return oss.str();
}

inline std::string Nonlocal::to_string(int indent) const {
    std::ostringstream oss;
    oss << indent_str(indent) << "Nonlocal(\n";
    oss << indent_str(indent + 1) << "names=[";
    for (size_t i = 0; i < names_.size(); ++i) {
        oss << "\"" << names_[i] << "\"";
        if (i < names_.size() - 1) oss << ", ";
    }
    oss << "]\n";
    oss << indent_str(indent) << ")";
    return oss.str();
}


// Match statement implementation
inline std::string Match::to_string(int indent) const {
    std::ostringstream oss;
    oss << indent_str(indent) << "Match(\n";
    oss << indent_str(indent + 1) << "subject=\n";
    oss << subject_->to_string(indent + 2) << ",\n";
    oss << indent_str(indent + 1) << "cases=[\n";
    for (const auto& case_block : cases_) {
        oss << indent_str(indent + 2) << "match_case(\n";
        oss << indent_str(indent + 3) << "pattern=\n";
        oss << case_block.pattern()->to_string(indent + 4) << ",\n";
        if (case_block.guard()) {
            oss << indent_str(indent + 3) << "guard=\n";
            oss << case_block.guard()->to_string(indent + 4) << ",\n";
        }
        oss << indent_str(indent + 3) << "body=[\n";
        for (const auto& stmt : case_block.body()) {
            oss << stmt->to_string(indent + 4) << ",\n";
        }
        oss << indent_str(indent + 3) << "]\n";
        oss << indent_str(indent + 2) << "),\n";
    }
    oss << indent_str(indent + 1) << "]\n";
    oss << indent_str(indent) << ")";
    return oss.str();
}

// TypeVar to_string implementation
inline std::string TypeVar::to_string(int indent) const {
    std::ostringstream oss;
    oss << indent_str(indent) << "TypeVar(\n";
    oss << indent_str(indent + 1) << "name=\"" << name_ << "\"";
    if (bound_) {
        oss << ",\n" << indent_str(indent + 1) << "bound=\n";
        oss << bound_->to_string(indent + 2);
    }
    if (default_value_) {
        oss << ",\n" << indent_str(indent + 1) << "default_value=\n";
        oss << default_value_->to_string(indent + 2);
    }
    oss << "\n" << indent_str(indent) << ")";
    return oss.str();
}

// ParamSpec to_string implementation
inline std::string ParamSpec::to_string(int indent) const {
    std::ostringstream oss;
    oss << indent_str(indent) << "ParamSpec(\n";
    oss << indent_str(indent + 1) << "name=\"" << name_ << "\"";
    if (default_value_) {
        oss << ",\n" << indent_str(indent + 1) << "default_value=\n";
        oss << default_value_->to_string(indent + 2);
    }
    oss << "\n" << indent_str(indent) << ")";
    return oss.str();
}

// TypeVarTuple to_string implementation
inline std::string TypeVarTuple::to_string(int indent) const {
    std::ostringstream oss;
    oss << indent_str(indent) << "TypeVarTuple(\n";
    oss << indent_str(indent + 1) << "name=\"" << name_ << "\"";
    if (default_value_) {
        oss << ",\n" << indent_str(indent + 1) << "default_value=\n";
        oss << default_value_->to_string(indent + 2);
    }
    oss << "\n" << indent_str(indent) << ")";
    return oss.str();
}

// TypeAlias to_string implementation
inline std::string TypeAlias::to_string(int indent) const {
    std::ostringstream oss;
    oss << indent_str(indent) << "TypeAlias(\n";
    
    // Name
    oss << indent_str(indent + 1) << "name=\n";
    oss << name_->to_string(indent + 2) << ",\n";
    
    // Type parameters (if any)
    if (!type_params_.empty()) {
        oss << indent_str(indent + 1) << "type_params=[\n";
        for (const auto& param : type_params_) {
            oss << param->to_string(indent + 2) << ",\n";
        }
        oss << indent_str(indent + 1) << "],\n";
    } else {
        oss << indent_str(indent + 1) << "type_params=[],\n";
    }
    
    // Value (the aliased type)
    oss << indent_str(indent + 1) << "value=\n";
    oss << value_->to_string(indent + 2) << "\n";
    
    oss << indent_str(indent) << ")";
    return oss.str();
}

} // namespace ast
} // namespace cpython_cpp
#endif // CPYTHON_CPP_AST_STMT_HPP


