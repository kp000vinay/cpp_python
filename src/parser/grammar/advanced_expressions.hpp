#ifndef CPYTHON_CPP_GRAMMAR_ADVANCED_EXPRESSIONS_HPP
#define CPYTHON_CPP_GRAMMAR_ADVANCED_EXPRESSIONS_HPP

/**
 * Advanced Expression Grammar Rules
 * 
 * Implements additional CPython expression features:
 * - Lambda expressions
 * - List/Dict/Set comprehensions
 * - Generator expressions
 * - Starred expressions
 * - Named expressions (walrus operator)
 * - F-strings
 * 
 * Reference: CPython Grammar/python.gram
 */

#include "expressions.hpp"
#include "../../ast/expr.hpp"
#include <memory>

namespace cpython_cpp {
namespace parser {
namespace grammar {

// ============================================================================
// Lambda Expression
// ============================================================================

/**
 * lambdef[expr_ty]:
 *     | 'lambda' lambda_params? ':' expression
 */
inline ParseResult<ExprPtr> parse_lambda(ParserState& state) {
    if (state.current().type != TokenType::LAMBDA) {
        return std::nullopt;
    }
    
    state.update_location();
    int line = state.start_lineno;
    int col = state.start_col_offset;
    state.advance();  // consume 'lambda'
    
    // Parse parameters (simplified - just names)
    std::vector<std::string> params;
    while (state.current().type == TokenType::IDENTIFIER) {
        params.push_back(state.current().value);
        state.advance();
        if (state.current().type == TokenType::COMMA) {
            state.advance();
        } else {
            break;
        }
    }
    
    // Expect ':'
    if (state.current().type != TokenType::COLON) {
        return std::nullopt;
    }
    state.advance();
    
    // Parse body expression
    auto body = parse_expression(state);
    if (!body) {
        return std::nullopt;
    }
    
    return make_expr(std::make_shared<ast::Lambda>(params, *body, line, col));
}

// ============================================================================
// Named Expression (Walrus Operator)
// ============================================================================

/**
 * named_expression[expr_ty]:
 *     | NAME ':=' expression
 */
inline ParseResult<ExprPtr> parse_named_expression(ParserState& state) {
    size_t mark = state.mark();
    
    if (state.current().type != TokenType::IDENTIFIER) {
        return std::nullopt;
    }
    
    state.update_location();
    int line = state.start_lineno;
    int col = state.start_col_offset;
    std::string name = state.current().value;
    state.advance();
    
    // Check for ':='
    if (state.current().type != TokenType::COLONEQUAL &&
        state.current().type != TokenType::WALRUS) {
        state.reset(mark);
        return std::nullopt;
    }
    state.advance();
    
    auto value = parse_expression(state);
    if (!value) {
        state.reset(mark);
        return std::nullopt;
    }
    
    auto target = std::make_shared<ast::Name>(name, ast::ExprContext::Store, line, col);
    return make_expr(std::make_shared<ast::NamedExpr>(target, *value, line, col));
}

// ============================================================================
// Starred Expression
// ============================================================================

/**
 * starred_expression[expr_ty]:
 *     | '*' expression
 */
inline ParseResult<ExprPtr> parse_starred_expression(ParserState& state) {
    if (state.current().type != TokenType::STAR) {
        return std::nullopt;
    }
    
    state.update_location();
    int line = state.start_lineno;
    int col = state.start_col_offset;
    state.advance();
    
    auto value = parse_expression(state);
    if (!value) {
        return std::nullopt;
    }
    
    return make_expr(std::make_shared<ast::Starred>(*value, ast::ExprContext::Load, line, col));
}

// ============================================================================
// Comprehension helpers
// ============================================================================

/**
 * for_if_clauses[asdl_comprehension_seq*]:
 *     | for_if_clause+
 * 
 * for_if_clause[comprehension_ty]:
 *     | 'for' star_targets 'in' disjunction ('if' disjunction)*
 */
inline ParseResult<std::vector<ast::Comprehension>> parse_for_if_clauses(ParserState& state) {
    std::vector<ast::Comprehension> comprehensions;
    
    while (state.current().type == TokenType::FOR) {
        state.advance();  // consume 'for'
        
        // Parse target
        auto target = parse_expression(state);
        if (!target) {
            return std::nullopt;
        }
        
        // Expect 'in'
        if (state.current().type != TokenType::IN) {
            return std::nullopt;
        }
        state.advance();
        
        // Parse iterator
        auto iter = parse_disjunction(state);
        if (!iter) {
            return std::nullopt;
        }
        
        // Parse optional 'if' conditions
        std::vector<ExprPtr> ifs;
        while (state.current().type == TokenType::IF) {
            state.advance();
            auto condition = parse_disjunction(state);
            if (!condition) {
                break;
            }
            ifs.push_back(*condition);
        }
        
        comprehensions.push_back(ast::Comprehension(*target, *iter, ifs, false));
    }
    
    if (comprehensions.empty()) {
        return std::nullopt;
    }
    
    return ParseResult<std::vector<ast::Comprehension>>(comprehensions);
}

// ============================================================================
// List Comprehension
// ============================================================================

/**
 * listcomp[expr_ty]:
 *     | '[' expression for_if_clauses ']'
 */
inline ParseResult<ExprPtr> parse_listcomp(ParserState& state) {
    if (state.current().type != TokenType::LSQB &&
        state.current().type != TokenType::LBRACKET) {
        return std::nullopt;
    }
    
    size_t mark = state.mark();
    state.update_location();
    int line = state.start_lineno;
    int col = state.start_col_offset;
    state.advance();  // consume '['
    
    // Parse element expression
    auto elt = parse_expression(state);
    if (!elt) {
        state.reset(mark);
        return std::nullopt;
    }
    
    // Check for 'for' (distinguishes listcomp from list literal)
    if (state.current().type != TokenType::FOR) {
        state.reset(mark);
        return std::nullopt;
    }
    
    // Parse for_if_clauses
    auto generators = parse_for_if_clauses(state);
    if (!generators) {
        state.reset(mark);
        return std::nullopt;
    }
    
    // Expect ']'
    if (state.current().type != TokenType::RSQB &&
        state.current().type != TokenType::RBRACKET) {
        state.reset(mark);
        return std::nullopt;
    }
    state.advance();
    
    return make_expr(std::make_shared<ast::ListComp>(*elt, *generators, line, col));
}

// ============================================================================
// Set Comprehension
// ============================================================================

/**
 * setcomp[expr_ty]:
 *     | '{' expression for_if_clauses '}'
 */
inline ParseResult<ExprPtr> parse_setcomp(ParserState& state) {
    if (state.current().type != TokenType::LBRACE) {
        return std::nullopt;
    }
    
    size_t mark = state.mark();
    state.update_location();
    int line = state.start_lineno;
    int col = state.start_col_offset;
    state.advance();  // consume '{'
    
    // Parse element expression
    auto elt = parse_expression(state);
    if (!elt) {
        state.reset(mark);
        return std::nullopt;
    }
    
    // Check for 'for' (distinguishes setcomp from set literal)
    if (state.current().type != TokenType::FOR) {
        state.reset(mark);
        return std::nullopt;
    }
    
    // Parse for_if_clauses
    auto generators = parse_for_if_clauses(state);
    if (!generators) {
        state.reset(mark);
        return std::nullopt;
    }
    
    // Expect '}'
    if (state.current().type != TokenType::RBRACE) {
        state.reset(mark);
        return std::nullopt;
    }
    state.advance();
    
    return make_expr(std::make_shared<ast::SetComp>(*elt, *generators, line, col));
}

// ============================================================================
// Dict Comprehension
// ============================================================================

/**
 * dictcomp[expr_ty]:
 *     | '{' kvpair for_if_clauses '}'
 */
inline ParseResult<ExprPtr> parse_dictcomp(ParserState& state) {
    if (state.current().type != TokenType::LBRACE) {
        return std::nullopt;
    }
    
    size_t mark = state.mark();
    state.update_location();
    int line = state.start_lineno;
    int col = state.start_col_offset;
    state.advance();  // consume '{'
    
    // Parse key expression
    auto key = parse_expression(state);
    if (!key) {
        state.reset(mark);
        return std::nullopt;
    }
    
    // Expect ':'
    if (state.current().type != TokenType::COLON) {
        state.reset(mark);
        return std::nullopt;
    }
    state.advance();
    
    // Parse value expression
    auto value = parse_expression(state);
    if (!value) {
        state.reset(mark);
        return std::nullopt;
    }
    
    // Check for 'for' (distinguishes dictcomp from dict literal)
    if (state.current().type != TokenType::FOR) {
        state.reset(mark);
        return std::nullopt;
    }
    
    // Parse for_if_clauses
    auto generators = parse_for_if_clauses(state);
    if (!generators) {
        state.reset(mark);
        return std::nullopt;
    }
    
    // Expect '}'
    if (state.current().type != TokenType::RBRACE) {
        state.reset(mark);
        return std::nullopt;
    }
    state.advance();
    
    return make_expr(std::make_shared<ast::DictComp>(*key, *value, *generators, line, col));
}

// ============================================================================
// Generator Expression
// ============================================================================

/**
 * genexp[expr_ty]:
 *     | '(' expression for_if_clauses ')'
 */
inline ParseResult<ExprPtr> parse_genexp(ParserState& state) {
    if (state.current().type != TokenType::LPAR &&
        state.current().type != TokenType::LPAREN) {
        return std::nullopt;
    }
    
    size_t mark = state.mark();
    state.update_location();
    int line = state.start_lineno;
    int col = state.start_col_offset;
    state.advance();  // consume '('
    
    // Parse element expression
    auto elt = parse_expression(state);
    if (!elt) {
        state.reset(mark);
        return std::nullopt;
    }
    
    // Check for 'for' (distinguishes genexp from parenthesized expr)
    if (state.current().type != TokenType::FOR) {
        state.reset(mark);
        return std::nullopt;
    }
    
    // Parse for_if_clauses
    auto generators = parse_for_if_clauses(state);
    if (!generators) {
        state.reset(mark);
        return std::nullopt;
    }
    
    // Expect ')'
    if (state.current().type != TokenType::RPAR &&
        state.current().type != TokenType::RPAREN) {
        state.reset(mark);
        return std::nullopt;
    }
    state.advance();
    
    return make_expr(std::make_shared<ast::GeneratorExp>(*elt, *generators, line, col));
}

// ============================================================================
// Yield Expression
// ============================================================================

/**
 * yield_expr[expr_ty]:
 *     | 'yield' 'from' expression
 *     | 'yield' [star_expressions]
 */
inline ParseResult<ExprPtr> parse_yield_expr(ParserState& state) {
    if (state.current().type != TokenType::YIELD) {
        return std::nullopt;
    }
    
    state.update_location();
    int line = state.start_lineno;
    int col = state.start_col_offset;
    state.advance();  // consume 'yield'
    
    // Check for 'yield from'
    if (state.current().type == TokenType::FROM) {
        state.advance();
        auto value = parse_expression(state);
        if (!value) {
            return std::nullopt;
        }
        return make_expr(std::make_shared<ast::YieldFrom>(*value, line, col));
    }
    
    // Regular yield
    ExprPtr value = nullptr;
    if (state.current().type != TokenType::NEWLINE &&
        state.current().type != TokenType::RPAR &&
        state.current().type != TokenType::RPAREN &&
        state.current().type != TokenType::RSQB &&
        state.current().type != TokenType::RBRACKET &&
        state.current().type != TokenType::RBRACE) {
        auto expr = parse_star_expressions(state);
        if (expr) {
            value = *expr;
        }
    }
    
    return make_expr(std::make_shared<ast::Yield>(value, line, col));
}

// ============================================================================
// Slice Expression
// ============================================================================

/**
 * slice[expr_ty]:
 *     | [expression] ':' [expression] [':' [expression]]
 */
inline ParseResult<ExprPtr> parse_slice(ParserState& state) {
    state.update_location();
    int line = state.start_lineno;
    int col = state.start_col_offset;
    
    ExprPtr lower = nullptr;
    ExprPtr upper = nullptr;
    ExprPtr step = nullptr;
    
    // Parse lower bound (optional)
    if (state.current().type != TokenType::COLON) {
        auto expr = parse_expression(state);
        if (expr) {
            lower = *expr;
        }
    }
    
    // First colon
    if (state.current().type != TokenType::COLON) {
        // Not a slice, just return the expression
        if (lower) {
            return ParseResult<ExprPtr>(lower);
        }
        return std::nullopt;
    }
    state.advance();
    
    // Parse upper bound (optional)
    if (state.current().type != TokenType::COLON &&
        state.current().type != TokenType::RSQB &&
        state.current().type != TokenType::RBRACKET &&
        state.current().type != TokenType::COMMA) {
        auto expr = parse_expression(state);
        if (expr) {
            upper = *expr;
        }
    }
    
    // Second colon (optional)
    if (state.current().type == TokenType::COLON) {
        state.advance();
        // Parse step (optional)
        if (state.current().type != TokenType::RSQB &&
            state.current().type != TokenType::RBRACKET &&
            state.current().type != TokenType::COMMA) {
            auto expr = parse_expression(state);
            if (expr) {
                step = *expr;
            }
        }
    }
    
    return make_expr(std::make_shared<ast::Slice>(lower, upper, step, line, col));
}

} // namespace grammar
} // namespace parser
} // namespace cpython_cpp

#endif // CPYTHON_CPP_GRAMMAR_ADVANCED_EXPRESSIONS_HPP
