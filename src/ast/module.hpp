#ifndef CPYTHON_CPP_AST_MODULE_HPP
#define CPYTHON_CPP_AST_MODULE_HPP

#include "node.hpp"
#include "stmt.hpp"
#include <vector>
#include <memory>
#include <string>
#include <iostream>
#include <sstream>

namespace cpython_cpp {
namespace ast {

/**
 * Module AST node (top-level)
 * Reference: Parser/Python.asdl (mod definitions)
 */
class Module : public ASTNodeBase {
public:
    Module(std::vector<std::shared_ptr<Stmt>> body)
        : ASTNodeBase(1, 0), body_(body) {}

    const std::vector<std::shared_ptr<Stmt>>& body() const { return body_; }
    void add_stmt(std::shared_ptr<Stmt> stmt) { body_.push_back(stmt); }

    std::string to_string(int indent = 0) const override;

    // Pretty print the entire module
    void print() const {
        std::cout << to_string(0) << std::endl;
    }

private:
    std::vector<std::shared_ptr<Stmt>> body_;
};

// Implementation
inline std::string Module::to_string(int indent) const {
    std::ostringstream oss;
    oss << indent_str(indent) << "Module(\n";
    oss << indent_str(indent + 1) << "body=[\n";
    for (size_t i = 0; i < body_.size(); ++i) {
        oss << body_[i]->to_string(indent + 2);
        if (i < body_.size() - 1) oss << ",";
        oss << "\n";
    }
    oss << indent_str(indent + 1) << "]\n";
    oss << indent_str(indent) << ")";
    return oss.str();
}

} // namespace ast
} // namespace cpython_cpp

#endif // CPYTHON_CPP_AST_MODULE_HPP



