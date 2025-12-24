#ifndef CPYTHON_CPP_PARSER_HPP
#define CPYTHON_CPP_PARSER_HPP

#include "tokenizer.hpp"
#include "../ast/module.hpp"
#include "../ast/stmt.hpp"
#include "../ast/expr.hpp"
#include <memory>
#include <vector>
#include <stdexcept>
#include <sstream>

namespace cpython_cpp {
namespace parser {

/**
 * Parser - converts tokens to AST
 * Reference: Parser/parser.c, Parser/pegen.c
 *
 * This is a simplified recursive descent parser.
 * CPython uses a PEG parser generated from Grammar/python.gram
 */
class Parser {
public:
    explicit Parser(const std::string& source);

    // Parse the source code and return an AST Module
    std::shared_ptr<ast::Module> parse();

private:
    Tokenizer tokenizer_;
    std::vector<Token> tokens_;
    size_t current_token_;

    // Current token helpers
    Token current() const;
    Token peek() const;
    void advance();
    bool match(TokenType type);
    bool is_at_end() const;

    // Parsing methods (recursive descent)
    std::shared_ptr<ast::Module> parse_module();
    std::shared_ptr<ast::Stmt> parse_stmt();
    
    // Helper to check if current token is an augmented assignment operator
    bool is_augmented_assign() const;
    
    // Helper to convert augmented assignment token to Operator
    ast::Operator token_to_operator(TokenType type) const;
    std::shared_ptr<ast::Stmt> parse_function_def();
    std::shared_ptr<ast::Stmt> parse_function_def_raw();
    std::vector<std::shared_ptr<ast::Expr>> parse_decorators();
    std::shared_ptr<ast::Stmt> parse_return();
    std::shared_ptr<ast::Stmt> parse_assign();
    std::shared_ptr<ast::Stmt> parse_if();
    std::shared_ptr<ast::Stmt> parse_elif_stmt();
    std::vector<std::shared_ptr<ast::Stmt>> parse_else_block();
    std::shared_ptr<ast::Stmt> parse_while();
    std::shared_ptr<ast::Stmt> parse_for();
    std::shared_ptr<ast::Stmt> parse_async_function_def();  // Python 3.5+
    std::shared_ptr<ast::Stmt> parse_async_for();           // Python 3.5+
    std::shared_ptr<ast::Stmt> parse_async_with();          // Python 3.5+
    std::shared_ptr<ast::Stmt> parse_break();
    std::shared_ptr<ast::Stmt> parse_continue();
    std::shared_ptr<ast::Stmt> parse_expr_stmt();

    // Simple statements (matches CPython's pass_stmt_rule, raise_stmt_rule, etc.)
    std::shared_ptr<ast::Stmt> parse_pass_stmt();
    std::shared_ptr<ast::Stmt> parse_raise_stmt();
    std::shared_ptr<ast::Stmt> parse_del_stmt();
    std::shared_ptr<ast::Stmt> parse_assert_stmt();
    std::shared_ptr<ast::Stmt> parse_global_stmt();
    std::shared_ptr<ast::Stmt> parse_nonlocal_stmt();

    std::shared_ptr<ast::Expr> parse_expr();
    std::shared_ptr<ast::Expr> parse_disjunction();  // or (lowest precedence)
    std::shared_ptr<ast::Expr> parse_conjunction(); // and
    std::shared_ptr<ast::Expr> parse_inversion();   // not
    std::shared_ptr<ast::Expr> parse_comparison();   // <, >, ==, etc.
    std::shared_ptr<ast::Expr> parse_bitwise_or();   // |
    std::shared_ptr<ast::Expr> parse_bitwise_xor();  // ^
    std::shared_ptr<ast::Expr> parse_bitwise_and();  // &
    std::shared_ptr<ast::Expr> parse_shift_expr();    // <<, >>
    std::shared_ptr<ast::Expr> parse_sum();           // +, -
    std::shared_ptr<ast::Expr> parse_term();         // *, /, %, //
    std::shared_ptr<ast::Expr> parse_factor();       // **, unary +, -, ~
    std::shared_ptr<ast::Expr> parse_primary();
    std::shared_ptr<ast::Expr> parse_atom();
    std::shared_ptr<ast::Expr> parse_call();
    std::shared_ptr<ast::Expr> parse_list();
    std::shared_ptr<ast::Expr> parse_dict();

    // CPython-style parsing for for loops (star_targets and star_expressions)
    std::shared_ptr<ast::Expr> parse_star_targets();
    std::shared_ptr<ast::Expr> parse_star_target();
    std::shared_ptr<ast::Expr> parse_star_expressions();
    std::shared_ptr<ast::Expr> parse_star_expression();

    // Helper for parsing argument lists
    std::vector<std::string> parse_arg_list();
    std::vector<std::shared_ptr<ast::Expr>> parse_expr_list();

    // CPython-style arguments parsing (matches arguments_rule and args_rule)
    std::shared_ptr<ast::Call> parse_arguments();  // Returns Call node with args (matches arguments_rule)
    std::shared_ptr<ast::Call> parse_args();      // Returns Call node (matches args_rule)

    // Try/except/finally (matches try_stmt_rule, except_block_rule, finally_block_rule)
    std::shared_ptr<ast::Stmt> parse_try_stmt();
    std::shared_ptr<ast::ExceptHandler> parse_except_block();
    std::vector<std::shared_ptr<ast::Stmt>> parse_finally_block();

    // Class definition (matches class_def_rule, class_def_raw_rule)
    std::shared_ptr<ast::Stmt> parse_class_def();
    std::shared_ptr<ast::Stmt> parse_class_def_raw();

    // Import statements (matches import_stmt_rule, import_name_rule, import_from_rule)
    std::shared_ptr<ast::Stmt> parse_import_stmt();
    std::shared_ptr<ast::Stmt> parse_import_name();
    std::shared_ptr<ast::Stmt> parse_import_from();
    std::shared_ptr<ast::Expr> parse_dotted_name();
    std::shared_ptr<ast::Alias> parse_dotted_as_name();
    std::vector<std::shared_ptr<ast::Alias>> parse_dotted_as_names();
    std::shared_ptr<ast::Alias> parse_import_from_as_name();
    std::vector<std::shared_ptr<ast::Alias>> parse_import_from_as_names();
    std::vector<std::shared_ptr<ast::Alias>> parse_import_from_targets();

    // With statement (matches with_stmt_rule, with_item_rule)
    std::shared_ptr<ast::Stmt> parse_with_stmt();
    ast::WithItem parse_with_item();

    // Lambda and yield expressions
    std::shared_ptr<ast::Expr> parse_lambda();
    std::shared_ptr<ast::Expr> parse_yield_expr();

    // Conditional expression (ternary operator): x if condition else y
    std::shared_ptr<ast::Expr> parse_conditional();

    // Comprehensions
    std::shared_ptr<ast::Expr> parse_comprehension();
    ast::Comprehension parse_for_if_clause();
    std::vector<ast::Comprehension> parse_for_if_clauses();

    // Slicing: [start:end:step]
    // Matches CPython's slice_rule: returns nullptr if not a slice (allows fallback to named_expression)
    std::shared_ptr<ast::Expr> parse_slice();

    // F-string parsing
    std::shared_ptr<ast::Expr> parse_fstring();
    std::shared_ptr<ast::Expr> parse_formatted_value();
    std::shared_ptr<ast::Expr> parse_fstring_format_spec();

    // Error handling
    void error(const std::string& message);
};

// Implementations
inline Parser::Parser(const std::string& source)
    : tokenizer_(source), current_token_(0) {
    tokens_ = tokenizer_.tokenize();

    // Print full token stream for debugging
    std::cerr << "\n[DEBUG Parser] Full token stream (" << tokens_.size() << " tokens):" << std::endl;
    for (size_t i = 0; i < tokens_.size(); i++) {
        std::cerr << "  Token[" << i << "] = type=" << static_cast<int>(tokens_[i].type)
                  << ", value='" << tokens_[i].value << "'"
                  << ", line=" << tokens_[i].line << ", col=" << tokens_[i].column << std::endl;
    }
    std::cerr << std::endl;
    std::cerr.flush();
}

inline Token Parser::current() const {
    if (current_token_ >= tokens_.size()) {
        return tokens_.back(); // Return END_OF_FILE
    }
    return tokens_[current_token_];
}

inline Token Parser::peek() const {
    if (current_token_ + 1 >= tokens_.size()) {
        return tokens_.back(); // Return END_OF_FILE
    }
    return tokens_[current_token_ + 1];
}

inline void Parser::advance() {
    if (!is_at_end()) {
        current_token_++;
    }
}

inline bool Parser::match(TokenType type) {
    if (current().type == type) {
        advance();
        return true;
    }
    return false;
}

inline bool Parser::is_at_end() const {
    return current().type == TokenType::END_OF_FILE;
}

inline bool Parser::is_augmented_assign() const {
    TokenType type = current().type;
    return type == TokenType::PLUSEQUAL ||
           type == TokenType::MINEQUAL ||
           type == TokenType::STAREQUAL ||
           type == TokenType::SLASHEQUAL ||
           type == TokenType::PERCENTEQUAL ||
           type == TokenType::DOUBLESTAREQUAL ||
           type == TokenType::DOUBLESLASHEQUAL ||
           type == TokenType::AMPEREQUAL ||
           type == TokenType::VBAREQUAL ||
           type == TokenType::CIRCUMFLEXEQUAL ||
           type == TokenType::LEFTSHIFTEQUAL ||
           type == TokenType::RIGHTSHIFTEQUAL;
}

inline ast::Operator Parser::token_to_operator(TokenType type) const {
    switch (type) {
        case TokenType::PLUSEQUAL: return ast::Operator::Add;
        case TokenType::MINEQUAL: return ast::Operator::Sub;
        case TokenType::STAREQUAL: return ast::Operator::Mult;
        case TokenType::SLASHEQUAL: return ast::Operator::Div;
        case TokenType::PERCENTEQUAL: return ast::Operator::Mod;
        case TokenType::DOUBLESTAREQUAL: return ast::Operator::Pow;
        case TokenType::DOUBLESLASHEQUAL: return ast::Operator::FloorDiv;
        case TokenType::AMPEREQUAL: return ast::Operator::BitAnd;
        case TokenType::VBAREQUAL: return ast::Operator::BitOr;
        case TokenType::CIRCUMFLEXEQUAL: return ast::Operator::BitXor;
        case TokenType::LEFTSHIFTEQUAL: return ast::Operator::LShift;
        case TokenType::RIGHTSHIFTEQUAL: return ast::Operator::RShift;
        default:
            throw std::runtime_error("Invalid augmented assignment operator");
            return ast::Operator::Add; // Never reached
    }
}

inline void Parser::error(const std::string& message) {
    std::ostringstream oss;
    oss << "Parse error at line " << current().line
        << ", column " << current().column << ": " << message;
    throw std::runtime_error(oss.str());
}

inline std::shared_ptr<ast::Module> Parser::parse() {
    return parse_module();
}

inline std::shared_ptr<ast::Module> Parser::parse_module() {
    std::cerr << "[DEBUG parse_module] Starting, token=" << current_token_ << std::endl;
    std::cerr.flush();
    auto module = std::make_shared<ast::Module>(std::vector<std::shared_ptr<ast::Stmt>>());

    while (!is_at_end() && current().type != TokenType::END_OF_FILE) {
        if (current().type == TokenType::NEWLINE) {
            advance();
            continue;
        }
        std::cerr << "[DEBUG parse_module] Parsing statement, token=" << current_token_
                  << ", type=" << static_cast<int>(current().type)
                  << ", value='" << current().value << "'" << std::endl;
        std::cerr.flush();
        auto stmt = parse_stmt();
        if (stmt) {
            module->add_stmt(stmt);
            std::cerr << "[DEBUG parse_module] Statement parsed, token=" << current_token_ << std::endl;
            std::cerr.flush();
        }
    }
    std::cerr << "[DEBUG parse_module] Complete, " << module->body().size() << " statements" << std::endl;
    std::cerr.flush();
    return module;
}

inline std::shared_ptr<ast::Stmt> Parser::parse_stmt() {
    std::cerr << "[DEBUG parse_stmt] Entry, token=" << current_token_
              << ", type=" << static_cast<int>(current().type)
              << ", value='" << current().value << "'" << std::endl;
    std::cerr.flush();
    // Check for decorators - they come before def/class
    // Look ahead to see if @ is followed by def or class
    std::cerr << "[DEBUG parse_stmt] Checking AT: current().type=" << static_cast<int>(current().type)
              << ", TokenType::AT=" << static_cast<int>(TokenType::AT) << std::endl;
    std::cerr.flush();
    if (current().type == TokenType::AT) {
        std::cerr << "[DEBUG parse_stmt] Found AT token!" << std::endl;
        std::cerr.flush();
        // Peek ahead to find the next non-decorator token
        size_t lookahead = current_token_;
        while (lookahead < tokens_.size() && tokens_[lookahead].type == TokenType::AT) {
            lookahead++;  // skip @
            // Skip the decorator expression (everything until NEWLINE, DEF, or CLASS)
            while (lookahead < tokens_.size() &&
                   tokens_[lookahead].type != TokenType::NEWLINE &&
                   tokens_[lookahead].type != TokenType::DEF &&
                   tokens_[lookahead].type != TokenType::CLASS &&
                   tokens_[lookahead].type != TokenType::AT) {
                lookahead++;
            }
            // Skip NEWLINE if present (decorators are typically on separate lines)
            if (lookahead < tokens_.size() && tokens_[lookahead].type == TokenType::NEWLINE) {
                lookahead++;
            }
        }
        // Check what comes after decorators
        std::cerr << "[DEBUG parse_stmt] After lookahead, lookahead=" << lookahead
                  << ", token type=" << (lookahead < tokens_.size() ? static_cast<int>(tokens_[lookahead].type) : -1)
                  << ", value='" << (lookahead < tokens_.size() ? tokens_[lookahead].value : "") << "'" << std::endl;
        std::cerr.flush();
        if (lookahead < tokens_.size() && tokens_[lookahead].type == TokenType::DEF) {
            std::cerr << "[DEBUG parse_stmt] Calling parse_function_def()" << std::endl;
            std::cerr.flush();
            return parse_function_def();
        } else if (lookahead < tokens_.size() && tokens_[lookahead].type == TokenType::CLASS) {
            std::cerr << "[DEBUG parse_stmt] Calling parse_class_def()" << std::endl;
            std::cerr.flush();
            return parse_class_def();
        } else {
            std::cerr << "[DEBUG parse_stmt] ERROR: Decorators not followed by def/class" << std::endl;
            std::cerr.flush();
            error("Decorators can only be applied to functions or classes");
        }
    }
    // Handle async keyword (Python 3.5+)
    if (current().type == TokenType::ASYNC) {
        advance(); // consume 'async'
        if (current().type == TokenType::DEF) {
            return parse_async_function_def();
        } else if (current().type == TokenType::FOR) {
            return parse_async_for();
        } else if (current().type == TokenType::WITH) {
            return parse_async_with();
        } else {
            error("'async' must be followed by 'def', 'for', or 'with'");
        }
    }
    if (current().type == TokenType::DEF) {
        return parse_function_def();
    } else if (current().type == TokenType::CLASS) {
        return parse_class_def();
    } else if (current().type == TokenType::RETURN) {
        return parse_return();
    } else if (current().type == TokenType::IF) {
        return parse_if();
    } else if (current().type == TokenType::WHILE) {
        return parse_while();
    } else if (current().type == TokenType::FOR) {
        return parse_for();
    } else if (current().type == TokenType::TRY) {
        return parse_try_stmt();
    } else if (current().type == TokenType::BREAK) {
        return parse_break();
    } else if (current().type == TokenType::CONTINUE) {
        return parse_continue();
    } else if (current().type == TokenType::PASS) {
        return parse_pass_stmt();
    } else if (current().type == TokenType::RAISE) {
        return parse_raise_stmt();
    } else if (current().type == TokenType::DEL) {
        return parse_del_stmt();
    } else if (current().type == TokenType::ASSERT) {
        return parse_assert_stmt();
    } else if (current().type == TokenType::GLOBAL) {
        return parse_global_stmt();
    } else if (current().type == TokenType::NONLOCAL) {
        return parse_nonlocal_stmt();
    } else if (current().type == TokenType::IMPORT || current().type == TokenType::FROM) {
        return parse_import_stmt();
    } else if (current().type == TokenType::WITH) {
        return parse_with_stmt();
    } else if (current().type == TokenType::ELIF) {
        // elif can only appear after if/elif, not at module level
        error("'elif' can only appear after 'if' or 'elif'");
        return nullptr; // Never reached, but satisfies compiler
    } else {
        // Try to parse as expression or assignment
        std::cerr << "[DEBUG parse_stmt] Falling through to expression/assignment, token=" << current_token_
                  << ", type=" << static_cast<int>(current().type) << std::endl;
        std::cerr.flush();
        auto expr = parse_expr();
        if (is_augmented_assign()) {
            // This is an augmented assignment (+=, -=, etc.)
            TokenType op_token = current().type;
            ast::Operator op = token_to_operator(op_token);
            advance(); // consume the augmented assignment operator
            auto value = parse_expr();
            return std::make_shared<ast::AugAssign>(expr, op, value,
                                                    expr->lineno(), expr->col_offset());
        } else if (match(TokenType::EQUAL)) {
            // This is a regular assignment
            auto value = parse_expr();
            std::vector<std::shared_ptr<ast::Expr>> targets = {expr};
            return std::make_shared<ast::Assign>(targets, value,
                                                expr->lineno(), expr->col_offset());
        } else {
            // Expression statement
            return std::make_shared<ast::ExprStmt>(expr,
                                                  expr->lineno(), expr->col_offset());
        }
    }
}

inline std::shared_ptr<ast::Stmt> Parser::parse_function_def() {
    std::cerr << "[DEBUG parse_function_def] Entry, token=" << current_token_
              << ", type=" << static_cast<int>(current().type)
              << ", value='" << current().value << "'" << std::endl;
    std::cerr.flush();
    // Check for decorators first
    std::vector<std::shared_ptr<ast::Expr>> decorators;
    if (current().type == TokenType::AT) {
        std::cerr << "[DEBUG parse_function_def] Found decorators, parsing..." << std::endl;
        std::cerr.flush();
        decorators = parse_decorators();
        std::cerr << "[DEBUG parse_function_def] Parsed " << decorators.size() << " decorators" << std::endl;
        std::cerr.flush();
    }
    auto func_def = parse_function_def_raw();
    // Apply decorators if any
    if (auto func = std::dynamic_pointer_cast<ast::FunctionDef>(func_def)) {
        return std::make_shared<ast::FunctionDef>(
            func->name(), func->args(), func->body(), decorators,
            func->lineno(), func->col_offset());
    }
    return func_def;
}

inline std::shared_ptr<ast::Stmt> Parser::parse_function_def_raw() {
    if (!match(TokenType::DEF)) {
        error("Expected 'def'");
    }

    Token name_token = current();
    if (name_token.type != TokenType::IDENTIFIER) {
        error("Expected function name");
    }
    std::string name = name_token.value;
    advance();

    if (!match(TokenType::LPAREN)) {
        error("Expected '(' after function name");
    }

    std::vector<std::string> args = parse_arg_list();

    if (!match(TokenType::RPAREN)) {
        error("Expected ')' after arguments");
    }

    if (!match(TokenType::COLON)) {
        error("Expected ':' after function signature");
    }

    // Parse function body (simplified - just parse statements until dedent)
    std::vector<std::shared_ptr<ast::Stmt>> body;
    while (!is_at_end() && current().type != TokenType::END_OF_FILE) {
        if (current().type == TokenType::NEWLINE) {
            advance();
            continue;
        }
        // Simple heuristic: stop at next def/class at same indentation level
        // In a real parser, we'd track indentation
        if (current().type == TokenType::DEF && peek().type == TokenType::IDENTIFIER) {
            break;
        }
        auto stmt = parse_stmt();
        if (stmt) {
            body.push_back(stmt);
        }
    }

    return std::make_shared<ast::FunctionDef>(name, args, body, std::vector<std::shared_ptr<ast::Expr>>(),
                                             name_token.line, name_token.column);
}

inline std::shared_ptr<ast::Stmt> Parser::parse_return() {
    Token return_token = current();
    if (!match(TokenType::RETURN)) {
        error("Expected 'return'");
    }

    std::shared_ptr<ast::Expr> value = nullptr;
    if (current().type != TokenType::NEWLINE && current().type != TokenType::END_OF_FILE) {
        value = parse_expr();
    }

    return std::make_shared<ast::Return>(value, return_token.line, return_token.column);
}

inline std::vector<std::shared_ptr<ast::Stmt>> Parser::parse_else_block() {
    std::vector<std::shared_ptr<ast::Stmt>> orelse;
    if (match(TokenType::ELSE)) {
        if (!match(TokenType::COLON)) {
            error("Expected ':' after else");
        }
        while (!is_at_end() && current().type != TokenType::END_OF_FILE) {
            if (current().type == TokenType::NEWLINE) {
                advance();
                continue;
            }
            // Stop at def, class, import, or finally (for try statements)
            if (current().type == TokenType::DEF || current().type == TokenType::CLASS ||
                current().type == TokenType::IMPORT || current().type == TokenType::FROM ||
                current().type == TokenType::FINALLY) {
                break;
            }
            auto stmt = parse_stmt();
            if (stmt) {
                orelse.push_back(stmt);
            }
        }
    }
    return orelse;
}

inline std::shared_ptr<ast::Stmt> Parser::parse_if() {
    Token if_token = current();
    if (!match(TokenType::IF)) {
        error("Expected 'if'");
    }

    auto test = parse_expr();

    if (!match(TokenType::COLON)) {
        error("Expected ':' after if condition");
    }

    // Parse block (body) - matches CPython's block_rule
    std::vector<std::shared_ptr<ast::Stmt>> body;
    while (!is_at_end() && current().type != TokenType::END_OF_FILE) {
        if (current().type == TokenType::NEWLINE) {
            advance();
            continue;
        }
        if (current().type == TokenType::ELSE || current().type == TokenType::ELIF || current().type == TokenType::DEF) {
            break;
        }
        auto stmt = parse_stmt();
        if (stmt) {
            body.push_back(stmt);
        }
    }

    // Try elif_stmt first (matches CPython: 'if' named_expression ':' block elif_stmt)
    // CPython tries this alternative first, and only if it fails, tries else_block
    // Save position before trying (like CPython's _mark)
    size_t saved_pos = current_token_;
    std::vector<std::shared_ptr<ast::Stmt>> orelse;
    auto elif_stmt = parse_elif_stmt();
    if (elif_stmt) {
        // elif_stmt returns an If node, wrap it in orelse (like _PyPegen_singleton_seq)
        orelse.push_back(elif_stmt);
    } else {
        // Restore position and try else_block (matches CPython: 'if' named_expression ':' block else_block?)
        current_token_ = saved_pos;
        orelse = parse_else_block();
    }

    return std::make_shared<ast::If>(test, body, orelse, if_token.line, if_token.column);
}

inline std::shared_ptr<ast::Stmt> Parser::parse_elif_stmt() {
    // Save position to restore if parsing fails (matches CPython's _mark)
    size_t saved_pos = current_token_;

    // Try first alternative: 'elif' named_expression ':' block elif_stmt
    if (!match(TokenType::ELIF)) {
        current_token_ = saved_pos; // Restore position
        return nullptr; // Not an elif
    }

    auto test = parse_expr();
    if (!test) {
        current_token_ = saved_pos; // Restore position on error
        return nullptr;
    }

    if (!match(TokenType::COLON)) {
        current_token_ = saved_pos; // Restore position on error
        return nullptr;
    }

    // Parse block (body) - matches CPython's block_rule
    std::vector<std::shared_ptr<ast::Stmt>> body;
    while (!is_at_end() && current().type != TokenType::END_OF_FILE) {
        if (current().type == TokenType::NEWLINE) {
            advance();
            continue;
        }
        if (current().type == TokenType::ELSE || current().type == TokenType::ELIF || current().type == TokenType::DEF) {
            break;
        }
        auto stmt = parse_stmt();
        if (stmt) {
            body.push_back(stmt);
        }
    }

    Token elif_token = tokens_[saved_pos]; // Use saved token for line/column

    // Try elif_stmt first (recursive - matches CPython: 'elif' named_expression ':' block elif_stmt)
    size_t saved_pos2 = current_token_; // Save position before trying nested elif
    auto nested_elif = parse_elif_stmt();
    std::vector<std::shared_ptr<ast::Stmt>> orelse;
    if (nested_elif) {
        // Recursively parsed nested elif_stmt, wrap it in orelse (like _PyPegen_singleton_seq)
        orelse.push_back(nested_elif);
    } else {
        // Restore position and try else_block (matches CPython: 'elif' named_expression ':' block else_block?)
        current_token_ = saved_pos2;
        orelse = parse_else_block();
    }

    // Create If node for this elif (matches CPython's _PyAST_If)
    return std::make_shared<ast::If>(test, body, orelse, elif_token.line, elif_token.column);
}

inline std::shared_ptr<ast::Stmt> Parser::parse_while() {
    Token while_token = current();
    if (!match(TokenType::WHILE)) {
        error("Expected 'while'");
    }

    auto test = parse_expr();

    if (!match(TokenType::COLON)) {
        error("Expected ':' after while condition");
    }

    // Parse block (body)
    std::vector<std::shared_ptr<ast::Stmt>> body;
    while (!is_at_end() && current().type != TokenType::END_OF_FILE) {
        if (current().type == TokenType::NEWLINE) {
            advance();
            continue;
        }
        if (current().type == TokenType::ELSE || current().type == TokenType::DEF) {
            break;
        }
        auto stmt = parse_stmt();
        if (stmt) {
            body.push_back(stmt);
        }
    }

    // Parse else clause if present
    std::vector<std::shared_ptr<ast::Stmt>> orelse = parse_else_block();

    return std::make_shared<ast::While>(test, body, orelse, while_token.line, while_token.column);
}

inline std::shared_ptr<ast::Stmt> Parser::parse_for() {
    Token for_token = current();
    if (!match(TokenType::FOR)) {
        error("Expected 'for'");
    }

    // Parse target using star_targets (matches CPython: 'for' star_targets 'in' ~ star_expressions)
    // star_targets doesn't parse 'in' as a comparison operator
    auto target = parse_star_targets();

    if (!match(TokenType::IN)) {
        error("Expected 'in' in for loop");
    }

    // Parse iterable using star_expressions (matches CPython)
    auto iter = parse_star_expressions();

    if (!match(TokenType::COLON)) {
        error("Expected ':' after for loop");
    }

    // Parse block (body)
    std::vector<std::shared_ptr<ast::Stmt>> body;
    while (!is_at_end() && current().type != TokenType::END_OF_FILE) {
        if (current().type == TokenType::NEWLINE) {
            advance();
            continue;
        }
        if (current().type == TokenType::ELSE || current().type == TokenType::DEF) {
            break;
        }
        auto stmt = parse_stmt();
        if (stmt) {
            body.push_back(stmt);
        }
    }

    // Parse else clause if present
    std::vector<std::shared_ptr<ast::Stmt>> orelse = parse_else_block();

    return std::make_shared<ast::For>(target, iter, body, orelse, for_token.line, for_token.column);
}

// Parse async function definition (Python 3.5+)
inline std::shared_ptr<ast::Stmt> Parser::parse_async_function_def() {
    Token async_token = tokens_[current_token_ - 1]; // 'async' was already consumed
    if (!match(TokenType::DEF)) {
        error("Expected 'def' after 'async'");
    }

    Token name_token = current();
    if (name_token.type != TokenType::IDENTIFIER) {
        error("Expected function name");
    }
    std::string name = name_token.value;
    advance();

    if (!match(TokenType::LPAREN)) {
        error("Expected '(' after function name");
    }

    std::vector<std::string> args = parse_arg_list();

    if (!match(TokenType::RPAREN)) {
        error("Expected ')' after arguments");
    }

    if (!match(TokenType::COLON)) {
        error("Expected ':' after function signature");
    }

    // Parse function body
    std::vector<std::shared_ptr<ast::Stmt>> body;
    while (!is_at_end() && current().type != TokenType::END_OF_FILE) {
        if (current().type == TokenType::NEWLINE) {
            advance();
            continue;
        }
        if (current().type == TokenType::DEF && peek().type == TokenType::IDENTIFIER) {
            break;
        }
        auto stmt = parse_stmt();
        if (stmt) {
            body.push_back(stmt);
        }
    }

    return std::make_shared<ast::AsyncFunctionDef>(name, args, body, std::vector<std::shared_ptr<ast::Expr>>(),
                                                    async_token.line, async_token.column);
}

// Parse async for loop (Python 3.5+)
inline std::shared_ptr<ast::Stmt> Parser::parse_async_for() {
    Token async_token = tokens_[current_token_ - 1]; // 'async' was already consumed
    if (!match(TokenType::FOR)) {
        error("Expected 'for' after 'async'");
    }

    auto target = parse_star_targets();

    if (!match(TokenType::IN)) {
        error("Expected 'in' in async for loop");
    }

    auto iter = parse_star_expressions();

    if (!match(TokenType::COLON)) {
        error("Expected ':' after async for loop");
    }

    // Parse block (body)
    std::vector<std::shared_ptr<ast::Stmt>> body;
    while (!is_at_end() && current().type != TokenType::END_OF_FILE) {
        if (current().type == TokenType::NEWLINE) {
            advance();
            continue;
        }
        if (current().type == TokenType::ELSE || current().type == TokenType::DEF) {
            break;
        }
        auto stmt = parse_stmt();
        if (stmt) {
            body.push_back(stmt);
        }
    }

    // Parse else clause if present
    std::vector<std::shared_ptr<ast::Stmt>> orelse = parse_else_block();

    return std::make_shared<ast::AsyncFor>(target, iter, body, orelse, async_token.line, async_token.column);
}

inline std::shared_ptr<ast::Stmt> Parser::parse_break() {
    Token break_token = current();
    if (!match(TokenType::BREAK)) {
        error("Expected 'break'");
    }
    return std::make_shared<ast::Break>(break_token.line, break_token.column);
}

inline std::shared_ptr<ast::Stmt> Parser::parse_continue() {
    Token continue_token = current();
    if (!match(TokenType::CONTINUE)) {
        error("Expected 'continue'");
    }
    return std::make_shared<ast::Continue>(continue_token.line, continue_token.column);
}

inline std::shared_ptr<ast::Stmt> Parser::parse_assign() {
    auto target = parse_expr();
    if (!match(TokenType::EQUAL)) {
        error("Expected '=' in assignment");
    }
    auto value = parse_expr();
    std::vector<std::shared_ptr<ast::Expr>> targets = {target};
    return std::make_shared<ast::Assign>(targets, value,
                                       target->lineno(), target->col_offset());
}

inline std::shared_ptr<ast::Stmt> Parser::parse_expr_stmt() {
    auto expr = parse_expr();
    return std::make_shared<ast::ExprStmt>(expr, expr->lineno(), expr->col_offset());
}

// Expression parsing with proper precedence (lowest to highest)
// Reference: Python grammar precedence
inline std::shared_ptr<ast::Expr> Parser::parse_expr() {
    std::cerr << "[DEBUG parse_expr] Entry, token=" << current_token_
              << ", type=" << static_cast<int>(current().type)
              << ", value='" << current().value << "'" << std::endl;
    std::cerr.flush();
    auto result = parse_disjunction();
    std::cerr << "[DEBUG parse_expr] Exit, token=" << current_token_
              << ", type=" << static_cast<int>(current().type) << std::endl;
    std::cerr.flush();
    return result;
}

// or (lowest precedence)
// Also handles conditional expressions: disjunction 'if' disjunction 'else' conditional
inline std::shared_ptr<ast::Expr> Parser::parse_disjunction() {
    auto left = parse_conjunction();
    std::vector<std::shared_ptr<ast::Expr>> values = {left};

    while (match(TokenType::OR)) {
        values.push_back(parse_conjunction());
    }

    std::shared_ptr<ast::Expr> result;
    if (values.size() == 1) {
        result = values[0];
    } else {
        Token token = tokens_[current_token_ - 1];
        result = std::make_shared<ast::BoolOpExpr>(ast::BoolOp::Or, values, token.line, token.column);
    }

    // Check for conditional expression: result 'if' test 'else' orelse
    // Matches CPython's conditional rule: disjunction 'if' disjunction 'else' conditional
    // Only parse as conditional if we see 'if' followed by an expression and then 'else'
    // Use lookahead to avoid conflicts with comprehensions (which have 'for' after 'if')
    if (current().type == TokenType::IF) {
        // Look ahead to see if this is a conditional expression or part of a comprehension
        // In comprehensions, we'd see: expr 'if' expr 'for' ...
        // In conditionals, we'd see: expr 'if' expr 'else' ...
        size_t saved_pos = current_token_;
        advance();  // consume 'if'
        auto test = parse_disjunction();  // Parse test (another disjunction)

        // Check if next token is 'else' (conditional) or 'for' (comprehension)
        if (current().type == TokenType::ELSE) {
            // This is a conditional expression
            advance();  // consume 'else'
            auto orelse = parse_conditional();  // Parse orelse (recursive, right-associative)
            Token if_token = tokens_[saved_pos];  // Get the 'if' token
            return std::make_shared<ast::IfExp>(test, result, orelse, if_token.line, if_token.column);
        } else {
            // This is not a conditional expression (probably part of a comprehension)
            // Restore position and return the result without conditional
            current_token_ = saved_pos;
        }
    }

    return result;
}

// Conditional expression (right-associative)
// conditional: disjunction 'if' disjunction 'else' conditional
inline std::shared_ptr<ast::Expr> Parser::parse_conditional() {
    return parse_disjunction();  // Delegate to parse_disjunction which handles conditionals
}

// and
inline std::shared_ptr<ast::Expr> Parser::parse_conjunction() {
    auto left = parse_inversion();
    std::vector<std::shared_ptr<ast::Expr>> values = {left};

    while (match(TokenType::AND)) {
        values.push_back(parse_inversion());
    }

    if (values.size() == 1) {
        return values[0];
    }

    Token token = tokens_[current_token_ - 1];
    return std::make_shared<ast::BoolOpExpr>(ast::BoolOp::And, values, token.line, token.column);
}

// not
inline std::shared_ptr<ast::Expr> Parser::parse_inversion() {
    if (match(TokenType::NOT)) {
        Token token = tokens_[current_token_ - 1];
        auto operand = parse_inversion();
        return std::make_shared<ast::UnaryOp>(ast::UnaryOp::UnaryOpType::Not, operand, token.line, token.column);
    }
    return parse_comparison();
}

// <, >, ==, !=, <=, >=, is, is not, in, not in
// Note: Python supports chained comparisons like "0 < x < 10"
// For now, we only parse single comparisons
inline std::shared_ptr<ast::Expr> Parser::parse_comparison() {
    auto left = parse_bitwise_or();

    // Try to parse a comparison operator
    if (match(TokenType::LESS)) {
        Token token = tokens_[current_token_ - 1];
        auto right = parse_bitwise_or();
        return std::make_shared<ast::Compare>(left, ast::CompareOp::Lt, right, token.line, token.column);
    } else if (match(TokenType::GREATER)) {
        Token token = tokens_[current_token_ - 1];
        auto right = parse_bitwise_or();
        return std::make_shared<ast::Compare>(left, ast::CompareOp::Gt, right, token.line, token.column);
    } else if (match(TokenType::LESS_EQUAL)) {
        Token token = tokens_[current_token_ - 1];
        auto right = parse_bitwise_or();
        return std::make_shared<ast::Compare>(left, ast::CompareOp::LtE, right, token.line, token.column);
    } else if (match(TokenType::GREATER_EQUAL)) {
        Token token = tokens_[current_token_ - 1];
        auto right = parse_bitwise_or();
        return std::make_shared<ast::Compare>(left, ast::CompareOp::GtE, right, token.line, token.column);
    } else if (match(TokenType::EQUAL_EQUAL)) {
        Token token = tokens_[current_token_ - 1];
        auto right = parse_bitwise_or();
        return std::make_shared<ast::Compare>(left, ast::CompareOp::Eq, right, token.line, token.column);
    } else if (match(TokenType::NOT_EQUAL)) {
        Token token = tokens_[current_token_ - 1];
        auto right = parse_bitwise_or();
        return std::make_shared<ast::Compare>(left, ast::CompareOp::NotEq, right, token.line, token.column);
    } else if (match(TokenType::IS)) {
        Token token = tokens_[current_token_ - 1];
        if (match(TokenType::NOT)) {
            auto right = parse_bitwise_or();
            return std::make_shared<ast::Compare>(left, ast::CompareOp::IsNot, right, token.line, token.column);
        }
        auto right = parse_bitwise_or();
        return std::make_shared<ast::Compare>(left, ast::CompareOp::Is, right, token.line, token.column);
    } else if (match(TokenType::IN)) {
        Token token = tokens_[current_token_ - 1];
        auto right = parse_bitwise_or();
        return std::make_shared<ast::Compare>(left, ast::CompareOp::In, right, token.line, token.column);
    } else if (match(TokenType::NOT) && current().type == TokenType::IN) {
        Token token = tokens_[current_token_ - 1];
        advance(); // consume IN
        auto right = parse_bitwise_or();
        return std::make_shared<ast::Compare>(left, ast::CompareOp::NotIn, right, token.line, token.column);
    }

    return left;
}

// |
inline std::shared_ptr<ast::Expr> Parser::parse_bitwise_or() {
    auto left = parse_bitwise_xor();

    while (match(TokenType::BIT_OR)) {
        Token token = tokens_[current_token_ - 1];
        auto right = parse_bitwise_xor();
        left = std::make_shared<ast::BinOp>(left, ast::Operator::BitOr, right, token.line, token.column);
    }

    return left;
}

// ^
inline std::shared_ptr<ast::Expr> Parser::parse_bitwise_xor() {
    auto left = parse_bitwise_and();

    while (match(TokenType::BIT_XOR)) {
        Token token = tokens_[current_token_ - 1];
        auto right = parse_bitwise_and();
        left = std::make_shared<ast::BinOp>(left, ast::Operator::BitXor, right, token.line, token.column);
    }

    return left;
}

// &
inline std::shared_ptr<ast::Expr> Parser::parse_bitwise_and() {
    auto left = parse_shift_expr();

    while (match(TokenType::BIT_AND)) {
        Token token = tokens_[current_token_ - 1];
        auto right = parse_shift_expr();
        left = std::make_shared<ast::BinOp>(left, ast::Operator::BitAnd, right, token.line, token.column);
    }

    return left;
}

// <<, >>
inline std::shared_ptr<ast::Expr> Parser::parse_shift_expr() {
    auto left = parse_sum();

    while (true) {
        Token token = current();
        ast::Operator op;
        if (match(TokenType::LEFT_SHIFT)) {
            op = ast::Operator::LShift;
        } else if (match(TokenType::RIGHT_SHIFT)) {
            op = ast::Operator::RShift;
        } else {
            break;
        }
        auto right = parse_sum();
        left = std::make_shared<ast::BinOp>(left, op, right, token.line, token.column);
    }

    return left;
}

// +, -
inline std::shared_ptr<ast::Expr> Parser::parse_sum() {
    auto left = parse_term();

    while (true) {
        Token token = current();
        ast::Operator op;
        if (match(TokenType::PLUS)) {
            op = ast::Operator::Add;
        } else if (match(TokenType::MINUS)) {
            op = ast::Operator::Sub;
        } else {
            break;
        }
        auto right = parse_term();
        left = std::make_shared<ast::BinOp>(left, op, right, token.line, token.column);
    }

    return left;
}

// *, /, %, //
inline std::shared_ptr<ast::Expr> Parser::parse_term() {
    auto left = parse_factor();

    while (true) {
        Token token = current();
        ast::Operator op;
        if (match(TokenType::STAR)) {
            op = ast::Operator::Mult;
        } else if (match(TokenType::SLASH)) {
            op = ast::Operator::Div;
        } else if (match(TokenType::PERCENT)) {
            op = ast::Operator::Mod;
        } else if (match(TokenType::FLOOR_DIV)) {
            op = ast::Operator::Div; // TODO: Add FloorDiv operator
        } else {
            break;
        }
        auto right = parse_factor();
        left = std::make_shared<ast::BinOp>(left, op, right, token.line, token.column);
    }

    return left;
}

// **, unary +, -, ~
inline std::shared_ptr<ast::Expr> Parser::parse_factor() {
    Token token = current();

    // Unary operators
    if (match(TokenType::PLUS)) {
        auto operand = parse_factor();
        return std::make_shared<ast::UnaryOp>(ast::UnaryOp::UnaryOpType::UAdd, operand, token.line, token.column);
    } else if (match(TokenType::MINUS)) {
        auto operand = parse_factor();
        return std::make_shared<ast::UnaryOp>(ast::UnaryOp::UnaryOpType::USub, operand, token.line, token.column);
    } else if (match(TokenType::BIT_NOT)) {
        auto operand = parse_factor();
        return std::make_shared<ast::UnaryOp>(ast::UnaryOp::UnaryOpType::Invert, operand, token.line, token.column);
    }

    // Power operator (right-associative)
    auto left = parse_primary();

    if (match(TokenType::POWER)) {
        Token pow_token = tokens_[current_token_ - 1];
        auto right = parse_factor(); // Right-associative
        return std::make_shared<ast::BinOp>(left, ast::Operator::Pow, right, pow_token.line, pow_token.column);
    }

    return left;
}

// Parse atom (lowest level of primary)
inline std::shared_ptr<ast::Expr> Parser::parse_atom() {
    Token token = current();

    if (match(TokenType::NUMBER)) {
        return std::make_shared<ast::Constant>(token.value, token.line, token.column);
    } else if (match(TokenType::STRING)) {
        return std::make_shared<ast::Constant>(token.value, token.line, token.column);
    } else if (current().type == TokenType::FSTRING_START) {
        // F-string - don't use match() because parse_fstring() expects to see FSTRING_START
        return parse_fstring();
    } else if (match(TokenType::TRUE)) {
        return std::make_shared<ast::Constant>("True", token.line, token.column);
    } else if (match(TokenType::FALSE)) {
        return std::make_shared<ast::Constant>("False", token.line, token.column);
    } else if (match(TokenType::NONE)) {
        return std::make_shared<ast::Constant>("None", token.line, token.column);
    } else if (match(TokenType::LBRACKET)) {
        std::cerr << "[DEBUG parse_atom] Matched LBRACKET, calling parse_list(), token=" << current_token_
                  << ", type=" << static_cast<int>(current().type) << std::endl;
        std::cerr.flush();
        return parse_list();
    } else if (match(TokenType::LBRACE)) {
        return parse_dict();
    } else if (match(TokenType::LAMBDA)) {
        // Lambda expression
        return parse_lambda();
    } else if (match(TokenType::YIELD)) {
        // Yield expression
        return parse_yield_expr();
    } else if (match(TokenType::AWAIT)) {
        // Await expression (Python 3.5+)
        auto value = parse_primary();
        return std::make_shared<ast::Await>(value, token.line, token.column);
    } else if (current().type == TokenType::ELLIPSIS) {
        // Ellipsis expression - token already matched, just create the node
        Token ellipsis_token = current();
        advance();  // consume ELLIPSIS token
        return std::make_shared<ast::Ellipsis>(ellipsis_token.line, ellipsis_token.column);
    } else if (match(TokenType::IDENTIFIER)) {
        // Create name, but don't handle call here - let parse_primary handle it
        return std::make_shared<ast::Name>(token.value, ast::ExprContext::Load,
                                           token.line, token.column);
    } else if (match(TokenType::LPAREN)) {
        // Check if it's a tuple, generator expression, or parenthesized expression
        if (current().type == TokenType::RPAREN) {
            // Empty tuple
            advance();
            return std::make_shared<ast::Tuple>(std::vector<std::shared_ptr<ast::Expr>>(),
                                                ast::ExprContext::Load, token.line, token.column);
        }

        auto expr = parse_expr();

        // Check if it's a generator expression: (expr for ...) or (expr async for ...)
        if (current().type == TokenType::FOR || current().type == TokenType::ASYNC) {
            auto generators = parse_for_if_clauses();
            if (!match(TokenType::RPAREN)) {
                error("Expected ')' after generator expression");
            }
            return std::make_shared<ast::GeneratorExp>(expr, generators, token.line, token.column);
        }

        // Check if it's a tuple (comma after expression)
        if (match(TokenType::COMMA)) {
            std::vector<std::shared_ptr<ast::Expr>> elts = {expr};
            while (current().type != TokenType::RPAREN) {
                elts.push_back(parse_expr());
                if (!match(TokenType::COMMA)) {
                    break;
                }
            }
            if (!match(TokenType::RPAREN)) {
                error("Expected ')'");
            }
            return std::make_shared<ast::Tuple>(elts, ast::ExprContext::Load, token.line, token.column);
        }

        if (!match(TokenType::RPAREN)) {
            error("Expected ')'");
        }
        return expr;
    }

    error("Unexpected token in expression");
    return nullptr;
}

// Parse f-string: FSTRING_START (FSTRING_MIDDLE | formatted_value)* FSTRING_END
inline std::shared_ptr<ast::Expr> Parser::parse_fstring() {
    Token start_token = current();
    if (start_token.type != TokenType::FSTRING_START) {
        error("Expected FSTRING_START");
        return nullptr;
    }
    advance();  // consume FSTRING_START

    std::vector<std::shared_ptr<ast::Expr>> values;

    // Add initial string part if present
    if (!start_token.value.empty()) {
        values.push_back(std::make_shared<ast::Constant>(
            start_token.value, start_token.line, start_token.column));
    }

    // Parse alternating FSTRING_MIDDLE and formatted_value until FSTRING_END
    while (current().type != TokenType::FSTRING_END) {
        if (current().type == TokenType::FSTRING_MIDDLE) {
            // Literal string part
            Token middle = current();
            advance();
            if (!middle.value.empty()) {
                values.push_back(std::make_shared<ast::Constant>(
                    middle.value, middle.line, middle.column));
            }
        } else if (current().type == TokenType::LBRACE) {
            // Expression part: {expr!conversion:format_spec}
            values.push_back(parse_formatted_value());
        } else {
            error("Expected FSTRING_MIDDLE or '{' in f-string");
            return nullptr;
        }
    }

    // Parse FSTRING_END
    Token end_token = current();
    advance();  // consume FSTRING_END

    // If we only have one constant value, return it directly (optimization)
    // Otherwise, return JoinedStr
    if (values.size() == 1 &&
        std::dynamic_pointer_cast<ast::Constant>(values[0])) {
        return values[0];
    }

    return std::make_shared<ast::JoinedStr>(values, start_token.line, start_token.column);
}

// Parse formatted value: {expr [!conversion] [:format_spec]}
inline std::shared_ptr<ast::Expr> Parser::parse_formatted_value() {
    // Parse '{'
    Token lbrace = current();
    if (lbrace.type != TokenType::LBRACE) {
        error("Expected '{' in formatted value");
        return nullptr;
    }
    advance();

    // Parse expression (can be any expression, including nested f-strings)
    // Now that we use EXCLAIM token for !, we can safely use parse_expr()
    // which supports full Python expressions including comparisons, and, or, not
    std::shared_ptr<ast::Expr> value = parse_expr();

    // Parse optional conversion: !s, !r, !a
    int conversion = -1;
    if (match(TokenType::EXCLAIM)) {
        if (current().type == TokenType::IDENTIFIER) {
            std::string conv_char = current().value;
            if (conv_char == "s") {
                conversion = 115;
            } else if (conv_char == "r") {
                conversion = 114;
            } else if (conv_char == "a") {
                conversion = 97;
            } else {
                error("Invalid conversion specifier: !" + conv_char);
                return nullptr;
            }
            advance();
        } else {
            error("Expected conversion specifier (s, r, or a) after '!'");
            return nullptr;
        }
    }

    // Parse optional format specifier: :format_spec
    std::shared_ptr<ast::Expr> format_spec = nullptr;
    if (match(TokenType::COLON)) {
        format_spec = parse_fstring_format_spec();
    }

    // Parse '}'
    if (!match(TokenType::RBRACE)) {
        error("Expected '}' in f-string expression");
        return nullptr;
    }

    return std::make_shared<ast::FormattedValue>(
        value, conversion, format_spec, lbrace.line, lbrace.column);
}

// Parse format specifier: can contain FSTRING_MIDDLE tokens and nested formatted_value
inline std::shared_ptr<ast::Expr> Parser::parse_fstring_format_spec() {
    std::vector<std::shared_ptr<ast::Expr>> values;

    // Parse format spec parts until we see } (end of formatted value)
    // We need to track when we're done - the } will be consumed by parse_formatted_value
    while (current().type != TokenType::RBRACE) {
        if (current().type == TokenType::FSTRING_MIDDLE) {
            // Literal format text (e.g., ".2f", ">10")
            Token middle = current();
            advance();
            if (!middle.value.empty()) {
                values.push_back(std::make_shared<ast::Constant>(
                    middle.value, middle.line, middle.column));
            }
        } else if (current().type == TokenType::LBRACE) {
            // Nested f-string replacement field in format spec
            values.push_back(parse_formatted_value());
        } else {
            // End of format spec (should be })
            break;
        }
    }

    // If we only have one constant value, return it directly
    if (values.size() == 1 &&
        std::dynamic_pointer_cast<ast::Constant>(values[0])) {
        return values[0];
    }

    // If empty, return nullptr (no format spec)
    if (values.empty()) {
        return nullptr;
    }

    // Otherwise return JoinedStr
    return std::make_shared<ast::JoinedStr>(values, current().line, current().column);
}

// Parse primary expression (handles left-recursive attribute, subscript, call)
// primary: atom | primary '.' NAME | primary '[' expr ']' | primary '(' arguments ')'
inline std::shared_ptr<ast::Expr> Parser::parse_primary() {
    // Start with atom
    auto expr = parse_atom();

    // Handle left-recursive constructs: .attr, [index], (args)
    while (true) {
        if (match(TokenType::DOT)) {
            // Attribute access: obj.attr
            if (current().type != TokenType::IDENTIFIER) {
                error("Expected attribute name after '.'");
            }
            Token attr_token = current();
            advance();
            expr = std::make_shared<ast::Attribute>(expr, attr_token.value, ast::ExprContext::Load,
                                                    attr_token.line, attr_token.column);
        } else if (match(TokenType::LBRACKET)) {
            // Subscript: obj[key] or obj[start:end:step]
            // Matches CPython: primary '[' slices ']'
            // slices_rule tries: slice !',' OR ','.(slice | starred_expression)+
            // For now, handle single slice or single expression (matches slice_rule logic)
            std::shared_ptr<ast::Expr> slice;

            std::cerr << "[DEBUG parse_primary] MATCHED LBRACKET! After matching '[', token=" << current_token_
                      << ", type=" << static_cast<int>(current().type)
                      << ", value='" << current().value << "'" << std::endl;
            std::cerr.flush();

            // Try to parse as slice first (matches CPython: slice_rule tries slice pattern first)
            // slice_rule tries: expression? ':' expression? [':' expression?] OR named_expression
            size_t saved_pos = current_token_;
            slice = parse_slice();

            std::cerr << "[DEBUG parse_primary] After parse_slice(), token=" << current_token_
                      << ", type=" << static_cast<int>(current().type)
                      << ", value='" << current().value << "'"
                      << ", slice=" << (slice ? "valid" : "nullptr") << std::endl;
            std::cerr.flush();

            // If slice parsing failed (returned nullptr), try as regular expression
            // This matches CPython's fallback to named_expression in slice_rule
            if (!slice) {
                std::cerr << "[DEBUG parse_primary] Slice parsing failed, trying as regular expression" << std::endl;
                current_token_ = saved_pos;  // Reset position
                // Parse as regular expression (named_expression in CPython)
                if (current().type == TokenType::RBRACKET) {
                    error("Empty subscript not allowed");
                }
                slice = parse_expr();
            }

            // Expect closing bracket (matches CPython: primary '[' slices ']')
            std::cerr << "[DEBUG parse_primary] Before matching ']', token=" << current_token_
                      << ", type=" << static_cast<int>(current().type)
                      << ", value='" << current().value << "'" << std::endl;
            std::cerr.flush();
            if (!match(TokenType::RBRACKET)) {
                std::ostringstream debug_msg;
                debug_msg << "[DEBUG parse_primary] ERROR: Failed to match ']', current token=" << current_token_
                          << ", type=" << static_cast<int>(current().type)
                          << ", value='" << current().value << "'";
                std::cerr << debug_msg.str() << std::endl;
                std::cerr.flush();
                error("Expected ']' - DEBUG: token=" + std::to_string(current_token_) +
                      ", type=" + std::to_string(static_cast<int>(current().type)) +
                      ", value='" + current().value + "'");
            }
            std::cerr << "[DEBUG parse_primary] Successfully matched ']'" << std::endl;
            expr = std::make_shared<ast::Subscript>(expr, slice, ast::ExprContext::Load,
                                                    expr->lineno(), expr->col_offset());
        } else if (current().type == TokenType::LPAREN) {
            // Function call: func(args) - matches CPython: primary '(' [arguments] ')'
            advance(); // consume '('

            // Parse arguments (optional) - matches CPython's arguments_rule
            std::shared_ptr<ast::Call> call_args = nullptr;
            if (current().type != TokenType::RPAREN) {
                call_args = parse_arguments();
            }

            if (!match(TokenType::RPAREN)) {
                error("Expected ')' in function call");
            }

            // Create Call with the current expression as function
            // If call_args is null, use empty args; otherwise extract args from Call node
            std::vector<std::shared_ptr<ast::Expr>> args;
            if (call_args) {
                args = call_args->args();
            }
            expr = std::make_shared<ast::Call>(expr, args, expr->lineno(), expr->col_offset());
        } else {
            // No more primary suffixes - exit the loop
            // This happens when we encounter a token that's not '.', '[', or '('
            // For example, when parsing slice upper bound '1' in '[0:1]', after parsing '1',
            // we're at ']', which doesn't match any of the primary suffixes, so we break
            break;
        }
    }

    return expr;
}

inline std::shared_ptr<ast::Expr> Parser::parse_list() {
    Token token = tokens_[current_token_ - 1]; // LBRACKET token

    std::cerr << "[DEBUG parse_list] Starting at token " << current_token_
              << ", current token type=" << static_cast<int>(current().type)
              << ", value='" << current().value << "'" << std::endl;
    std::cerr.flush();

    // Check for empty list
    if (current().type == TokenType::RBRACKET) {
        std::cerr << "[DEBUG parse_list] Empty list detected" << std::endl;
        std::cerr.flush();
        advance();
        return std::make_shared<ast::List>(std::vector<std::shared_ptr<ast::Expr>>(),
                                          ast::ExprContext::Load, token.line, token.column);
    }

    // Parse first expression
    std::cerr << "[DEBUG parse_list] Parsing first element, token=" << current_token_
              << ", type=" << static_cast<int>(current().type)
              << ", value='" << current().value << "'" << std::endl;
    std::cerr.flush();

    // Show next few tokens for context
    for (size_t i = 0; i < 5 && (current_token_ + i) < tokens_.size(); i++) {
        std::cerr << "  [DEBUG parse_list] Token[" << (current_token_ + i) << "] = type="
                  << static_cast<int>(tokens_[current_token_ + i].type)
                  << ", value='" << tokens_[current_token_ + i].value << "'" << std::endl;
    }
    std::cerr.flush();

    auto elt = parse_expr();
    std::cerr << "[DEBUG parse_list] After parsing first element, token=" << current_token_
              << ", type=" << static_cast<int>(current().type)
              << ", value='" << current().value << "'" << std::endl;

    // Show next few tokens after parsing
    for (size_t i = 0; i < 5 && (current_token_ + i) < tokens_.size(); i++) {
        std::cerr << "  [DEBUG parse_list] Token[" << (current_token_ + i) << "] = type="
                  << static_cast<int>(tokens_[current_token_ + i].type)
                  << ", value='" << tokens_[current_token_ + i].value << "'" << std::endl;
    }
    std::cerr.flush();

    // Check if this is a list comprehension (next token is 'for' or 'async')
    if (current().type == TokenType::FOR || current().type == TokenType::ASYNC) {
        std::cerr << "[DEBUG parse_list] Detected list comprehension" << std::endl;
        std::cerr.flush();
        // Parse comprehension clauses
        auto generators = parse_for_if_clauses();
        if (!match(TokenType::RBRACKET)) {
            error("Expected ']' after list comprehension");
        }
        return std::make_shared<ast::ListComp>(elt, generators, token.line, token.column);
    }

    // Regular list literal - we already have first element
    std::cerr << "[DEBUG parse_list] Regular list literal, checking for comma or ']'" << std::endl;
    std::cerr.flush();
    std::vector<std::shared_ptr<ast::Expr>> elts = {elt};
    while (match(TokenType::COMMA)) {
        std::cerr << "[DEBUG parse_list] Found comma, token=" << current_token_
                  << ", type=" << static_cast<int>(current().type) << std::endl;
        std::cerr.flush();
        if (current().type == TokenType::RBRACKET) {
            std::cerr << "[DEBUG parse_list] Trailing comma, breaking" << std::endl;
            std::cerr.flush();
            break;  // Trailing comma
        }
        std::cerr << "[DEBUG parse_list] Parsing next element, token=" << current_token_
                  << ", type=" << static_cast<int>(current().type) << std::endl;
        std::cerr.flush();
        elts.push_back(parse_expr());
        std::cerr << "[DEBUG parse_list] After parsing element, token=" << current_token_
                  << ", type=" << static_cast<int>(current().type) << std::endl;
        std::cerr.flush();
    }

    std::cerr << "[DEBUG parse_list] Before matching ']', token=" << current_token_
              << ", type=" << static_cast<int>(current().type)
              << ", value='" << current().value << "'" << std::endl;
    std::cerr.flush();
    if (!match(TokenType::RBRACKET)) {
        std::cerr << "[DEBUG parse_list] ERROR: Failed to match ']', token=" << current_token_
                  << ", type=" << static_cast<int>(current().type)
                  << ", value='" << current().value << "'" << std::endl;
        std::cerr.flush();

        // Check if this looks like a slice syntax (e.g., [0:1])
        if (current().type == TokenType::COLON) {
            error("Invalid syntax: slice notation (e.g., [0:1]) can only be used in subscripts (e.g., arr[0:1]), not as a list literal");
        } else {
            error("Expected ']' after list element");
        }
    }
    std::cerr << "[DEBUG parse_list] Successfully matched ']'" << std::endl;
    std::cerr.flush();

    return std::make_shared<ast::List>(elts, ast::ExprContext::Load, token.line, token.column);
}

inline std::shared_ptr<ast::Expr> Parser::parse_dict() {
    Token token = tokens_[current_token_ - 1]; // LBRACE token

    // Check for empty dict/set
    if (current().type == TokenType::RBRACE) {
        advance();
        return std::make_shared<ast::Dict>(std::vector<std::shared_ptr<ast::Expr>>(),
                                          std::vector<std::shared_ptr<ast::Expr>>(),
                                          token.line, token.column);
    }

    // Parse first expression
    auto first_expr = parse_expr();

    // Check if this is a set comprehension: {expr for ...} or {expr async for ...}
    if (current().type == TokenType::FOR || current().type == TokenType::ASYNC) {
        auto generators = parse_for_if_clauses();
        if (!match(TokenType::RBRACE)) {
            error("Expected '}' after set comprehension");
        }
        return std::make_shared<ast::SetComp>(first_expr, generators, token.line, token.column);
    }

    // Check if this is a dict (has colon) or set (has comma/rbrace)
    if (current().type == TokenType::COLON) {
        // This is a dict literal or dict comprehension
        advance();  // consume the colon
        
        // Parse value
        auto value = parse_expr();
        
        // Check if this is a dict comprehension: {key: value for ...} or {key: value async for ...}
        if (current().type == TokenType::FOR || current().type == TokenType::ASYNC) {
            auto generators = parse_for_if_clauses();
            if (!match(TokenType::RBRACE)) {
                error("Expected '}' after dict comprehension");
            }
            return std::make_shared<ast::DictComp>(first_expr, value, generators, token.line, token.column);
        }
        
        // Regular dict literal
        std::vector<std::shared_ptr<ast::Expr>> keys = {first_expr};
        std::vector<std::shared_ptr<ast::Expr>> values = {value};
        
        while (match(TokenType::COMMA)) {
            if (current().type == TokenType::RBRACE) {
                break;  // Trailing comma
            }
            keys.push_back(parse_expr());
            if (!match(TokenType::COLON)) {
                error("Expected ':' in dictionary");
            }
            values.push_back(parse_expr());
        }
        
        if (!match(TokenType::RBRACE)) {
            error("Expected '}'");
        }
        
        return std::make_shared<ast::Dict>(keys, values, token.line, token.column);
        
    } else if (current().type == TokenType::COMMA || current().type == TokenType::RBRACE) {
        // This is a set literal: {1, 2, 3} or {1}
        std::vector<std::shared_ptr<ast::Expr>> elts = {first_expr};
        
        while (match(TokenType::COMMA)) {
            if (current().type == TokenType::RBRACE) {
                break;  // Trailing comma
            }
            elts.push_back(parse_expr());
        }
        
        if (!match(TokenType::RBRACE)) {
            error("Expected '}'");
        }
        
        return std::make_shared<ast::Set>(elts, ast::ExprContext::Load, token.line, token.column);
        
    } else {
        error("Expected ':', ',' or '}' after expression in braces");
        return nullptr;  // Unreachable, but satisfies compiler
    }
}

inline std::shared_ptr<ast::Expr> Parser::parse_call() {
    // We've already consumed the identifier, now parse the call
    Token func_token = tokens_[current_token_ - 1]; // Previous token was the function name
    auto func = std::make_shared<ast::Name>(func_token.value, ast::ExprContext::Load,
                                           func_token.line, func_token.column);

    if (!match(TokenType::LPAREN)) {
        error("Expected '(' in function call");
    }

    std::vector<std::shared_ptr<ast::Expr>> args = parse_expr_list();

    if (!match(TokenType::RPAREN)) {
        error("Expected ')' in function call");
    }

    return std::make_shared<ast::Call>(func, args, func_token.line, func_token.column);
}

inline std::vector<std::string> Parser::parse_arg_list() {
    std::vector<std::string> args;

    if (current().type == TokenType::RPAREN) {
        return args; // Empty argument list
    }

    while (true) {
        if (current().type != TokenType::IDENTIFIER) {
            error("Expected argument name");
        }
        args.push_back(current().value);
        advance();

        if (match(TokenType::COMMA)) {
            continue;
        } else {
            break;
        }
    }

    return args;
}

// CPython-style arguments parsing
// arguments: args [','] &')' | invalid_arguments
// This matches CPython's arguments_rule exactly
inline std::shared_ptr<ast::Call> Parser::parse_arguments() {
    // Try to parse args [','] &')'
    size_t saved_pos = current_token_;

    auto args_result = parse_args();
    if (!args_result) {
        current_token_ = saved_pos;
        return nullptr;
    }

    // Optionally consume comma
    if (current().type == TokenType::COMMA) {
        advance();
    }

    // Look ahead for ')' (don't consume it - that's done by the caller)
    if (current().type != TokenType::RPAREN) {
        current_token_ = saved_pos;
        return nullptr;
    }

    return args_result;
}

// args: ','.(starred_expression | (assignment_expression | expression !':=') !'=')+ [',' kwargs] | kwargs
// Simplified version matching CPython's args_rule
inline std::shared_ptr<ast::Call> Parser::parse_args() {
    std::vector<std::shared_ptr<ast::Expr>> exprs;

    // Parse comma-separated expressions (gather pattern: ','.expr+)
    if (current().type == TokenType::RPAREN) {
        // Empty args - return Call with empty args
        return std::make_shared<ast::Call>(
            std::make_shared<ast::Name>("", ast::ExprContext::Load, 0, 0),
            exprs, 0, 0);
    }

    while (true) {
        // Parse expression (for now, just use parse_expr - starred expressions not yet supported)
        auto expr = parse_expr();

        // Check if this is a generator expression without parentheses: expr 'for' ...
        // In function calls, generator expressions can appear without outer parentheses
        // Example: sum(x * x for x in range(10))
        if (current().type == TokenType::FOR) {
            // Parse as generator expression
            auto generators = parse_for_if_clauses();
            Token gen_token = tokens_[current_token_ - 1];
            expr = std::make_shared<ast::GeneratorExp>(expr, generators, gen_token.line, gen_token.column);
        }

        exprs.push_back(expr);

        // Check if next is comma
        if (match(TokenType::COMMA)) {
            // Check if comma is followed by ')' (trailing comma) or kwargs
            if (current().type == TokenType::RPAREN) {
                break;
            }
            // Continue parsing more expressions
            continue;
        } else {
            // No comma - we're done
            break;
        }
    }

    // Return Call node with args (matches CPython's structure)
    return std::make_shared<ast::Call>(
        std::make_shared<ast::Name>("", ast::ExprContext::Load, 0, 0),
        exprs, 0, 0);
}

// Legacy function for backward compatibility
inline std::vector<std::shared_ptr<ast::Expr>> Parser::parse_expr_list() {
    auto call_result = parse_args();
    if (call_result) {
        return call_result->args();
    }
    return {};
}

// CPython-style star_targets and star_expressions parsing
// These are used in for loops to avoid parsing 'in' as a comparison operator

// star_targets: star_target | star_target (',' star_target)* [',']
// Returns a single target or a Tuple of targets
inline std::shared_ptr<ast::Expr> Parser::parse_star_targets() {
    auto first = parse_star_target();

    if (current().type == TokenType::COMMA) {
        // Multiple targets - create a tuple
        std::vector<std::shared_ptr<ast::Expr>> targets = {first};
        advance(); // consume comma

        while (current().type != TokenType::IN && current().type != TokenType::END_OF_FILE) {
            if (current().type == TokenType::COMMA && peek().type != TokenType::IDENTIFIER) {
                // Trailing comma
                advance();
                break;
            }
            targets.push_back(parse_star_target());
            if (current().type == TokenType::COMMA) {
                advance();
            } else {
                break;
            }
        }

        // Create Tuple node for multiple targets
        Token tuple_token = tokens_[current_token_ - 1]; // Use first target's token
        return std::make_shared<ast::Tuple>(targets, ast::ExprContext::Store,
                                            tuple_token.line, tuple_token.column);
    }

    return first;
}

// star_target: '*' star_target | target_with_star_atom
// Simplified: just parse as a simple target (name, attribute, subscript)
// Key: This must NOT parse 'in' as a comparison operator
inline std::shared_ptr<ast::Expr> Parser::parse_star_target() {
    Token token = current();

    // Check for '*' (starred target) - not yet implemented
    if (match(TokenType::STAR)) {
        error("Starred targets not yet supported");
    }

    // Parse as simple name (most common case: for x in ...)
    if (token.type == TokenType::IDENTIFIER) {
        auto name = std::make_shared<ast::Name>(token.value, ast::ExprContext::Store,
                                               token.line, token.column);
        advance();
        return name;
    }

    // For more complex targets (attribute, subscript), use primary but stop before comparisons
    // For now, just support simple names
    error("Complex for loop targets not yet supported");
    return nullptr;
}

// star_expressions: star_expression | star_expression (',' star_expression)+ [',']
inline std::shared_ptr<ast::Expr> Parser::parse_star_expressions() {
    auto first = parse_star_expression();

    if (current().type == TokenType::COMMA) {
        // Multiple expressions - create a tuple
        std::vector<std::shared_ptr<ast::Expr>> exprs = {first};
        advance(); // consume comma

        while (current().type != TokenType::COLON && current().type != TokenType::END_OF_FILE) {
            if (current().type == TokenType::COMMA && peek().type == TokenType::COLON) {
                // Trailing comma
                advance();
                break;
            }
            exprs.push_back(parse_star_expression());
            if (current().type == TokenType::COMMA) {
                advance();
            } else {
                break;
            }
        }

        // Create Tuple node for multiple expressions
        Token tuple_token = tokens_[current_token_ - 1]; // Use first expression's token
        return std::make_shared<ast::Tuple>(exprs, ast::ExprContext::Load,
                                            tuple_token.line, tuple_token.column);
    }

    return first;
}

// star_expression: '*' bitwise_or | expression
// According to CPython grammar: star_expression can be '*' bitwise_or OR expression
// In for loops, we use star_expressions which calls star_expression, which can be expression
// The key is that expression will parse comparisons, but in the context of 'for x in y',
// the 'in' is already consumed as a keyword, so expression parsing won't see it as comparison
inline std::shared_ptr<ast::Expr> Parser::parse_star_expression() {
    // Check for '*' (starred expression) - not yet implemented
    if (current().type == TokenType::STAR) {
        // TODO: Handle starred expressions
        error("Starred expressions not yet supported");
    }

    // Otherwise parse as regular expression (matches CPython: star_expression -> expression)
    // This is safe because 'in' is already consumed as a keyword in 'for x in y'
    return parse_expr();
}

// Try/except/finally parsing (matches CPython's try_stmt_rule, except_block_rule, finally_block_rule)
// try_stmt: 'try' ':' block except_block+ else_block? finally_block? | 'try' ':' block finally_block
inline std::shared_ptr<ast::Stmt> Parser::parse_try_stmt() {
    Token try_token = current();
    if (!match(TokenType::TRY)) {
        error("Expected 'try'");
    }

    if (!match(TokenType::COLON)) {
        error("Expected ':' after try");
    }

    // Parse try block
    std::vector<std::shared_ptr<ast::Stmt>> body;
    while (!is_at_end() && current().type != TokenType::END_OF_FILE) {
        if (current().type == TokenType::NEWLINE) {
            advance();
            continue;
        }
        // Stop at except, finally, else, def, class, or import statements
        if (current().type == TokenType::EXCEPT || current().type == TokenType::FINALLY ||
            current().type == TokenType::ELSE || current().type == TokenType::DEF ||
            current().type == TokenType::CLASS || current().type == TokenType::IMPORT ||
            current().type == TokenType::FROM) {
            break;
        }
        auto stmt = parse_stmt();
        if (stmt) {
            body.push_back(stmt);
        }
    }

    // Try to parse except blocks first (matches CPython: 'try' ':' block except_block+ else_block? finally_block?)
    size_t saved_pos = current_token_;
    std::vector<std::shared_ptr<ast::ExceptHandler>> handlers;
    std::vector<std::shared_ptr<ast::Stmt>> orelse;
    std::vector<std::shared_ptr<ast::Stmt>> finalbody;

    // Try parsing except blocks
    while (current().type == TokenType::EXCEPT) {
        auto handler = parse_except_block();
        if (handler) {
            handlers.push_back(handler);
        } else {
            break;
        }
    }

    if (!handlers.empty()) {
        // We have except blocks, try else and finally
        orelse = parse_else_block();
        if (current().type == TokenType::FINALLY) {
            finalbody = parse_finally_block();
        }
    } else {
        // No except blocks, try finally only (matches CPython: 'try' ':' block finally_block)
        current_token_ = saved_pos;
        if (current().type == TokenType::FINALLY) {
            finalbody = parse_finally_block();
        } else {
            error("Expected 'except' or 'finally' block");
        }
    }

    return std::make_shared<ast::Try>(body, handlers, orelse, finalbody,
                                      try_token.line, try_token.column);
}

// except_block: 'except' expression? 'as' NAME? ':' block
inline std::shared_ptr<ast::ExceptHandler> Parser::parse_except_block() {
    size_t saved_pos = current_token_;

    if (!match(TokenType::EXCEPT)) {
        current_token_ = saved_pos;
        return nullptr;
    }

    Token except_token = tokens_[saved_pos];
    std::shared_ptr<ast::Expr> type = nullptr;
    std::string name;

    // Parse exception type (optional)
    if (current().type != TokenType::COLON && current().type != TokenType::AS) {
        type = parse_expr();
    }

    // Parse 'as name' (optional)
    if (match(TokenType::AS)) {
        if (current().type != TokenType::IDENTIFIER) {
            error("Expected identifier after 'as'");
        }
        name = current().value;
        advance();
    }

    if (!match(TokenType::COLON)) {
        error("Expected ':' after except clause");
    }

    // Parse except block body
    std::vector<std::shared_ptr<ast::Stmt>> body;
    while (!is_at_end() && current().type != TokenType::END_OF_FILE) {
        if (current().type == TokenType::NEWLINE) {
            advance();
            continue;
        }
        // Stop at except, finally, else, def, class, or import statements
        if (current().type == TokenType::EXCEPT || current().type == TokenType::FINALLY ||
            current().type == TokenType::ELSE || current().type == TokenType::DEF ||
            current().type == TokenType::CLASS || current().type == TokenType::IMPORT ||
            current().type == TokenType::FROM) {
            break;
        }
        auto stmt = parse_stmt();
        if (stmt) {
            body.push_back(stmt);
        }
    }

    return std::make_shared<ast::ExceptHandler>(type, name, body,
                                                 except_token.line, except_token.column);
}

// finally_block: 'finally' ':' block
inline std::vector<std::shared_ptr<ast::Stmt>> Parser::parse_finally_block() {
    std::vector<std::shared_ptr<ast::Stmt>> finalbody;

    if (!match(TokenType::FINALLY)) {
        return finalbody;
    }

    if (!match(TokenType::COLON)) {
        error("Expected ':' after finally");
    }

    // Parse finally block body
    while (!is_at_end() && current().type != TokenType::END_OF_FILE) {
        if (current().type == TokenType::NEWLINE) {
            advance();
            continue;
        }
        if (current().type == TokenType::DEF) {
            break;
        }
        auto stmt = parse_stmt();
        if (stmt) {
            finalbody.push_back(stmt);
        }
    }

    return finalbody;
}

// Class definition parsing (matches CPython's class_def_rule, class_def_raw_rule)
// class_def_raw: 'class' NAME ['(' [arguments] ')'] ':' block
inline std::shared_ptr<ast::Stmt> Parser::parse_class_def() {
    // Check for decorators first
    std::vector<std::shared_ptr<ast::Expr>> decorators;
    if (current().type == TokenType::AT) {
        decorators = parse_decorators();
    }
    auto class_def = parse_class_def_raw();
    // Apply decorators if any
    if (auto cls = std::dynamic_pointer_cast<ast::ClassDef>(class_def)) {
        return std::make_shared<ast::ClassDef>(
            cls->name(), cls->bases(), cls->body(), decorators,
            cls->lineno(), cls->col_offset());
    }
    return class_def;
}

inline std::shared_ptr<ast::Stmt> Parser::parse_class_def_raw() {
    Token class_token = current();
    if (!match(TokenType::CLASS)) {
        error("Expected 'class'");
    }

    if (current().type != TokenType::IDENTIFIER) {
        error("Expected class name");
    }
    std::string name = current().value;
    Token name_token = current();
    advance();

    // Parse optional base classes: ['(' [arguments] ')']
    std::vector<std::shared_ptr<ast::Expr>> bases;
    if (match(TokenType::LPAREN)) {
        if (current().type != TokenType::RPAREN) {
            // Parse arguments (bases and keywords)
            auto call_args = parse_arguments();
            if (call_args) {
                // Extract positional args as bases
                bases = call_args->args();
                // TODO: Handle keywords for metaclass, etc.
            }
        }
        if (!match(TokenType::RPAREN)) {
            error("Expected ')' after class bases");
        }
    }

    if (!match(TokenType::COLON)) {
        error("Expected ':' after class definition");
    }

    // Parse class body
    std::vector<std::shared_ptr<ast::Stmt>> body;
    while (!is_at_end() && current().type != TokenType::END_OF_FILE) {
        if (current().type == TokenType::NEWLINE) {
            advance();
            continue;
        }
        if (current().type == TokenType::DEF || current().type == TokenType::CLASS) {
            // Allow nested defs and classes
        }
        auto stmt = parse_stmt();
        if (stmt) {
            body.push_back(stmt);
        }
    }

    return std::make_shared<ast::ClassDef>(name, bases, body, std::vector<std::shared_ptr<ast::Expr>>(),
                                          class_token.line, class_token.column);
}

// Import statement parsing (matches CPython's import_stmt_rule, import_name_rule, import_from_rule)
// import_stmt: import_name | import_from
inline std::shared_ptr<ast::Stmt> Parser::parse_import_stmt() {
    if (current().type == TokenType::IMPORT) {
        return parse_import_name();
    } else if (current().type == TokenType::FROM) {
        return parse_import_from();
    } else {
        error("Expected 'import' or 'from'");
        return nullptr;
    }
}

// import_name: 'import' dotted_as_names
inline std::shared_ptr<ast::Stmt> Parser::parse_import_name() {
    Token import_token = current();
    if (!match(TokenType::IMPORT)) {
        error("Expected 'import'");
    }

    auto names = parse_dotted_as_names();
    return std::make_shared<ast::Import>(names, import_token.line, import_token.column);
}

// import_from: 'from' ('.' | '...')* dotted_name? 'import' import_from_targets
inline std::shared_ptr<ast::Stmt> Parser::parse_import_from() {
    Token from_token = current();
    if (!match(TokenType::FROM)) {
        error("Expected 'from'");
    }

    // Parse relative import level: ('.' | '...')*
    int level = 0;
    while (current().type == TokenType::DOT) {
        level++;
        advance();
    }

    // Parse optional module name
    std::string module;
    if (current().type == TokenType::IDENTIFIER) {
        Token module_token = current();
        auto dotted = parse_dotted_name();
        if (dotted) {
            // For now, just use the first identifier (full dotted name parsing not yet implemented)
            module = module_token.value;
        }
    }

    if (!match(TokenType::IMPORT)) {
        error("Expected 'import' after 'from'");
    }

    auto names = parse_import_from_targets();
    return std::make_shared<ast::ImportFrom>(module, names, level,
                                            from_token.line, from_token.column);
}

// dotted_name: NAME ('.' NAME)*
inline std::shared_ptr<ast::Expr> Parser::parse_dotted_name() {
    if (current().type != TokenType::IDENTIFIER) {
        return nullptr;
    }

    Token first_token = current();
    std::string name = current().value;
    advance();

    // For now, just return the first name (full dotted name parsing not yet implemented)
    return std::make_shared<ast::Name>(name, ast::ExprContext::Load,
                                      first_token.line, first_token.column);
}

// dotted_as_name: dotted_name ['as' NAME]
inline std::shared_ptr<ast::Alias> Parser::parse_dotted_as_name() {
    size_t saved_pos = current_token_;
    auto dotted = parse_dotted_name();
    if (!dotted) {
        return nullptr;
    }

    Token alias_token = tokens_[saved_pos];
    // Extract name from Name node (simplified - full dotted name would be more complex)
    std::string name;
    if (auto name_node = std::dynamic_pointer_cast<ast::Name>(dotted)) {
        name = name_node->id();
    } else {
        name = dotted->to_string(); // Fallback
    }
    std::string asname;

    if (match(TokenType::AS)) {
        if (current().type != TokenType::IDENTIFIER) {
            error("Expected identifier after 'as'");
        }
        asname = current().value;
        advance();
    }

    return std::make_shared<ast::Alias>(name, asname, alias_token.line, alias_token.column);
}

// dotted_as_names: ','.dotted_as_name+
inline std::vector<std::shared_ptr<ast::Alias>> Parser::parse_dotted_as_names() {
    std::vector<std::shared_ptr<ast::Alias>> names;

    while (true) {
        auto alias = parse_dotted_as_name();
        if (!alias) {
            break;
        }
        names.push_back(alias);

        if (match(TokenType::COMMA)) {
            continue;
        } else {
            break;
        }
    }

    return names;
}

// import_from_as_name: NAME ['as' NAME]
inline std::shared_ptr<ast::Alias> Parser::parse_import_from_as_name() {
    if (current().type != TokenType::IDENTIFIER) {
        return nullptr;
    }

    Token name_token = current();
    std::string name = current().value;
    advance();

    std::string asname;
    if (match(TokenType::AS)) {
        if (current().type != TokenType::IDENTIFIER) {
            error("Expected identifier after 'as'");
        }
        asname = current().value;
        advance();
    }

    return std::make_shared<ast::Alias>(name, asname, name_token.line, name_token.column);
}

// import_from_as_names: ','.import_from_as_name+
inline std::vector<std::shared_ptr<ast::Alias>> Parser::parse_import_from_as_names() {
    std::vector<std::shared_ptr<ast::Alias>> names;

    while (true) {
        auto alias = parse_import_from_as_name();
        if (!alias) {
            break;
        }
        names.push_back(alias);

        if (match(TokenType::COMMA)) {
            continue;
        } else {
            break;
        }
    }

    return names;
}

// import_from_targets: '(' import_from_as_names [','] ')' | import_from_as_names !',' | '*'
inline std::vector<std::shared_ptr<ast::Alias>> Parser::parse_import_from_targets() {
    // Check for '*'
    if (current().type == TokenType::STAR) {
        advance();
        // Return special alias for star import
        Token star_token = tokens_[current_token_ - 1];
        return {std::make_shared<ast::Alias>("*", "", star_token.line, star_token.column)};
    }

    // Check for parenthesized import
    if (match(TokenType::LPAREN)) {
        auto names = parse_import_from_as_names();
        if (match(TokenType::COMMA)) {
            // Trailing comma allowed
        }
        if (!match(TokenType::RPAREN)) {
            error("Expected ')' after import list");
        }
        return names;
    }

    // Regular import list
    return parse_import_from_as_names();
}

// Simple statement parsing (matches CPython's pass_stmt_rule, raise_stmt_rule, etc.)
// pass_stmt: 'pass'
inline std::shared_ptr<ast::Stmt> Parser::parse_pass_stmt() {
    Token pass_token = current();
    if (!match(TokenType::PASS)) {
        error("Expected 'pass'");
    }
    return std::make_shared<ast::Pass>(pass_token.line, pass_token.column);
}

// raise_stmt: 'raise' expression? 'from' expression? | 'raise' expression? | 'raise'
inline std::shared_ptr<ast::Stmt> Parser::parse_raise_stmt() {
    Token raise_token = current();
    if (!match(TokenType::RAISE)) {
        error("Expected 'raise'");
    }

    std::shared_ptr<ast::Expr> exc = nullptr;
    std::shared_ptr<ast::Expr> cause = nullptr;

    // Parse optional exception expression
    if (current().type != TokenType::NEWLINE && current().type != TokenType::END_OF_FILE) {
        if (current().type == TokenType::FROM) {
            // Bare 'raise' or 'raise from' - skip
        } else {
            exc = parse_expr();
        }
    }

    // Parse optional 'from' cause
    if (match(TokenType::FROM)) {
        cause = parse_expr();
    }

    return std::make_shared<ast::Raise>(exc, cause, raise_token.line, raise_token.column);
}

// del_stmt: 'del' del_targets
// Simplified: use star_targets for now (del_targets is similar)
inline std::shared_ptr<ast::Stmt> Parser::parse_del_stmt() {
    Token del_token = current();
    if (!match(TokenType::DEL)) {
        error("Expected 'del'");
    }

    // Parse targets (comma-separated)
    std::vector<std::shared_ptr<ast::Expr>> targets;
    while (true) {
        // For now, parse as star_targets (del_targets is similar but with Del context)
        auto target = parse_star_targets();
        targets.push_back(target);

        if (match(TokenType::COMMA)) {
            continue;
        } else {
            break;
        }
    }

    return std::make_shared<ast::Delete>(targets, del_token.line, del_token.column);
}

// assert_stmt: 'assert' expression [',' expression]
inline std::shared_ptr<ast::Stmt> Parser::parse_assert_stmt() {
    Token assert_token = current();
    if (!match(TokenType::ASSERT)) {
        error("Expected 'assert'");
    }

    auto test = parse_expr();
    std::shared_ptr<ast::Expr> msg = nullptr;

    // Parse optional message
    if (match(TokenType::COMMA)) {
        msg = parse_expr();
    }

    return std::make_shared<ast::Assert>(test, msg, assert_token.line, assert_token.column);
}

// global_stmt: 'global' ','.NAME+
inline std::shared_ptr<ast::Stmt> Parser::parse_global_stmt() {
    Token global_token = current();
    if (!match(TokenType::GLOBAL)) {
        error("Expected 'global'");
    }

    std::vector<std::string> names;
    while (true) {
        if (current().type != TokenType::IDENTIFIER) {
            error("Expected identifier in global statement");
        }
        names.push_back(current().value);
        advance();

        if (match(TokenType::COMMA)) {
            continue;
        } else {
            break;
        }
    }

    return std::make_shared<ast::Global>(names, global_token.line, global_token.column);
}

// nonlocal_stmt: 'nonlocal' ','.NAME+
inline std::shared_ptr<ast::Stmt> Parser::parse_nonlocal_stmt() {
    Token nonlocal_token = current();
    if (!match(TokenType::NONLOCAL)) {
        error("Expected 'nonlocal'");
    }

    std::vector<std::string> names;
    while (true) {
        if (current().type != TokenType::IDENTIFIER) {
            error("Expected identifier in nonlocal statement");
        }
        names.push_back(current().value);
        advance();

        if (match(TokenType::COMMA)) {
            continue;
        } else {
            break;
        }
    }

    return std::make_shared<ast::Nonlocal>(names, nonlocal_token.line, nonlocal_token.column);
}

// with_stmt: 'with' ( '(' ','.with_item+ ','? ')' | ','.with_item+ ) ':' block
// Matches CPython's with_stmt_rule
inline std::shared_ptr<ast::Stmt> Parser::parse_with_stmt() {
    Token with_token = current();
    if (!match(TokenType::WITH)) {
        error("Expected 'with'");
    }

    std::vector<ast::WithItem> items;

    // Check for parenthesized form: 'with' '(' items ')'
    bool has_parens = match(TokenType::LPAREN);

    // Parse first with_item
    items.push_back(parse_with_item());

    // Parse remaining items (comma-separated)
    while (match(TokenType::COMMA)) {
        // Check for optional trailing comma before closing paren
        if (has_parens && current().type == TokenType::RPAREN) {
            break;
        }
        items.push_back(parse_with_item());
    }

    // Consume closing paren if present
    if (has_parens) {
        if (!match(TokenType::RPAREN)) {
            error("Expected ')' after with items");
        }
    }

    // Expect colon
    if (!match(TokenType::COLON)) {
        error("Expected ':' after with statement");
    }

    // Parse body
    std::vector<std::shared_ptr<ast::Stmt>> body;
    while (current().type != TokenType::END_OF_FILE &&
           current().type != TokenType::DEDENT &&
           current().type != TokenType::NEWLINE) {
        body.push_back(parse_stmt());
    }

    return std::make_shared<ast::With>(items, body, with_token.line, with_token.column);
}

// Parse async with statement (Python 3.5+)
inline std::shared_ptr<ast::Stmt> Parser::parse_async_with() {
    Token async_token = tokens_[current_token_ - 1]; // 'async' was already consumed
    if (!match(TokenType::WITH)) {
        error("Expected 'with' after 'async'");
    }

    std::vector<ast::WithItem> items;

    // Check for parenthesized form: 'with' '(' items ')'
    bool has_parens = match(TokenType::LPAREN);

    // Parse first with_item
    items.push_back(parse_with_item());

    // Parse remaining items (comma-separated)
    while (match(TokenType::COMMA)) {
        // Check for optional trailing comma before closing paren
        if (has_parens && current().type == TokenType::RPAREN) {
            break;
        }
        items.push_back(parse_with_item());
    }

    // Consume closing paren if present
    if (has_parens) {
        if (!match(TokenType::RPAREN)) {
            error("Expected ')' after with items");
        }
    }

    // Expect colon
    if (!match(TokenType::COLON)) {
        error("Expected ':' after async with statement");
    }

    // Parse body
    std::vector<std::shared_ptr<ast::Stmt>> body;
    while (current().type != TokenType::END_OF_FILE &&
           current().type != TokenType::DEDENT &&
           current().type != TokenType::NEWLINE) {
        body.push_back(parse_stmt());
    }

    return std::make_shared<ast::AsyncWith>(items, body, async_token.line, async_token.column);
}

// with_item: expression ['as' star_target]
// Matches CPython's with_item_rule
inline ast::WithItem Parser::parse_with_item() {
    // Parse context expression
    auto context_expr = parse_expr();

    // Optional 'as' target
    std::shared_ptr<ast::Expr> optional_vars = nullptr;
    if (match(TokenType::AS)) {
        // Parse target (use star_target for now, simplified)
        optional_vars = parse_star_target();
    }

    return ast::WithItem(context_expr, optional_vars);
}

// lambdef: 'lambda' [lambda_params] ':' expression
// Simplified: lambda [arg_list] ':' expression
// Note: This function is called after 'lambda' token has already been matched
inline std::shared_ptr<ast::Expr> Parser::parse_lambda() {
    Token lambda_token = tokens_[current_token_ - 1]; // Get the lambda token we just matched

    // Parse optional parameters
    std::vector<std::string> args;
    if (current().type != TokenType::COLON) {
        // Parse argument list
        args = parse_arg_list();
    }

    // Expect colon
    if (!match(TokenType::COLON)) {
        error("Expected ':' after lambda parameters");
    }

    // Parse body expression
    auto body = parse_expr();

    return std::make_shared<ast::Lambda>(args, body, lambda_token.line, lambda_token.column);
}

// yield_expr: 'yield' ['from' expression | star_expressions]
// Matches CPython's yield_expr rule
// Note: This function is called after 'yield' token has already been matched
inline std::shared_ptr<ast::Expr> Parser::parse_yield_expr() {
    Token yield_token = tokens_[current_token_ - 1]; // Get the yield token we just matched

    // Check for 'yield from'
    if (match(TokenType::FROM)) {
        auto value = parse_expr();
        return std::make_shared<ast::YieldFrom>(value, yield_token.line, yield_token.column);
    }

    // Optional value (star_expressions)
    std::shared_ptr<ast::Expr> value = nullptr;
    if (current().type != TokenType::NEWLINE &&
        current().type != TokenType::END_OF_FILE &&
        current().type != TokenType::COMMA &&
        current().type != TokenType::RPAREN &&
        current().type != TokenType::RBRACKET &&
        current().type != TokenType::COLON) {
        // Parse star_expressions (simplified: just parse expression)
        value = parse_star_expression();
    }

    return std::make_shared<ast::Yield>(value, yield_token.line, yield_token.column);
}

// for_if_clauses: for_if_clause+
// Matches CPython's for_if_clauses rule
inline std::vector<ast::Comprehension> Parser::parse_for_if_clauses() {
    std::vector<ast::Comprehension> generators;
    generators.push_back(parse_for_if_clause());

    // Parse additional for/if clauses
    while (current().type == TokenType::FOR || current().type == TokenType::IF || current().type == TokenType::ASYNC) {
        if (current().type == TokenType::FOR || current().type == TokenType::ASYNC) {
            generators.push_back(parse_for_if_clause());
        } else if (current().type == TokenType::IF) {
            // Additional if condition for the last comprehension
            if (generators.empty()) {
                error("'if' clause must follow a 'for' clause");
            }
            advance();  // consume 'if'
            auto condition = parse_expr();
            generators.back().ifs.push_back(condition);
        }
    }

    return generators;
}

// for_if_clause: ['async'] 'for' star_targets 'in' disjunction ('if' disjunction)*
// Matches CPython's for_if_clause rule (Python 3.6+ adds async support)
inline ast::Comprehension Parser::parse_for_if_clause() {
    // Check for 'async' keyword (Python 3.6+)
    bool is_async = false;
    if (current().type == TokenType::ASYNC) {
        is_async = true;
        advance();  // consume 'async'
    }

    if (!match(TokenType::FOR)) {
        error("Expected 'for' in comprehension");
    }

    // Parse target (star_targets)
    auto target = parse_star_targets();

    // Expect 'in'
    if (!match(TokenType::IN)) {
        error("Expected 'in' in comprehension");
    }

    // Parse iterable (disjunction)
    auto iter = parse_expr();  // Use parse_expr() which handles disjunction

    // Parse optional if conditions
    std::vector<std::shared_ptr<ast::Expr>> ifs;
    while (match(TokenType::IF)) {
        ifs.push_back(parse_expr());
    }

    return ast::Comprehension(target, iter, ifs, is_async);
}

// slice: [lower] ':' [upper] [ ':' [step] ]
// Matches CPython's slice_rule: expression? ':' expression? [':' expression?] OR named_expression
// Returns nullptr if not a slice (no ':' found), allowing fallback to named_expression
// This matches CPython's logic where slice_rule tries slice pattern first, then named_expression
inline std::shared_ptr<ast::Expr> Parser::parse_slice() {
    // Save position for backtracking (matches CPython's _mark)
    size_t saved_pos = current_token_;
    Token slice_token = current();

    std::cerr << "[DEBUG parse_slice] Starting at token " << saved_pos
              << ", type=" << static_cast<int>(current().type)
              << ", value='" << current().value << "'" << std::endl;
    std::cerr.flush();

    std::shared_ptr<ast::Expr> lower = nullptr;
    std::shared_ptr<ast::Expr> upper = nullptr;
    std::shared_ptr<ast::Expr> step = nullptr;

    // Try to parse first expression (optional) - matches CPython: a=expression_rule(p), !p->error_indicator
    // In CPython, expression_rule can return NULL if optional and not present
    // If we're already at ':', lower is empty (OK)
    if (current().type != TokenType::COLON) {
        std::cerr << "[DEBUG parse_slice] Before parsing lower, token=" << current_token_
                  << ", type=" << static_cast<int>(current().type) << std::endl;
        std::cerr.flush();
        // Try to parse expression
        lower = parse_expr();
        std::cerr << "[DEBUG parse_slice] After parsing lower, token=" << current_token_
                  << ", type=" << static_cast<int>(current().type)
                  << ", value='" << current().value << "'" << std::endl;
        std::cerr.flush();
        // After parsing, check if we're at ':' - if not, this is not a slice
        if (current().type != TokenType::COLON) {
            std::cerr << "[DEBUG parse_slice] Not at ':' after lower, returning nullptr" << std::endl;
            // Not a slice - reset and return nullptr (matches CPython's fallback to named_expression)
            current_token_ = saved_pos;
            return nullptr;
        }
    } else {
        std::cerr << "[DEBUG parse_slice] Already at ':', lower is empty" << std::endl;
    }

    // Expect first ':' - matches CPython: _PyPegen_expect_token(p, 11) for ':'
    // This is REQUIRED for a slice - we should be at ':' now
    if (current().type != TokenType::COLON) {
        std::cerr << "[DEBUG parse_slice] Expected ':' but got type=" << static_cast<int>(current().type) << std::endl;
        // Not a slice - reset and return nullptr
        current_token_ = saved_pos;
        return nullptr;
    }
    advance();  // consume ':'
    std::cerr << "[DEBUG parse_slice] After consuming ':', token=" << current_token_
              << ", type=" << static_cast<int>(current().type)
              << ", value='" << current().value << "'" << std::endl;
    std::cerr.flush();

    // Try to parse second expression (optional) - matches CPython: b=expression_rule(p), !p->error_indicator
    // In CPython, expression_rule can return NULL if optional and not present
    // expression_rule stops naturally at ']' or ':' - it doesn't consume them
    // The key is that expression_rule in CPython uses PEG parsing which naturally stops
    // at tokens that don't match the grammar. In our recursive descent parser,
    // parse_expr() should also stop at ']' or ':' because they're not valid expression tokens
    if (current().type != TokenType::COLON && current().type != TokenType::RBRACKET) {
        std::cerr << "[DEBUG parse_slice] Before parsing upper, token=" << current_token_
                  << ", type=" << static_cast<int>(current().type)
                  << ", value='" << current().value << "'" << std::endl;
        std::cerr.flush();
        // Try to parse expression - parse_expr() should stop at ']' or ':'
        // Save position before parsing to verify
        size_t before_upper = current_token_;
        upper = parse_expr();
        size_t after_upper = current_token_;
        std::cerr << "[DEBUG parse_slice] After parsing upper, token=" << current_token_
                  << " (was " << before_upper << "), type=" << static_cast<int>(current().type)
                  << ", value='" << current().value << "'" << std::endl;
        std::cerr.flush();

        // After parsing, we should be at ']' (end of slice) or ':' (start of step)
        // Verify the token position is correct
        if (current().type != TokenType::COLON && current().type != TokenType::RBRACKET) {
            std::cerr << "[DEBUG parse_slice] ERROR: After parsing upper, not at ']' or ':', got type="
                      << static_cast<int>(current().type) << std::endl;
            std::cerr.flush();
            // Parsing went too far - not a valid slice
            // This means parse_expr() consumed ']' or something else, which shouldn't happen
            current_token_ = saved_pos;
            return nullptr;
        }
    } else {
        std::cerr << "[DEBUG parse_slice] Upper is empty (already at ':' or ']')" << std::endl;
    }

    // Try to parse step (optional) - matches CPython: c=_tmp_65_rule(p), !p->error_indicator
    // _tmp_65_rule is: ':' expression?
    if (current().type == TokenType::COLON) {
        std::cerr << "[DEBUG parse_slice] Found step ':', consuming it" << std::endl;
        advance();  // consume second ':'
        // Step expression is optional - if we're at ']', step is None
        if (current().type != TokenType::RBRACKET) {
            std::cerr << "[DEBUG parse_slice] Before parsing step, token=" << current_token_
                      << ", type=" << static_cast<int>(current().type) << std::endl;
            step = parse_expr();
            std::cerr << "[DEBUG parse_slice] After parsing step, token=" << current_token_
                      << ", type=" << static_cast<int>(current().type) << std::endl;
            // After parsing step, we should be at ']'
            if (current().type != TokenType::RBRACKET) {
                std::cerr << "[DEBUG parse_slice] ERROR: After parsing step, not at ']'" << std::endl;
                current_token_ = saved_pos;
                return nullptr;
            }
        } else {
            std::cerr << "[DEBUG parse_slice] Step is empty (already at ']')" << std::endl;
        }
    }

    // Successfully parsed slice - matches CPython: _PyAST_Slice(a, b, c, EXTRA)
    // At this point, we should be at ']' (the caller will consume it)
    std::cerr << "[DEBUG parse_slice] Successfully parsed slice, returning. Current token=" << current_token_
              << ", type=" << static_cast<int>(current().type)
              << ", value='" << current().value << "'" << std::endl;
    std::cerr.flush();
    return std::make_shared<ast::Slice>(lower, upper, step, slice_token.line, slice_token.column);
}

// Decorators parsing (matches CPython's decorators rule)
// decorators: ('@' named_expression NEWLINE)+
inline std::vector<std::shared_ptr<ast::Expr>> Parser::parse_decorators() {
    std::vector<std::shared_ptr<ast::Expr>> decorators;

    while (current().type == TokenType::AT) {
        advance();  // consume '@'

        // Parse the decorator expression (named_expression in CPython)
        // This can be a simple name, attribute access, or function call
        auto decorator = parse_expr();
        if (!decorator) {
            error("Expected expression after '@'");
        }
        decorators.push_back(decorator);

        // Expect NEWLINE after decorator (matches CPython: decorators rule)
        if (current().type == TokenType::NEWLINE) {
            advance();
        } else if (current().type != TokenType::AT &&
                   current().type != TokenType::DEF &&
                   current().type != TokenType::CLASS) {
            // If not at another decorator or def/class, this might be an error
            // But be lenient - allow if next is def/class
            break;
        }
    }

    return decorators;
}


} // namespace parser
} // namespace cpython_cpp

#endif // CPYTHON_CPP_PARSER_HPP



