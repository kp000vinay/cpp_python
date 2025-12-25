#ifndef CPYTHON_CPP_PEG_PARSER_IMPL_HPP
#define CPYTHON_CPP_PEG_PARSER_IMPL_HPP

/**
 * PEG Parser Implementation - Main Interface
 * 
 * This is the main entry point for parsing Python source code.
 * It provides a clean API for parsing expressions, statements, and modules.
 * 
 * Usage:
 *     auto module = parse("x = 1 + 2\n");
 *     auto expr = parse_expr("1 + 2 * 3");
 */

#include "peg_parser.hpp"
#include "grammar/expressions.hpp"
#include "grammar/statements.hpp"
#include "../ast/module.hpp"
#include <memory>
#include <string>
#include <string_view>

namespace cpython_cpp {
namespace parser {

/**
 * Parse a Python expression from source code
 * 
 * @param source The Python expression source code
 * @return Shared pointer to the parsed expression AST, or nullptr on failure
 */
inline std::shared_ptr<ast::Expr> parse_expr(std::string_view source) {
    std::string src{source};
    Tokenizer tokenizer{src};
    auto tokens = tokenizer.tokenize();
    
    ParserState state(std::move(tokens));
    auto result = grammar::parse_expression(state);
    
    if (result) {
        return *result;
    }
    return nullptr;
}

/**
 * Parse a Python module (multiple statements) from source code
 * 
 * @param source The Python source code
 * @return Shared pointer to the parsed Module AST, or nullptr on failure
 */
inline std::shared_ptr<ast::Module> parse(std::string_view source) {
    std::string src{source};
    Tokenizer tokenizer{src};
    auto tokens = tokenizer.tokenize();
    
    ParserState state(std::move(tokens));
    std::vector<std::shared_ptr<ast::Stmt>> body;
    
    while (!state.at_end()) {
        // Skip newlines
        while (state.current().type == TokenType::NEWLINE) {
            state.advance();
        }
        
        if (state.at_end()) {
            break;
        }
        
        auto stmt = grammar::parse_statement(state);
        if (stmt) {
            body.push_back(*stmt);
        } else {
            // Skip to next line on error
            while (!state.at_end() && state.current().type != TokenType::NEWLINE) {
                state.advance();
            }
        }
    }
    
    return std::make_shared<ast::Module>(body);
}

/**
 * Parse a single Python statement from source code
 * 
 * @param source The Python statement source code
 * @return Shared pointer to the parsed statement AST, or nullptr on failure
 */
inline std::shared_ptr<ast::Stmt> parse_stmt(std::string_view source) {
    std::string src{source};
    Tokenizer tokenizer{src};
    auto tokens = tokenizer.tokenize();
    
    ParserState state(std::move(tokens));
    auto result = grammar::parse_statement(state);
    
    if (result) {
        return *result;
    }
    return nullptr;
}

/**
 * PEGParser class - Object-oriented interface
 * 
 * Provides a class-based interface for parsing Python source code.
 * Maintains internal state for error reporting and incremental parsing.
 */
class PEGParser {
public:
    explicit PEGParser(std::string_view source)
        : source_(source), tokenizer_(std::string(source)) {
        tokens_ = tokenizer_.tokenize();
    }
    
    /**
     * Parse the source as a module (file)
     */
    std::shared_ptr<ast::Module> parse_module() {
        ParserState state(tokens_);
        std::vector<std::shared_ptr<ast::Stmt>> body;
        
        while (!state.at_end()) {
            while (state.current().type == TokenType::NEWLINE) {
                state.advance();
            }
            
            if (state.at_end()) {
                break;
            }
            
            auto stmt = grammar::parse_statement(state);
            if (stmt) {
                body.push_back(*stmt);
            } else {
                // Record error position
                if (!state.at_end()) {
                    error_line_ = state.current().line;
                    error_col_ = state.current().column;
                    has_error_ = true;
                }
                // Skip to next line
                while (!state.at_end() && state.current().type != TokenType::NEWLINE) {
                    state.advance();
                }
            }
        }
        
        return std::make_shared<ast::Module>(body);
    }
    
    /**
     * Parse the source as a single expression
     */
    std::shared_ptr<ast::Expr> parse_expression() {
        ParserState state(tokens_);
        auto result = grammar::parse_expression(state);
        
        if (result) {
            return *result;
        }
        
        if (!state.at_end()) {
            error_line_ = state.current().line;
            error_col_ = state.current().column;
            has_error_ = true;
        }
        
        return nullptr;
    }
    
    /**
     * Check if there was a parsing error
     */
    bool has_error() const { return has_error_; }
    
    /**
     * Get the line number of the first error
     */
    int error_line() const { return error_line_; }
    
    /**
     * Get the column offset of the first error
     */
    int error_col() const { return error_col_; }
    
    /**
     * Get the tokens (for debugging)
     */
    const std::vector<Token>& tokens() const { return tokens_; }

private:
    std::string source_;
    Tokenizer tokenizer_;
    std::vector<Token> tokens_;
    bool has_error_ = false;
    int error_line_ = 0;
    int error_col_ = 0;
};

} // namespace parser
} // namespace cpython_cpp

#endif // CPYTHON_CPP_PEG_PARSER_IMPL_HPP
