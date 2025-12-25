#ifndef CPYTHON_CPP_GRAMMAR_STATEMENTS_HPP
#define CPYTHON_CPP_GRAMMAR_STATEMENTS_HPP

/**
 * Statement Grammar Rules - Template-based implementation
 * 
 * Follows CPython's Grammar/python.gram statement rules
 * Reference: CPython Grammar/python.gram lines 80-710
 */

#include "../peg_parser.hpp"
#include "../tokenizer_v2.hpp"
#include "expressions.hpp"
#include "../../ast/stmt.hpp"
#include <memory>

namespace cpython_cpp {
namespace parser {
namespace grammar {

// Type aliases
using StmtPtr = std::shared_ptr<ast::Stmt>;
using StmtList = std::vector<StmtPtr>;
using ExprPtr = std::shared_ptr<ast::Expr>;

// Helper to convert derived AST types to StmtPtr
template<typename T>
inline ParseResult<StmtPtr> make_stmt(std::shared_ptr<T> ptr) {
    return ParseResult<StmtPtr>(std::static_pointer_cast<ast::Stmt>(ptr));
}

// ============================================================================
// Forward declarations
// ============================================================================

inline ParseResult<StmtPtr> parse_statement(ParserState& state);
inline ParseResult<StmtPtr> parse_simple_stmt(ParserState& state);
inline ParseResult<StmtPtr> parse_compound_stmt(ParserState& state);
inline ParseResult<StmtList> parse_block(ParserState& state);

// ============================================================================
// Block - Indented statement block
// ============================================================================

inline ParseResult<StmtList> parse_block(ParserState& state) {
    // Skip newlines
    while (state.current().type == TokenType::NEWLINE) {
        state.advance();
    }
    
    // Check for INDENT
    if (state.current().type == TokenType::INDENT) {
        state.advance();
        
        StmtList stmts;
        while (state.current().type != TokenType::DEDENT &&
               state.current().type != TokenType::ENDMARKER) {
            // Skip newlines
            while (state.current().type == TokenType::NEWLINE) {
                state.advance();
            }
            if (state.current().type == TokenType::DEDENT ||
                state.current().type == TokenType::ENDMARKER) {
                break;
            }
            
            auto stmt = parse_statement(state);
            if (stmt) {
                stmts.push_back(*stmt);
            } else {
                break;
            }
        }
        
        if (state.current().type == TokenType::DEDENT) {
            state.advance();
        }
        
        return ParseResult<StmtList>(stmts);
    }
    
    // Simple statement on same line
    auto stmt = parse_simple_stmt(state);
    if (stmt) {
        return ParseResult<StmtList>(StmtList{*stmt});
    }
    
    return std::nullopt;
}

// ============================================================================
// Simple Statements
// ============================================================================

inline ParseResult<StmtPtr> parse_pass_stmt(ParserState& state) {
    if (state.current().type != TokenType::PASS) {
        return std::nullopt;
    }
    state.update_location();
    int line = state.start_lineno;
    int col = state.start_col_offset;
    state.advance();
    return make_stmt(std::make_shared<ast::Pass>(line, col));
}

inline ParseResult<StmtPtr> parse_break_stmt(ParserState& state) {
    if (state.current().type != TokenType::BREAK) {
        return std::nullopt;
    }
    state.update_location();
    int line = state.start_lineno;
    int col = state.start_col_offset;
    state.advance();
    return make_stmt(std::make_shared<ast::Break>(line, col));
}

inline ParseResult<StmtPtr> parse_continue_stmt(ParserState& state) {
    if (state.current().type != TokenType::CONTINUE) {
        return std::nullopt;
    }
    state.update_location();
    int line = state.start_lineno;
    int col = state.start_col_offset;
    state.advance();
    return make_stmt(std::make_shared<ast::Continue>(line, col));
}

inline ParseResult<StmtPtr> parse_return_stmt(ParserState& state) {
    if (state.current().type != TokenType::RETURN) {
        return std::nullopt;
    }
    state.update_location();
    int line = state.start_lineno;
    int col = state.start_col_offset;
    state.advance();
    
    ExprPtr value = nullptr;
    if (state.current().type != TokenType::NEWLINE &&
        state.current().type != TokenType::ENDMARKER) {
        auto expr = parse_star_expressions(state);
        if (expr) {
            value = *expr;
        }
    }
    
    return make_stmt(std::make_shared<ast::Return>(value, line, col));
}

inline ParseResult<StmtPtr> parse_raise_stmt(ParserState& state) {
    if (state.current().type != TokenType::RAISE) {
        return std::nullopt;
    }
    state.update_location();
    int line = state.start_lineno;
    int col = state.start_col_offset;
    state.advance();
    
    ExprPtr exc = nullptr;
    ExprPtr cause = nullptr;
    
    if (state.current().type != TokenType::NEWLINE &&
        state.current().type != TokenType::ENDMARKER) {
        auto expr = parse_expression(state);
        if (expr) {
            exc = *expr;
            if (state.current().type == TokenType::FROM) {
                state.advance();
                auto from_expr = parse_expression(state);
                if (from_expr) {
                    cause = *from_expr;
                }
            }
        }
    }
    
    return make_stmt(std::make_shared<ast::Raise>(exc, cause, line, col));
}

inline ParseResult<StmtPtr> parse_del_stmt(ParserState& state) {
    if (state.current().type != TokenType::DEL) {
        return std::nullopt;
    }
    state.update_location();
    int line = state.start_lineno;
    int col = state.start_col_offset;
    state.advance();
    
    std::vector<ExprPtr> targets;
    auto target = parse_expression(state);
    if (target) {
        targets.push_back(*target);
        while (state.current().type == TokenType::COMMA) {
            state.advance();
            auto next = parse_expression(state);
            if (next) {
                targets.push_back(*next);
            }
        }
    }
    
    return make_stmt(std::make_shared<ast::Delete>(targets, line, col));
}

inline ParseResult<StmtPtr> parse_assert_stmt(ParserState& state) {
    if (state.current().type != TokenType::ASSERT) {
        return std::nullopt;
    }
    state.update_location();
    int line = state.start_lineno;
    int col = state.start_col_offset;
    state.advance();
    
    auto test = parse_expression(state);
    if (!test) {
        return std::nullopt;
    }
    
    ExprPtr msg = nullptr;
    if (state.current().type == TokenType::COMMA) {
        state.advance();
        auto msg_expr = parse_expression(state);
        if (msg_expr) {
            msg = *msg_expr;
        }
    }
    
    return make_stmt(std::make_shared<ast::Assert>(*test, msg, line, col));
}

inline ParseResult<StmtPtr> parse_global_stmt(ParserState& state) {
    if (state.current().type != TokenType::GLOBAL) {
        return std::nullopt;
    }
    state.update_location();
    int line = state.start_lineno;
    int col = state.start_col_offset;
    state.advance();
    
    std::vector<std::string> names;
    if (state.current().type == TokenType::IDENTIFIER) {
        names.push_back(state.current().value);
        state.advance();
        while (state.current().type == TokenType::COMMA) {
            state.advance();
            if (state.current().type == TokenType::IDENTIFIER) {
                names.push_back(state.current().value);
                state.advance();
            }
        }
    }
    
    return make_stmt(std::make_shared<ast::Global>(names, line, col));
}

inline ParseResult<StmtPtr> parse_nonlocal_stmt(ParserState& state) {
    if (state.current().type != TokenType::NONLOCAL) {
        return std::nullopt;
    }
    state.update_location();
    int line = state.start_lineno;
    int col = state.start_col_offset;
    state.advance();
    
    std::vector<std::string> names;
    if (state.current().type == TokenType::IDENTIFIER) {
        names.push_back(state.current().value);
        state.advance();
        while (state.current().type == TokenType::COMMA) {
            state.advance();
            if (state.current().type == TokenType::IDENTIFIER) {
                names.push_back(state.current().value);
                state.advance();
            }
        }
    }
    
    return make_stmt(std::make_shared<ast::Nonlocal>(names, line, col));
}

inline ParseResult<StmtPtr> parse_assignment(ParserState& state) {
    size_t mark = state.mark();
    state.update_location();
    int line = state.start_lineno;
    int col = state.start_col_offset;
    
    // Try to parse target expression
    auto target = parse_expression(state);
    if (!target) {
        return std::nullopt;
    }
    
    // Check for annotated assignment: NAME ':' expression
    if (state.current().type == TokenType::COLON) {
        state.advance();
        auto annotation = parse_expression(state);
        if (!annotation) {
            state.reset(mark);
            return std::nullopt;
        }
        
        ExprPtr value = nullptr;
        if (state.current().type == TokenType::EQUAL) {
            state.advance();
            auto val = parse_expression(state);
            if (val) {
                value = *val;
            }
        }
        
        return make_stmt(std::make_shared<ast::AnnAssign>(
            *target, *annotation, value, true, line, col));
    }
    
    // Check for augmented assignment
    ast::Operator aug_op;
    bool is_augmented = false;
    
    switch (state.current().type) {
        case TokenType::PLUSEQUAL:
            aug_op = ast::Operator::Add;
            is_augmented = true;
            break;
        case TokenType::MINEQUAL:
            aug_op = ast::Operator::Sub;
            is_augmented = true;
            break;
        case TokenType::STAREQUAL:
            aug_op = ast::Operator::Mult;
            is_augmented = true;
            break;
        case TokenType::SLASHEQUAL:
            aug_op = ast::Operator::Div;
            is_augmented = true;
            break;
        case TokenType::PERCENTEQUAL:
            aug_op = ast::Operator::Mod;
            is_augmented = true;
            break;
        case TokenType::DOUBLESTAREQUAL:
            aug_op = ast::Operator::Pow;
            is_augmented = true;
            break;
        case TokenType::DOUBLESLASHEQUAL:
            aug_op = ast::Operator::FloorDiv;
            is_augmented = true;
            break;
        case TokenType::AMPEREQUAL:
            aug_op = ast::Operator::BitAnd;
            is_augmented = true;
            break;
        case TokenType::VBAREQUAL:
            aug_op = ast::Operator::BitOr;
            is_augmented = true;
            break;
        case TokenType::CIRCUMFLEXEQUAL:
            aug_op = ast::Operator::BitXor;
            is_augmented = true;
            break;
        case TokenType::LEFTSHIFTEQUAL:
            aug_op = ast::Operator::LShift;
            is_augmented = true;
            break;
        case TokenType::RIGHTSHIFTEQUAL:
            aug_op = ast::Operator::RShift;
            is_augmented = true;
            break;
        case TokenType::ATEQUAL:
            aug_op = ast::Operator::MatMult;
            is_augmented = true;
            break;
        default:
            break;
    }
    
    if (is_augmented) {
        state.advance();
        auto value = parse_expression(state);
        if (!value) {
            state.reset(mark);
            return std::nullopt;
        }
        return make_stmt(std::make_shared<ast::AugAssign>(*target, aug_op, *value, line, col));
    }
    
    // Regular assignment: targets '=' value
    if (state.current().type == TokenType::EQUAL) {
        std::vector<ExprPtr> targets;
        targets.push_back(*target);
        
        while (state.current().type == TokenType::EQUAL) {
            state.advance();
            auto next = parse_expression(state);
            if (!next) {
                state.reset(mark);
                return std::nullopt;
            }
            // Check if this is another target or the final value
            if (state.current().type == TokenType::EQUAL) {
                targets.push_back(*next);
            } else {
                // This is the value
                return make_stmt(std::make_shared<ast::Assign>(targets, *next, line, col));
            }
        }
    }
    
    // Not an assignment - this is an expression statement
    state.reset(mark);
    return std::nullopt;
}

inline ParseResult<StmtPtr> parse_simple_stmt(ParserState& state) {
    // Use lookahead for efficiency
    switch (state.current().type) {
        case TokenType::RETURN:
            return parse_return_stmt(state);
        case TokenType::RAISE:
            return parse_raise_stmt(state);
        case TokenType::PASS:
            return parse_pass_stmt(state);
        case TokenType::DEL:
            return parse_del_stmt(state);
        case TokenType::ASSERT:
            return parse_assert_stmt(state);
        case TokenType::BREAK:
            return parse_break_stmt(state);
        case TokenType::CONTINUE:
            return parse_continue_stmt(state);
        case TokenType::GLOBAL:
            return parse_global_stmt(state);
        case TokenType::NONLOCAL:
            return parse_nonlocal_stmt(state);
        default:
            break;
    }
    
    // Try assignment first
    auto assign = parse_assignment(state);
    if (assign) {
        return assign;
    }
    
    // Expression statement
    state.update_location();
    int line = state.start_lineno;
    int col = state.start_col_offset;
    auto expr = parse_star_expressions(state);
    if (expr) {
        return make_stmt(std::make_shared<ast::ExprStmt>(*expr, line, col));
    }
    
    return std::nullopt;
}

// ============================================================================
// Compound Statements
// ============================================================================

inline ParseResult<StmtPtr> parse_if_stmt(ParserState& state);

inline ParseResult<StmtPtr> parse_elif_stmt(ParserState& state) {
    if (state.current().type != TokenType::ELIF) {
        return std::nullopt;
    }
    state.update_location();
    int line = state.start_lineno;
    int col = state.start_col_offset;
    state.advance();
    
    auto test = parse_expression(state);
    if (!test) {
        return std::nullopt;
    }
    
    if (state.current().type != TokenType::COLON) {
        return std::nullopt;
    }
    state.advance();
    
    auto body = parse_block(state);
    if (!body) {
        return std::nullopt;
    }
    
    StmtList orelse;
    
    // Check for another elif
    if (state.current().type == TokenType::ELIF) {
        auto elif = parse_elif_stmt(state);
        if (elif) {
            orelse.push_back(*elif);
        }
    }
    // Check for else
    else if (state.current().type == TokenType::ELSE) {
        state.advance();
        if (state.current().type != TokenType::COLON) {
            return std::nullopt;
        }
        state.advance();
        auto else_body = parse_block(state);
        if (else_body) {
            orelse = *else_body;
        }
    }
    
    return make_stmt(std::make_shared<ast::If>(*test, *body, orelse, line, col));
}

inline ParseResult<StmtPtr> parse_if_stmt(ParserState& state) {
    if (state.current().type != TokenType::IF) {
        return std::nullopt;
    }
    state.update_location();
    int line = state.start_lineno;
    int col = state.start_col_offset;
    state.advance();
    
    auto test = parse_expression(state);
    if (!test) {
        return std::nullopt;
    }
    
    if (state.current().type != TokenType::COLON) {
        return std::nullopt;
    }
    state.advance();
    
    auto body = parse_block(state);
    if (!body) {
        return std::nullopt;
    }
    
    StmtList orelse;
    
    // Check for elif
    if (state.current().type == TokenType::ELIF) {
        auto elif = parse_elif_stmt(state);
        if (elif) {
            orelse.push_back(*elif);
        }
    }
    // Check for else
    else if (state.current().type == TokenType::ELSE) {
        state.advance();
        if (state.current().type != TokenType::COLON) {
            return std::nullopt;
        }
        state.advance();
        auto else_body = parse_block(state);
        if (else_body) {
            orelse = *else_body;
        }
    }
    
    return make_stmt(std::make_shared<ast::If>(*test, *body, orelse, line, col));
}

inline ParseResult<StmtPtr> parse_while_stmt(ParserState& state) {
    if (state.current().type != TokenType::WHILE) {
        return std::nullopt;
    }
    state.update_location();
    int line = state.start_lineno;
    int col = state.start_col_offset;
    state.advance();
    
    auto test = parse_expression(state);
    if (!test) {
        return std::nullopt;
    }
    
    if (state.current().type != TokenType::COLON) {
        return std::nullopt;
    }
    state.advance();
    
    auto body = parse_block(state);
    if (!body) {
        return std::nullopt;
    }
    
    StmtList orelse;
    if (state.current().type == TokenType::ELSE) {
        state.advance();
        if (state.current().type != TokenType::COLON) {
            return std::nullopt;
        }
        state.advance();
        auto else_body = parse_block(state);
        if (else_body) {
            orelse = *else_body;
        }
    }
    
    return make_stmt(std::make_shared<ast::While>(*test, *body, orelse, line, col));
}

inline ParseResult<StmtPtr> parse_for_stmt(ParserState& state) {
    if (state.current().type != TokenType::FOR) {
        return std::nullopt;
    }
    state.update_location();
    int line = state.start_lineno;
    int col = state.start_col_offset;
    state.advance();
    
    auto target = parse_expression(state);
    if (!target) {
        return std::nullopt;
    }
    
    if (state.current().type != TokenType::IN) {
        return std::nullopt;
    }
    state.advance();
    
    auto iter = parse_star_expressions(state);
    if (!iter) {
        return std::nullopt;
    }
    
    if (state.current().type != TokenType::COLON) {
        return std::nullopt;
    }
    state.advance();
    
    auto body = parse_block(state);
    if (!body) {
        return std::nullopt;
    }
    
    StmtList orelse;
    if (state.current().type == TokenType::ELSE) {
        state.advance();
        if (state.current().type != TokenType::COLON) {
            return std::nullopt;
        }
        state.advance();
        auto else_body = parse_block(state);
        if (else_body) {
            orelse = *else_body;
        }
    }
    
    return make_stmt(std::make_shared<ast::For>(*target, *iter, *body, orelse, line, col));
}

inline ParseResult<StmtPtr> parse_function_def(ParserState& state) {
    std::vector<ExprPtr> decorators;
    
    // Parse decorators
    while (state.current().type == TokenType::AT) {
        state.advance();
        auto dec = parse_expression(state);
        if (dec) {
            decorators.push_back(*dec);
        }
        // Skip newline after decorator
        while (state.current().type == TokenType::NEWLINE) {
            state.advance();
        }
    }
    
    // Check for async
    bool is_async = false;
    if (state.current().type == TokenType::ASYNC) {
        is_async = true;
        state.advance();
    }
    
    if (state.current().type != TokenType::DEF) {
        return std::nullopt;
    }
    state.update_location();
    int line = state.start_lineno;
    int col = state.start_col_offset;
    state.advance();
    
    // Function name
    if (state.current().type != TokenType::IDENTIFIER) {
        return std::nullopt;
    }
    std::string name = state.current().value;
    state.advance();
    
    // Parameters
    if (state.current().type != TokenType::LPAR &&
        state.current().type != TokenType::LPAREN) {
        return std::nullopt;
    }
    state.advance();
    
    std::vector<ast::arg> args;
    while (state.current().type != TokenType::RPAR &&
           state.current().type != TokenType::RPAREN) {
        if (state.current().type == TokenType::IDENTIFIER) {
            std::string arg_name = state.current().value;
            state.advance();
            
            // Check for annotation
            ExprPtr annotation = nullptr;
            if (state.current().type == TokenType::COLON) {
                state.advance();
                auto ann = parse_expression(state);
                if (ann) {
                    annotation = *ann;
                }
            }
            
            // Check for default value
            ExprPtr default_val = nullptr;
            if (state.current().type == TokenType::EQUAL) {
                state.advance();
                auto def = parse_expression(state);
                if (def) {
                    default_val = *def;
                }
            }
            
            args.push_back(ast::arg(arg_name, annotation));
            
            if (state.current().type == TokenType::COMMA) {
                state.advance();
            }
        } else {
            break;
        }
    }
    
    if (state.current().type != TokenType::RPAR &&
        state.current().type != TokenType::RPAREN) {
        return std::nullopt;
    }
    state.advance();
    
    // Return annotation
    ExprPtr returns = nullptr;
    if (state.current().type == TokenType::ARROW) {
        state.advance();
        auto ret = parse_expression(state);
        if (ret) {
            returns = *ret;
        }
    }
    
    if (state.current().type != TokenType::COLON) {
        return std::nullopt;
    }
    state.advance();
    
    auto body = parse_block(state);
    if (!body) {
        return std::nullopt;
    }
    
    if (is_async) {
        return make_stmt(std::make_shared<ast::AsyncFunctionDef>(
            name, args, *body, decorators, returns, line, col));
    }
    
    return make_stmt(std::make_shared<ast::FunctionDef>(
        name, args, *body, decorators, returns, line, col));
}

inline ParseResult<StmtPtr> parse_class_def(ParserState& state) {
    std::vector<ExprPtr> decorators;
    
    // Parse decorators
    while (state.current().type == TokenType::AT) {
        state.advance();
        auto dec = parse_expression(state);
        if (dec) {
            decorators.push_back(*dec);
        }
        while (state.current().type == TokenType::NEWLINE) {
            state.advance();
        }
    }
    
    if (state.current().type != TokenType::CLASS) {
        return std::nullopt;
    }
    state.update_location();
    int line = state.start_lineno;
    int col = state.start_col_offset;
    state.advance();
    
    // Class name
    if (state.current().type != TokenType::IDENTIFIER) {
        return std::nullopt;
    }
    std::string name = state.current().value;
    state.advance();
    
    // Base classes
    std::vector<ExprPtr> bases;
    if (state.current().type == TokenType::LPAR ||
        state.current().type == TokenType::LPAREN) {
        state.advance();
        while (state.current().type != TokenType::RPAR &&
               state.current().type != TokenType::RPAREN) {
            auto base = parse_expression(state);
            if (base) {
                bases.push_back(*base);
            }
            if (state.current().type == TokenType::COMMA) {
                state.advance();
            } else {
                break;
            }
        }
        if (state.current().type == TokenType::RPAR ||
            state.current().type == TokenType::RPAREN) {
            state.advance();
        }
    }
    
    if (state.current().type != TokenType::COLON) {
        return std::nullopt;
    }
    state.advance();
    
    auto body = parse_block(state);
    if (!body) {
        return std::nullopt;
    }
    
    return make_stmt(std::make_shared<ast::ClassDef>(
        name, bases, *body, decorators, line, col));
}

inline ParseResult<StmtPtr> parse_try_stmt(ParserState& state) {
    if (state.current().type != TokenType::TRY) {
        return std::nullopt;
    }
    state.update_location();
    int line = state.start_lineno;
    int col = state.start_col_offset;
    state.advance();
    
    if (state.current().type != TokenType::COLON) {
        return std::nullopt;
    }
    state.advance();
    
    auto body = parse_block(state);
    if (!body) {
        return std::nullopt;
    }
    
    std::vector<std::shared_ptr<ast::ExceptHandler>> handlers;
    StmtList orelse;
    StmtList finalbody;
    
    // Parse except blocks
    while (state.current().type == TokenType::EXCEPT) {
        state.update_location();
        int handler_line = state.start_lineno;
        int handler_col = state.start_col_offset;
        state.advance();
        
        ExprPtr type = nullptr;
        std::string name;
        
        if (state.current().type != TokenType::COLON) {
            auto exc_type = parse_expression(state);
            if (exc_type) {
                type = *exc_type;
            }
            
            if (state.current().type == TokenType::AS) {
                state.advance();
                if (state.current().type == TokenType::IDENTIFIER) {
                    name = state.current().value;
                    state.advance();
                }
            }
        }
        
        if (state.current().type != TokenType::COLON) {
            return std::nullopt;
        }
        state.advance();
        
        auto handler_body = parse_block(state);
        if (!handler_body) {
            return std::nullopt;
        }
        
        handlers.push_back(std::make_shared<ast::ExceptHandler>(
            type, name, *handler_body, handler_line, handler_col));
    }
    
    // Parse else block
    if (state.current().type == TokenType::ELSE) {
        state.advance();
        if (state.current().type != TokenType::COLON) {
            return std::nullopt;
        }
        state.advance();
        auto else_body = parse_block(state);
        if (else_body) {
            orelse = *else_body;
        }
    }
    
    // Parse finally block
    if (state.current().type == TokenType::FINALLY) {
        state.advance();
        if (state.current().type != TokenType::COLON) {
            return std::nullopt;
        }
        state.advance();
        auto final_body = parse_block(state);
        if (final_body) {
            finalbody = *final_body;
        }
    }
    
    return make_stmt(std::make_shared<ast::Try>(
        *body, handlers, orelse, finalbody, line, col));
}

inline ParseResult<StmtPtr> parse_with_stmt(ParserState& state) {
    bool is_async = false;
    if (state.current().type == TokenType::ASYNC) {
        is_async = true;
        state.advance();
    }
    
    if (state.current().type != TokenType::WITH) {
        return std::nullopt;
    }
    state.update_location();
    int line = state.start_lineno;
    int col = state.start_col_offset;
    state.advance();
    
    std::vector<ast::WithItem> items;
    
    // Parse first with_item
    auto context = parse_expression(state);
    if (!context) {
        return std::nullopt;
    }
    
    ExprPtr optional_vars = nullptr;
    if (state.current().type == TokenType::AS) {
        state.advance();
        auto vars = parse_expression(state);
        if (vars) {
            optional_vars = *vars;
        }
    }
    items.push_back(ast::WithItem{*context, optional_vars});
    
    // Parse additional with_items
    while (state.current().type == TokenType::COMMA) {
        state.advance();
        auto ctx = parse_expression(state);
        if (!ctx) {
            break;
        }
        ExprPtr vars = nullptr;
        if (state.current().type == TokenType::AS) {
            state.advance();
            auto v = parse_expression(state);
            if (v) {
                vars = *v;
            }
        }
        items.push_back(ast::WithItem{*ctx, vars});
    }
    
    if (state.current().type != TokenType::COLON) {
        return std::nullopt;
    }
    state.advance();
    
    auto body = parse_block(state);
    if (!body) {
        return std::nullopt;
    }
    
    if (is_async) {
        return make_stmt(std::make_shared<ast::AsyncWith>(items, *body, line, col));
    }
    
    return make_stmt(std::make_shared<ast::With>(items, *body, line, col));
}

inline ParseResult<StmtPtr> parse_match_stmt(ParserState& state) {
    if (state.current().type != TokenType::MATCH) {
        return std::nullopt;
    }
    state.update_location();
    int line = state.start_lineno;
    int col = state.start_col_offset;
    state.advance();
    
    auto subject = parse_expression(state);
    if (!subject) {
        return std::nullopt;
    }
    
    if (state.current().type != TokenType::COLON) {
        return std::nullopt;
    }
    state.advance();
    
    // Skip newlines
    while (state.current().type == TokenType::NEWLINE) {
        state.advance();
    }
    
    if (state.current().type != TokenType::INDENT) {
        return std::nullopt;
    }
    state.advance();
    
    std::vector<ast::match_case> cases;
    
    while (state.current().type == TokenType::CASE) {
        state.advance();
        
        auto pattern = parse_expression(state);  // Simplified
        if (!pattern) {
            break;
        }
        
        ExprPtr guard = nullptr;
        if (state.current().type == TokenType::IF) {
            state.advance();
            auto g = parse_expression(state);
            if (g) {
                guard = *g;
            }
        }
        
        if (state.current().type != TokenType::COLON) {
            break;
        }
        state.advance();
        
        auto body = parse_block(state);
        if (!body) {
            break;
        }
        
        cases.push_back(ast::match_case{*pattern, guard, *body});
    }
    
    if (state.current().type == TokenType::DEDENT) {
        state.advance();
    }
    
    return make_stmt(std::make_shared<ast::Match>(*subject, cases, line, col));
}

inline ParseResult<StmtPtr> parse_compound_stmt(ParserState& state) {
    // Use lookahead for efficiency
    switch (state.current().type) {
        case TokenType::DEF:
        case TokenType::ASYNC:
        case TokenType::AT:
            return parse_function_def(state);
        case TokenType::IF:
            return parse_if_stmt(state);
        case TokenType::CLASS:
            return parse_class_def(state);
        case TokenType::WITH:
            return parse_with_stmt(state);
        case TokenType::FOR:
            return parse_for_stmt(state);
        case TokenType::TRY:
            return parse_try_stmt(state);
        case TokenType::WHILE:
            return parse_while_stmt(state);
        case TokenType::MATCH:
            return parse_match_stmt(state);
        default:
            return std::nullopt;
    }
}

inline ParseResult<StmtPtr> parse_statement(ParserState& state) {
    // Skip newlines
    while (state.current().type == TokenType::NEWLINE) {
        state.advance();
    }
    
    // Try compound statement first
    auto compound = parse_compound_stmt(state);
    if (compound) {
        return compound;
    }
    
    // Try simple statement
    return parse_simple_stmt(state);
}

// ============================================================================
// Wrapper structs for compatibility
// ============================================================================

struct Statement {
    static ParseResult<StmtPtr> parse(ParserState& state) {
        return parse_statement(state);
    }
};

} // namespace grammar
} // namespace parser
} // namespace cpython_cpp

#endif // CPYTHON_CPP_GRAMMAR_STATEMENTS_HPP
