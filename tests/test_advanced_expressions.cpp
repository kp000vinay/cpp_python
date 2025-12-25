/**
 * Test suite for advanced expression features
 * 
 * Tests lambda, comprehensions, generators, and other advanced features
 */

#include <iostream>
#include <cassert>
#include <string>
#include "../src/parser/peg_parser_impl.hpp"
#include "../src/parser/grammar/advanced_expressions.hpp"

using namespace cpython_cpp;
using namespace cpython_cpp::parser;

#define TEST(name) void test_##name()
#define RUN_TEST(name) do { \
    std::cout << "Running " #name "... "; \
    test_##name(); \
    std::cout << "PASSED" << std::endl; \
} while(0)

// ============================================================================
// Lambda Tests
// ============================================================================

TEST(lambda_simple) {
    std::string src{"lambda x: x + 1"};
    Tokenizer tokenizer{src};
    auto tokens = tokenizer.tokenize();
    ParserState state(std::move(tokens));
    
    auto result = grammar::parse_lambda(state);
    assert(result);
}

TEST(lambda_multiple_args) {
    std::string src{"lambda x, y: x + y"};
    Tokenizer tokenizer{src};
    auto tokens = tokenizer.tokenize();
    ParserState state(std::move(tokens));
    
    auto result = grammar::parse_lambda(state);
    assert(result);
}

TEST(lambda_no_args) {
    std::string src{"lambda: 42"};
    Tokenizer tokenizer{src};
    auto tokens = tokenizer.tokenize();
    ParserState state(std::move(tokens));
    
    auto result = grammar::parse_lambda(state);
    assert(result);
}

// ============================================================================
// Yield Tests
// ============================================================================

TEST(yield_simple) {
    std::string src{"yield x"};
    Tokenizer tokenizer{src};
    auto tokens = tokenizer.tokenize();
    ParserState state(std::move(tokens));
    
    auto result = grammar::parse_yield_expr(state);
    assert(result);
}

TEST(yield_from) {
    std::string src{"yield from other_gen"};
    Tokenizer tokenizer{src};
    auto tokens = tokenizer.tokenize();
    ParserState state(std::move(tokens));
    
    auto result = grammar::parse_yield_expr(state);
    assert(result);
}

// ============================================================================
// Starred Expression Tests
// ============================================================================

TEST(starred_expr) {
    std::string src{"*args"};
    Tokenizer tokenizer{src};
    auto tokens = tokenizer.tokenize();
    ParserState state(std::move(tokens));
    
    auto result = grammar::parse_starred_expression(state);
    assert(result);
}

// ============================================================================
// Slice Tests
// ============================================================================

TEST(slice_full) {
    std::string src{"1:10:2"};
    Tokenizer tokenizer{src};
    auto tokens = tokenizer.tokenize();
    ParserState state(std::move(tokens));
    
    auto result = grammar::parse_slice(state);
    assert(result);
}

TEST(slice_partial) {
    std::string src{":10"};
    Tokenizer tokenizer{src};
    auto tokens = tokenizer.tokenize();
    ParserState state(std::move(tokens));
    
    auto result = grammar::parse_slice(state);
    assert(result);
}

// ============================================================================
// Main
// ============================================================================

int main() {
    std::cout << "=== Advanced Expressions Test Suite ===" << std::endl;
    std::cout << std::endl;
    
    std::cout << "--- Lambda Tests ---" << std::endl;
    RUN_TEST(lambda_simple);
    RUN_TEST(lambda_multiple_args);
    RUN_TEST(lambda_no_args);
    
    std::cout << std::endl;
    std::cout << "--- Yield Tests ---" << std::endl;
    RUN_TEST(yield_simple);
    RUN_TEST(yield_from);
    
    std::cout << std::endl;
    std::cout << "--- Starred Expression Tests ---" << std::endl;
    RUN_TEST(starred_expr);
    
    std::cout << std::endl;
    std::cout << "--- Slice Tests ---" << std::endl;
    RUN_TEST(slice_full);
    RUN_TEST(slice_partial);
    
    std::cout << std::endl;
    std::cout << "=== All advanced expression tests passed! ===" << std::endl;
    
    return 0;
}
