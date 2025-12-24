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

// Function definition
class FunctionDef : public ASTNodeBase {
public:
    FunctionDef(const std::string& name,
                std::vector<std::string> args,
                std::vector<std::shared_ptr<Stmt>> body,
                std::vector<std::shared_ptr<Expr>> decorator_list,
                int lineno, int col_offset)
        : ASTNodeBase(lineno, col_offset), name_(name), args_(args), body_(body),
          decorator_list_(decorator_list) {}

    std::string name() const { return name_; }
    const std::vector<std::string>& args() const { return args_; }
    const std::vector<std::shared_ptr<Stmt>>& body() const { return body_; }
    const std::vector<std::shared_ptr<Expr>>& decorator_list() const { return decorator_list_; }
    std::string to_string(int indent = 0) const override;

private:
    std::string name_;
    std::vector<std::string> args_;
    std::vector<std::shared_ptr<Stmt>> body_;
    std::vector<std::shared_ptr<Expr>> decorator_list_;
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

// Stmt is already defined in node.hpp as ASTNode

// Implementations
inline std::string FunctionDef::to_string(int indent) const {
    std::ostringstream oss;
    oss << indent_str(indent) << "FunctionDef(\n";
    oss << indent_str(indent + 1) << "name='" << name_ << "',\n";
    if (!decorator_list_.empty()) {
        oss << indent_str(indent + 1) << "decorator_list=[\n";
        for (const auto& deco : decorator_list_) {
            oss << deco->to_string(indent + 2) << ",\n";
        }
        oss << indent_str(indent + 1) << "],\n";
    }
    oss << indent_str(indent + 1) << "args=[";
    for (size_t i = 0; i < args_.size(); ++i) {
        oss << "'" << args_[i] << "'";
        if (i < args_.size() - 1) oss << ", ";
    }
    oss << "],\n";
    oss << indent_str(indent + 1) << "body=[\n";
    for (const auto& stmt : body_) {
        oss << stmt->to_string(indent + 2) << ",\n";
    }
    oss << indent_str(indent + 1) << "]\n";
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

// Class definition
class ClassDef : public ASTNodeBase {
public:
    ClassDef(const std::string& name,
             std::vector<std::shared_ptr<Expr>> bases,
             std::vector<std::shared_ptr<Stmt>> body,
             std::vector<std::shared_ptr<Expr>> decorator_list,
             int lineno, int col_offset)
        : ASTNodeBase(lineno, col_offset), name_(name), bases_(bases), body_(body),
          decorator_list_(decorator_list) {}

    std::string name() const { return name_; }
    const std::vector<std::shared_ptr<Expr>>& bases() const { return bases_; }
    const std::vector<std::shared_ptr<Stmt>>& body() const { return body_; }
    const std::vector<std::shared_ptr<Expr>>& decorator_list() const { return decorator_list_; }
    std::string to_string(int indent = 0) const override;

private:
    std::string name_;
    std::vector<std::shared_ptr<Expr>> bases_;  // Empty if no inheritance
    std::vector<std::shared_ptr<Stmt>> body_;
    std::vector<std::shared_ptr<Expr>> decorator_list_;
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

inline std::string ClassDef::to_string(int indent) const {
    std::ostringstream oss;
    oss << indent_str(indent) << "ClassDef(\n";
    oss << indent_str(indent + 1) << "name=\"" << name_ << "\",\n";
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

} // namespace ast
} // namespace cpython_cpp

#endif // CPYTHON_CPP_AST_STMT_HPP


