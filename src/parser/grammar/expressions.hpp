#ifndef CPYTHON_CPP_GRAMMAR_EXPRESSIONS_HPP
#define CPYTHON_CPP_GRAMMAR_EXPRESSIONS_HPP

/**
 * Expression Grammar Rules - Template-based implementation
 * 
 * Follows CPython's Grammar/python.gram expression rules
 * Reference: CPython Grammar/python.gram lines 711-950
 */

#include "../peg_parser.hpp"
#include "../tokenizer_v2.hpp"
#include "../../ast/expr.hpp"
#include <memory>

namespace cpython_cpp {
namespace parser {
namespace grammar {

// Type alias for expression result
using ExprPtr = std::shared_ptr<ast::Expr>;

// Helper to convert derived AST types to ExprPtr
template<typename T>
inline ParseResult<ExprPtr> make_expr(std::shared_ptr<T> ptr) {
    return ParseResult<ExprPtr>(std::static_pointer_cast<ast::Expr>(ptr));
}

// ============================================================================
// Forward declarations - all parsers return ParseResult<ExprPtr>
// ============================================================================

// Expression parser declarations
inline ParseResult<ExprPtr> parse_expression(ParserState& state);
inline ParseResult<ExprPtr> parse_star_expressions(ParserState& state);
inline ParseResult<ExprPtr> parse_disjunction(ParserState& state);
inline ParseResult<ExprPtr> parse_conjunction(ParserState& state);
inline ParseResult<ExprPtr> parse_inversion(ParserState& state);
inline ParseResult<ExprPtr> parse_comparison(ParserState& state);
inline ParseResult<ExprPtr> parse_bitwise_or(ParserState& state);
inline ParseResult<ExprPtr> parse_bitwise_xor(ParserState& state);
inline ParseResult<ExprPtr> parse_bitwise_and(ParserState& state);
inline ParseResult<ExprPtr> parse_shift_expr(ParserState& state);
inline ParseResult<ExprPtr> parse_sum(ParserState& state);
inline ParseResult<ExprPtr> parse_term(ParserState& state);
inline ParseResult<ExprPtr> parse_factor(ParserState& state);
inline ParseResult<ExprPtr> parse_power(ParserState& state);
inline ParseResult<ExprPtr> parse_await_primary(ParserState& state);
inline ParseResult<ExprPtr> parse_primary(ParserState& state);
inline ParseResult<ExprPtr> parse_atom(ParserState& state);

// ============================================================================
// Atom - Lowest level expressions
// ============================================================================

inline ParseResult<ExprPtr> parse_atom(ParserState& state) {
    state.update_location();
    const Token& tok = state.current();
    int line = state.start_lineno;
    int col = state.start_col_offset;
    
    // NAME / IDENTIFIER
    if (tok.type == TokenType::IDENTIFIER) {
        state.advance();
        return make_expr(std::make_shared<ast::Name>(
            tok.value, ast::ExprContext::Load, line, col));
    }
    
    // True
    if (tok.type == TokenType::TRUE) {
        state.advance();
        return make_expr(std::make_shared<ast::Constant>("True", line, col));
    }
    
    // False
    if (tok.type == TokenType::FALSE) {
        state.advance();
        return make_expr(std::make_shared<ast::Constant>("False", line, col));
    }
    
    // None
    if (tok.type == TokenType::NONE) {
        state.advance();
        return make_expr(std::make_shared<ast::Constant>("None", line, col));
    }
    
    // NUMBER
    if (tok.type == TokenType::NUMBER) {
        state.advance();
        return make_expr(std::make_shared<ast::Constant>(tok.value, line, col));
    }
    
    // STRING
    if (tok.type == TokenType::STRING) {
        state.advance();
        return make_expr(std::make_shared<ast::Constant>(tok.value, line, col));
    }
    
    // Ellipsis (...)
    if (tok.type == TokenType::ELLIPSIS) {
        state.advance();
        return make_expr(std::make_shared<ast::Ellipsis>(line, col));
    }
    
    // Parenthesized expression, tuple, or generator
    if (tok.type == TokenType::LPAR || tok.type == TokenType::LPAREN) {
        size_t mark = state.mark();
        state.advance();  // consume '('
        
        // Empty tuple
        if (state.current().type == TokenType::RPAR || 
            state.current().type == TokenType::RPAREN) {
            state.advance();
            return make_expr(std::make_shared<ast::Tuple>(
                std::vector<ExprPtr>(), ast::ExprContext::Load, line, col));
        }
        
        // Parse first expression
        auto first = parse_expression(state);
        if (!first) {
            state.reset(mark);
            return std::nullopt;
        }
        
        // Check for tuple (comma after expression)
        if (state.current().type == TokenType::COMMA) {
            std::vector<ExprPtr> elts;
            elts.push_back(*first);
            
            while (state.current().type == TokenType::COMMA) {
                state.advance();  // consume ','
                if (state.current().type == TokenType::RPAR ||
                    state.current().type == TokenType::RPAREN) {
                    break;  // Trailing comma
                }
                auto elem = parse_expression(state);
                if (!elem) {
                    state.reset(mark);
                    return std::nullopt;
                }
                elts.push_back(*elem);
            }
            
            if (state.current().type != TokenType::RPAR &&
                state.current().type != TokenType::RPAREN) {
                state.reset(mark);
                return std::nullopt;
            }
            state.advance();
            
            return make_expr(std::make_shared<ast::Tuple>(
                elts, ast::ExprContext::Load, line, col));
        }
        
        // Single parenthesized expression
        if (state.current().type != TokenType::RPAR &&
            state.current().type != TokenType::RPAREN) {
            state.reset(mark);
            return std::nullopt;
        }
        state.advance();
        
        return first;
    }
    
    // List or list comprehension
    if (tok.type == TokenType::LSQB || tok.type == TokenType::LBRACKET) {
        size_t mark = state.mark();
        state.advance();  // consume '['
        
        // Empty list
        if (state.current().type == TokenType::RSQB ||
            state.current().type == TokenType::RBRACKET) {
            state.advance();
            return make_expr(std::make_shared<ast::List>(
                std::vector<ExprPtr>(), ast::ExprContext::Load, line, col));
        }
        
        // Parse elements
        std::vector<ExprPtr> elts;
        auto first = parse_expression(state);
        if (!first) {
            state.reset(mark);
            return std::nullopt;
        }
        elts.push_back(*first);
        
        while (state.current().type == TokenType::COMMA) {
            state.advance();
            if (state.current().type == TokenType::RSQB ||
                state.current().type == TokenType::RBRACKET) {
                break;  // Trailing comma
            }
            auto elem = parse_expression(state);
            if (!elem) {
                state.reset(mark);
                return std::nullopt;
            }
            elts.push_back(*elem);
        }
        
        if (state.current().type != TokenType::RSQB &&
            state.current().type != TokenType::RBRACKET) {
            state.reset(mark);
            return std::nullopt;
        }
        state.advance();
        
        return make_expr(std::make_shared<ast::List>(
            elts, ast::ExprContext::Load, line, col));
    }
    
    // Dict or set
    if (tok.type == TokenType::LBRACE) {
        size_t mark = state.mark();
        state.advance();  // consume '{'
        
        // Empty dict
        if (state.current().type == TokenType::RBRACE) {
            state.advance();
            return make_expr(std::make_shared<ast::Dict>(
                std::vector<ExprPtr>(), std::vector<ExprPtr>(), line, col));
        }
        
        // Parse first expression
        auto first = parse_expression(state);
        if (!first) {
            state.reset(mark);
            return std::nullopt;
        }
        
        // Check if dict (colon after key)
        if (state.current().type == TokenType::COLON) {
            state.advance();
            auto value = parse_expression(state);
            if (!value) {
                state.reset(mark);
                return std::nullopt;
            }
            
            std::vector<ExprPtr> keys, values;
            keys.push_back(*first);
            values.push_back(*value);
            
            while (state.current().type == TokenType::COMMA) {
                state.advance();
                if (state.current().type == TokenType::RBRACE) {
                    break;
                }
                auto key = parse_expression(state);
                if (!key) {
                    state.reset(mark);
                    return std::nullopt;
                }
                if (state.current().type != TokenType::COLON) {
                    state.reset(mark);
                    return std::nullopt;
                }
                state.advance();
                auto val = parse_expression(state);
                if (!val) {
                    state.reset(mark);
                    return std::nullopt;
                }
                keys.push_back(*key);
                values.push_back(*val);
            }
            
            if (state.current().type != TokenType::RBRACE) {
                state.reset(mark);
                return std::nullopt;
            }
            state.advance();
            
            return make_expr(std::make_shared<ast::Dict>(keys, values, line, col));
        }
        
        // Set literal
        std::vector<ExprPtr> elts;
        elts.push_back(*first);
        
        while (state.current().type == TokenType::COMMA) {
            state.advance();
            if (state.current().type == TokenType::RBRACE) {
                break;
            }
            auto elem = parse_expression(state);
            if (!elem) {
                state.reset(mark);
                return std::nullopt;
            }
            elts.push_back(*elem);
        }
        
        if (state.current().type != TokenType::RBRACE) {
            state.reset(mark);
            return std::nullopt;
        }
        state.advance();
        
        return make_expr(std::make_shared<ast::Set>(
            elts, ast::ExprContext::Load, line, col));
    }
    
    return std::nullopt;
}

// ============================================================================
// Primary - Atom with trailers (calls, subscripts, attributes)
// ============================================================================

inline ParseResult<ExprPtr> parse_primary(ParserState& state) {
    auto atom_result = parse_atom(state);
    if (!atom_result) {
        return std::nullopt;
    }
    
    ExprPtr result = *atom_result;
    
    // Parse trailers
    while (true) {
        state.update_location();
        int line = state.start_lineno;
        int col = state.start_col_offset;
        const Token& tok = state.current();
        
        // Attribute access: .NAME
        if (tok.type == TokenType::DOT) {
            state.advance();
            if (state.current().type != TokenType::IDENTIFIER) {
                return ParseResult<ExprPtr>(result);
            }
            std::string attr = state.current().value;
            state.advance();
            result = std::make_shared<ast::Attribute>(
                result, attr, ast::ExprContext::Load, line, col);
        }
        // Function call: (args)
        else if (tok.type == TokenType::LPAR || tok.type == TokenType::LPAREN) {
            state.advance();
            std::vector<ExprPtr> args;
            
            // Parse arguments
            if (state.current().type != TokenType::RPAR &&
                state.current().type != TokenType::RPAREN) {
                auto first_arg = parse_expression(state);
                if (first_arg) {
                    args.push_back(*first_arg);
                    while (state.current().type == TokenType::COMMA) {
                        state.advance();
                        if (state.current().type == TokenType::RPAR ||
                            state.current().type == TokenType::RPAREN) {
                            break;
                        }
                        auto arg = parse_expression(state);
                        if (arg) {
                            args.push_back(*arg);
                        }
                    }
                }
            }
            
            if (state.current().type != TokenType::RPAR &&
                state.current().type != TokenType::RPAREN) {
                return ParseResult<ExprPtr>(result);
            }
            state.advance();
            result = std::make_shared<ast::Call>(result, args, line, col);
        }
        // Subscript: [slice]
        else if (tok.type == TokenType::LSQB || tok.type == TokenType::LBRACKET) {
            state.advance();
            auto slice = parse_expression(state);
            if (!slice) {
                return ParseResult<ExprPtr>(result);
            }
            if (state.current().type != TokenType::RSQB &&
                state.current().type != TokenType::RBRACKET) {
                return ParseResult<ExprPtr>(result);
            }
            state.advance();
            result = std::make_shared<ast::Subscript>(
                result, *slice, ast::ExprContext::Load, line, col);
        }
        else {
            break;
        }
    }
    
    return ParseResult<ExprPtr>(result);
}

// ============================================================================
// Await Primary
// ============================================================================

inline ParseResult<ExprPtr> parse_await_primary(ParserState& state) {
    if (state.current().type == TokenType::AWAIT) {
        state.update_location();
        int line = state.start_lineno;
        int col = state.start_col_offset;
        state.advance();
        auto primary = parse_primary(state);
        if (!primary) {
            return std::nullopt;
        }
        return make_expr(std::make_shared<ast::Await>(*primary, line, col));
    }
    return parse_primary(state);
}

// ============================================================================
// Power - Exponentiation
// ============================================================================

inline ParseResult<ExprPtr> parse_power(ParserState& state) {
    auto left = parse_await_primary(state);
    if (!left) {
        return std::nullopt;
    }
    
    if (state.current().type == TokenType::DOUBLESTAR ||
        state.current().type == TokenType::POWER) {
        state.update_location();
        int line = state.start_lineno;
        int col = state.start_col_offset;
        state.advance();
        auto right = parse_factor(state);  // Right-associative
        if (!right) {
            return std::nullopt;
        }
        return make_expr(std::make_shared<ast::BinOp>(
            *left, ast::Operator::Pow, *right, line, col));
    }
    
    return left;
}

// ============================================================================
// Factor - Unary operators
// ============================================================================

inline ParseResult<ExprPtr> parse_factor(ParserState& state) {
    state.update_location();
    int line = state.start_lineno;
    int col = state.start_col_offset;
    const Token& tok = state.current();
    
    if (tok.type == TokenType::PLUS) {
        state.advance();
        auto operand = parse_factor(state);
        if (!operand) {
            return std::nullopt;
        }
        return make_expr(std::make_shared<ast::UnaryOp>(
            ast::UnaryOp::UnaryOpType::UAdd, *operand, line, col));
    }
    
    if (tok.type == TokenType::MINUS) {
        state.advance();
        auto operand = parse_factor(state);
        if (!operand) {
            return std::nullopt;
        }
        return make_expr(std::make_shared<ast::UnaryOp>(
            ast::UnaryOp::UnaryOpType::USub, *operand, line, col));
    }
    
    if (tok.type == TokenType::TILDE || tok.type == TokenType::BIT_NOT) {
        state.advance();
        auto operand = parse_factor(state);
        if (!operand) {
            return std::nullopt;
        }
        return make_expr(std::make_shared<ast::UnaryOp>(
            ast::UnaryOp::UnaryOpType::Invert, *operand, line, col));
    }
    
    return parse_power(state);
}

// ============================================================================
// Term - Multiplication, division, modulo
// ============================================================================

inline ParseResult<ExprPtr> parse_term(ParserState& state) {
    auto left = parse_factor(state);
    if (!left) {
        return std::nullopt;
    }
    
    while (true) {
        state.update_location();
        int line = state.start_lineno;
        int col = state.start_col_offset;
        ast::Operator op;
        bool matched = false;
        
        switch (state.current().type) {
            case TokenType::STAR:
                op = ast::Operator::Mult;
                matched = true;
                break;
            case TokenType::SLASH:
                op = ast::Operator::Div;
                matched = true;
                break;
            case TokenType::DOUBLESLASH:  // FLOOR_DIV is same value
                op = ast::Operator::FloorDiv;
                matched = true;
                break;
            case TokenType::PERCENT:
                op = ast::Operator::Mod;
                matched = true;
                break;
            case TokenType::AT:
                op = ast::Operator::MatMult;
                matched = true;
                break;
            default:
                break;
        }
        
        if (!matched) {
            break;
        }
        
        state.advance();
        auto right = parse_factor(state);
        if (!right) {
            return std::nullopt;
        }
        left = ParseResult<ExprPtr>(std::make_shared<ast::BinOp>(
            *left, op, *right, line, col));
    }
    
    return left;
}

// ============================================================================
// Sum - Addition and subtraction
// ============================================================================

inline ParseResult<ExprPtr> parse_sum(ParserState& state) {
    auto left = parse_term(state);
    if (!left) {
        return std::nullopt;
    }
    
    while (true) {
        state.update_location();
        int line = state.start_lineno;
        int col = state.start_col_offset;
        ast::Operator op;
        
        if (state.current().type == TokenType::PLUS) {
            op = ast::Operator::Add;
        } else if (state.current().type == TokenType::MINUS) {
            op = ast::Operator::Sub;
        } else {
            break;
        }
        
        state.advance();
        auto right = parse_term(state);
        if (!right) {
            return std::nullopt;
        }
        left = ParseResult<ExprPtr>(std::make_shared<ast::BinOp>(
            *left, op, *right, line, col));
    }
    
    return left;
}

// ============================================================================
// Shift Expression
// ============================================================================

inline ParseResult<ExprPtr> parse_shift_expr(ParserState& state) {
    auto left = parse_sum(state);
    if (!left) {
        return std::nullopt;
    }
    
    while (true) {
        state.update_location();
        int line = state.start_lineno;
        int col = state.start_col_offset;
        ast::Operator op;
        
        if (state.current().type == TokenType::LEFTSHIFT ||
            state.current().type == TokenType::LEFT_SHIFT) {
            op = ast::Operator::LShift;
        } else if (state.current().type == TokenType::RIGHTSHIFT ||
                   state.current().type == TokenType::RIGHT_SHIFT) {
            op = ast::Operator::RShift;
        } else {
            break;
        }
        
        state.advance();
        auto right = parse_sum(state);
        if (!right) {
            return std::nullopt;
        }
        left = ParseResult<ExprPtr>(std::make_shared<ast::BinOp>(
            *left, op, *right, line, col));
    }
    
    return left;
}

// ============================================================================
// Bitwise AND, XOR, OR
// ============================================================================

inline ParseResult<ExprPtr> parse_bitwise_and(ParserState& state) {
    auto left = parse_shift_expr(state);
    if (!left) {
        return std::nullopt;
    }
    
    while (state.current().type == TokenType::AMPER ||
           state.current().type == TokenType::BIT_AND) {
        state.update_location();
        int line = state.start_lineno;
        int col = state.start_col_offset;
        state.advance();
        auto right = parse_shift_expr(state);
        if (!right) {
            return std::nullopt;
        }
        left = ParseResult<ExprPtr>(std::make_shared<ast::BinOp>(
            *left, ast::Operator::BitAnd, *right, line, col));
    }
    
    return left;
}

inline ParseResult<ExprPtr> parse_bitwise_xor(ParserState& state) {
    auto left = parse_bitwise_and(state);
    if (!left) {
        return std::nullopt;
    }
    
    while (state.current().type == TokenType::CIRCUMFLEX ||
           state.current().type == TokenType::BIT_XOR) {
        state.update_location();
        int line = state.start_lineno;
        int col = state.start_col_offset;
        state.advance();
        auto right = parse_bitwise_and(state);
        if (!right) {
            return std::nullopt;
        }
        left = ParseResult<ExprPtr>(std::make_shared<ast::BinOp>(
            *left, ast::Operator::BitXor, *right, line, col));
    }
    
    return left;
}

inline ParseResult<ExprPtr> parse_bitwise_or(ParserState& state) {
    auto left = parse_bitwise_xor(state);
    if (!left) {
        return std::nullopt;
    }
    
    while (state.current().type == TokenType::VBAR ||
           state.current().type == TokenType::BIT_OR) {
        state.update_location();
        int line = state.start_lineno;
        int col = state.start_col_offset;
        state.advance();
        auto right = parse_bitwise_xor(state);
        if (!right) {
            return std::nullopt;
        }
        left = ParseResult<ExprPtr>(std::make_shared<ast::BinOp>(
            *left, ast::Operator::BitOr, *right, line, col));
    }
    
    return left;
}

// ============================================================================
// Comparison
// ============================================================================

inline ParseResult<ExprPtr> parse_comparison(ParserState& state) {
    auto left = parse_bitwise_or(state);
    if (!left) {
        return std::nullopt;
    }
    
    // Check for comparison operator
    ast::CompareOp op;
    bool has_comparison = false;
    
    switch (state.current().type) {
        case TokenType::LESS:
            op = ast::CompareOp::Lt;
            has_comparison = true;
            break;
        case TokenType::GREATER:
            op = ast::CompareOp::Gt;
            has_comparison = true;
            break;
        case TokenType::EQEQUAL:  // EQUAL_EQUAL is same value
            op = ast::CompareOp::Eq;
            has_comparison = true;
            break;
        case TokenType::NOTEQUAL:  // NOT_EQUAL is same value
            op = ast::CompareOp::NotEq;
            has_comparison = true;
            break;
        case TokenType::LESSEQUAL:  // LESS_EQUAL is same value
            op = ast::CompareOp::LtE;
            has_comparison = true;
            break;
        case TokenType::GREATEREQUAL:  // GREATER_EQUAL is same value
            op = ast::CompareOp::GtE;
            has_comparison = true;
            break;
        case TokenType::IN:
            op = ast::CompareOp::In;
            has_comparison = true;
            break;
        case TokenType::NOT:
            if (state.peek().type == TokenType::IN) {
                state.advance();  // consume 'not'
                op = ast::CompareOp::NotIn;
                has_comparison = true;
            }
            break;
        case TokenType::IS:
            if (state.peek().type == TokenType::NOT) {
                state.advance();  // consume 'is'
                op = ast::CompareOp::IsNot;
                has_comparison = true;
            } else {
                op = ast::CompareOp::Is;
                has_comparison = true;
            }
            break;
        default:
            break;
    }
    
    if (!has_comparison) {
        return left;
    }
    
    state.update_location();
    int line = state.start_lineno;
    int col = state.start_col_offset;
    state.advance();
    auto right = parse_bitwise_or(state);
    if (!right) {
        return std::nullopt;
    }
    
    return make_expr(std::make_shared<ast::Compare>(*left, op, *right, line, col));
}

// ============================================================================
// Inversion (not)
// ============================================================================

inline ParseResult<ExprPtr> parse_inversion(ParserState& state) {
    if (state.current().type == TokenType::NOT) {
        state.update_location();
        int line = state.start_lineno;
        int col = state.start_col_offset;
        state.advance();
        auto operand = parse_inversion(state);
        if (!operand) {
            return std::nullopt;
        }
        return make_expr(std::make_shared<ast::UnaryOp>(
            ast::UnaryOp::UnaryOpType::Not, *operand, line, col));
    }
    return parse_comparison(state);
}

// ============================================================================
// Conjunction (and)
// ============================================================================

inline ParseResult<ExprPtr> parse_conjunction(ParserState& state) {
    auto first = parse_inversion(state);
    if (!first) {
        return std::nullopt;
    }
    
    if (state.current().type != TokenType::AND) {
        return first;
    }
    
    state.update_location();
    int line = state.start_lineno;
    int col = state.start_col_offset;
    
    std::vector<ExprPtr> values;
    values.push_back(*first);
    
    while (state.current().type == TokenType::AND) {
        state.advance();
        auto next = parse_inversion(state);
        if (!next) {
            return std::nullopt;
        }
        values.push_back(*next);
    }
    
    return make_expr(std::make_shared<ast::BoolOpExpr>(
        ast::BoolOp::And, values, line, col));
}

// ============================================================================
// Disjunction (or)
// ============================================================================

inline ParseResult<ExprPtr> parse_disjunction(ParserState& state) {
    auto first = parse_conjunction(state);
    if (!first) {
        return std::nullopt;
    }
    
    if (state.current().type != TokenType::OR) {
        return first;
    }
    
    state.update_location();
    int line = state.start_lineno;
    int col = state.start_col_offset;
    
    std::vector<ExprPtr> values;
    values.push_back(*first);
    
    while (state.current().type == TokenType::OR) {
        state.advance();
        auto next = parse_conjunction(state);
        if (!next) {
            return std::nullopt;
        }
        values.push_back(*next);
    }
    
    return make_expr(std::make_shared<ast::BoolOpExpr>(
        ast::BoolOp::Or, values, line, col));
}

// ============================================================================
// Expression - Top level
// ============================================================================

inline ParseResult<ExprPtr> parse_expression(ParserState& state) {
    auto condition = parse_disjunction(state);
    if (!condition) {
        return std::nullopt;
    }
    
    // Check for conditional expression: x if condition else y
    if (state.current().type == TokenType::IF) {
        state.update_location();
        int line = state.start_lineno;
        int col = state.start_col_offset;
        state.advance();
        auto test = parse_disjunction(state);
        if (!test) {
            return std::nullopt;
        }
        if (state.current().type != TokenType::ELSE) {
            return std::nullopt;
        }
        state.advance();
        auto orelse = parse_expression(state);
        if (!orelse) {
            return std::nullopt;
        }
        return make_expr(std::make_shared<ast::IfExp>(*test, *condition, *orelse, line, col));
    }
    
    return condition;
}

// ============================================================================
// Star Expressions (for assignments and function calls)
// ============================================================================

inline ParseResult<ExprPtr> parse_star_expressions(ParserState& state) {
    auto first = parse_expression(state);
    if (!first) {
        return std::nullopt;
    }
    
    if (state.current().type != TokenType::COMMA) {
        return first;
    }
    
    state.update_location();
    int line = state.start_lineno;
    int col = state.start_col_offset;
    
    std::vector<ExprPtr> elts;
    elts.push_back(*first);
    
    while (state.current().type == TokenType::COMMA) {
        state.advance();
        // Check for trailing comma
        if (state.current().type == TokenType::NEWLINE ||
            state.current().type == TokenType::COLON ||
            state.current().type == TokenType::RPAR ||
            state.current().type == TokenType::RPAREN ||
            state.current().type == TokenType::RSQB ||
            state.current().type == TokenType::RBRACKET ||
            state.current().type == TokenType::RBRACE) {
            break;
        }
        auto next = parse_expression(state);
        if (!next) {
            break;
        }
        elts.push_back(*next);
    }
    
    return make_expr(std::make_shared<ast::Tuple>(
        elts, ast::ExprContext::Load, line, col));
}

// ============================================================================
// Wrapper structs for compatibility with template-based approach
// ============================================================================

struct Expression {
    static ParseResult<ExprPtr> parse(ParserState& state) {
        return parse_expression(state);
    }
};

struct StarExpressions {
    static ParseResult<ExprPtr> parse(ParserState& state) {
        return parse_star_expressions(state);
    }
};

} // namespace grammar
} // namespace parser
} // namespace cpython_cpp

#endif // CPYTHON_CPP_GRAMMAR_EXPRESSIONS_HPP
