/**
 * Test suite for the new PEG Parser implementation
 * 
 * Tests the template-based parser against various Python constructs
 */

#include <iostream>
#include <cassert>
#include <string>
#include "../src/parser/peg_parser_impl.hpp"

using namespace cpython_cpp;
using namespace cpython_cpp::parser;

// Test helper macro
#define TEST(name) void test_##name()
#define RUN_TEST(name) do { \
    std::cout << "Running " #name "... "; \
    test_##name(); \
    std::cout << "PASSED" << std::endl; \
} while(0)

// ============================================================================
// Expression Tests
// ============================================================================

TEST(simple_number) {
    auto expr = parse_expr("42");
    assert(expr != nullptr);
}

TEST(simple_name) {
    auto expr = parse_expr("foo");
    assert(expr != nullptr);
}

TEST(binary_add) {
    auto expr = parse_expr("1 + 2");
    assert(expr != nullptr);
}

TEST(binary_complex) {
    auto expr = parse_expr("1 + 2 * 3 - 4 / 5");
    assert(expr != nullptr);
}

TEST(comparison) {
    auto expr = parse_expr("x < y");
    assert(expr != nullptr);
}

TEST(boolean_and) {
    auto expr = parse_expr("x and y");
    assert(expr != nullptr);
}

TEST(boolean_or) {
    auto expr = parse_expr("x or y");
    assert(expr != nullptr);
}

TEST(unary_not) {
    auto expr = parse_expr("not x");
    assert(expr != nullptr);
}

TEST(conditional) {
    auto expr = parse_expr("x if condition else y");
    assert(expr != nullptr);
}

TEST(list_literal) {
    auto expr = parse_expr("[1, 2, 3]");
    assert(expr != nullptr);
}

TEST(dict_literal) {
    auto expr = parse_expr("{'a': 1, 'b': 2}");
    assert(expr != nullptr);
}

TEST(function_call) {
    auto expr = parse_expr("print(x)");
    assert(expr != nullptr);
}

TEST(attribute_access) {
    auto expr = parse_expr("obj.attr");
    assert(expr != nullptr);
}

TEST(subscript) {
    auto expr = parse_expr("arr[0]");
    assert(expr != nullptr);
}

TEST(tuple) {
    auto expr = parse_expr("(1, 2, 3)");
    assert(expr != nullptr);
}

TEST(power) {
    auto expr = parse_expr("2 ** 10");
    assert(expr != nullptr);
}

TEST(bitwise_ops) {
    auto expr = parse_expr("a & b | c ^ d");
    assert(expr != nullptr);
}

TEST(shift_ops) {
    auto expr = parse_expr("x << 2 >> 1");
    assert(expr != nullptr);
}

// ============================================================================
// Statement Tests
// ============================================================================

TEST(assignment) {
    auto module = parse("x = 42\n");
    assert(module != nullptr);
    assert(module->body().size() == 1);
}

TEST(augmented_assignment) {
    auto module = parse("x += 1\n");
    assert(module != nullptr);
}

TEST(annotated_assignment) {
    auto module = parse("x: int = 5\n");
    assert(module != nullptr);
}

TEST(if_statement) {
    auto module = parse(
        "if x:\n"
        "    y = 1\n"
    );
    assert(module != nullptr);
}

TEST(if_else) {
    auto module = parse(
        "if x:\n"
        "    y = 1\n"
        "else:\n"
        "    y = 2\n"
    );
    assert(module != nullptr);
}

TEST(if_elif_else) {
    auto module = parse(
        "if x:\n"
        "    y = 1\n"
        "elif z:\n"
        "    y = 2\n"
        "else:\n"
        "    y = 3\n"
    );
    assert(module != nullptr);
}

TEST(while_loop) {
    auto module = parse(
        "while x:\n"
        "    x -= 1\n"
    );
    assert(module != nullptr);
}

TEST(for_loop) {
    auto module = parse(
        "for i in range(10):\n"
        "    print(i)\n"
    );
    assert(module != nullptr);
}

TEST(function_def) {
    auto module = parse(
        "def foo(x, y):\n"
        "    return x + y\n"
    );
    assert(module != nullptr);
}

TEST(function_with_annotations) {
    auto module = parse(
        "def foo(x: int, y: int) -> int:\n"
        "    return x + y\n"
    );
    assert(module != nullptr);
}

TEST(class_def) {
    auto module = parse(
        "class Foo:\n"
        "    pass\n"
    );
    assert(module != nullptr);
}

TEST(class_with_base) {
    auto module = parse(
        "class Foo(Bar):\n"
        "    pass\n"
    );
    assert(module != nullptr);
}

TEST(try_except) {
    auto module = parse(
        "try:\n"
        "    x = 1\n"
        "except:\n"
        "    pass\n"
    );
    assert(module != nullptr);
}

TEST(try_except_finally) {
    auto module = parse(
        "try:\n"
        "    x = 1\n"
        "except Exception as e:\n"
        "    print(e)\n"
        "finally:\n"
        "    cleanup()\n"
    );
    assert(module != nullptr);
}

TEST(with_statement) {
    auto module = parse(
        "with open('file') as f:\n"
        "    data = f.read()\n"
    );
    assert(module != nullptr);
}

TEST(decorator) {
    auto module = parse(
        "@decorator\n"
        "def foo():\n"
        "    pass\n"
    );
    assert(module != nullptr);
}

TEST(return_stmt) {
    auto module = parse("return x\n");
    assert(module != nullptr);
}

TEST(pass_stmt) {
    auto module = parse("pass\n");
    assert(module != nullptr);
}

TEST(break_continue) {
    auto module = parse(
        "while True:\n"
        "    if x:\n"
        "        break\n"
        "    continue\n"
    );
    assert(module != nullptr);
}

TEST(raise_stmt) {
    auto module = parse("raise ValueError('error')\n");
    assert(module != nullptr);
}

TEST(assert_stmt) {
    auto module = parse("assert x > 0, 'must be positive'\n");
    assert(module != nullptr);
}

TEST(global_nonlocal) {
    auto module = parse(
        "global x\n"
        "nonlocal y\n"
    );
    assert(module != nullptr);
}

TEST(del_stmt) {
    auto module = parse("del x, y\n");
    assert(module != nullptr);
}

// ============================================================================
// Complex Tests
// ============================================================================

TEST(nested_functions) {
    auto module = parse(
        "def outer():\n"
        "    def inner():\n"
        "        return 1\n"
        "    return inner()\n"
    );
    assert(module != nullptr);
}

TEST(complex_expression) {
    auto module = parse(
        "result = (a + b) * (c - d) / (e ** f) % g\n"
    );
    assert(module != nullptr);
}

TEST(method_chaining) {
    auto expr = parse_expr("obj.method1().method2().attr");
    assert(expr != nullptr);
}

TEST(nested_calls) {
    auto expr = parse_expr("f(g(h(x)))");
    assert(expr != nullptr);
}

TEST(multiple_statements) {
    auto module = parse(
        "x = 1\n"
        "y = 2\n"
        "z = x + y\n"
    );
    assert(module != nullptr);
    assert(module->body().size() == 3);
}

// ============================================================================
// Tokenizer Tests
// ============================================================================

TEST(tokenizer_basic) {
    Tokenizer tokenizer("x = 1 + 2");
    auto tokens = tokenizer.tokenize();
    assert(tokens.size() > 0);
}

TEST(tokenizer_keywords) {
    Tokenizer tokenizer("if True and False or None");
    auto tokens = tokenizer.tokenize();
    // Should have: if, True, and, False, or, None, ENDMARKER
    assert(tokens.size() >= 6);
}

TEST(tokenizer_operators) {
    Tokenizer tokenizer("+ - * / ** // % @ & | ^ ~ << >> < > <= >= == != := ->");
    auto tokens = tokenizer.tokenize();
    assert(tokens.size() > 10);
}

TEST(tokenizer_indentation) {
    Tokenizer tokenizer(
        "if x:\n"
        "    y = 1\n"
        "z = 2\n"
    );
    auto tokens = tokenizer.tokenize();
    // Should have INDENT and DEDENT tokens
    bool has_indent = false;
    bool has_dedent = false;
    for (const auto& tok : tokens) {
        if (tok.type == TokenType::INDENT) has_indent = true;
        if (tok.type == TokenType::DEDENT) has_dedent = true;
    }
    assert(has_indent);
    assert(has_dedent);
}

// ============================================================================
// Main
// ============================================================================

int main() {
    std::cout << "=== PEG Parser Test Suite ===" << std::endl;
    std::cout << std::endl;
    
    std::cout << "--- Expression Tests ---" << std::endl;
    RUN_TEST(simple_number);
    RUN_TEST(simple_name);
    RUN_TEST(binary_add);
    RUN_TEST(binary_complex);
    RUN_TEST(comparison);
    RUN_TEST(boolean_and);
    RUN_TEST(boolean_or);
    RUN_TEST(unary_not);
    RUN_TEST(conditional);
    RUN_TEST(list_literal);
    RUN_TEST(dict_literal);
    RUN_TEST(function_call);
    RUN_TEST(attribute_access);
    RUN_TEST(subscript);
    RUN_TEST(tuple);
    RUN_TEST(power);
    RUN_TEST(bitwise_ops);
    RUN_TEST(shift_ops);
    
    std::cout << std::endl;
    std::cout << "--- Statement Tests ---" << std::endl;
    RUN_TEST(assignment);
    RUN_TEST(augmented_assignment);
    RUN_TEST(annotated_assignment);
    RUN_TEST(if_statement);
    RUN_TEST(if_else);
    RUN_TEST(if_elif_else);
    RUN_TEST(while_loop);
    RUN_TEST(for_loop);
    RUN_TEST(function_def);
    RUN_TEST(function_with_annotations);
    RUN_TEST(class_def);
    RUN_TEST(class_with_base);
    RUN_TEST(try_except);
    RUN_TEST(try_except_finally);
    RUN_TEST(with_statement);
    RUN_TEST(decorator);
    RUN_TEST(return_stmt);
    RUN_TEST(pass_stmt);
    RUN_TEST(break_continue);
    RUN_TEST(raise_stmt);
    RUN_TEST(assert_stmt);
    RUN_TEST(global_nonlocal);
    RUN_TEST(del_stmt);
    
    std::cout << std::endl;
    std::cout << "--- Complex Tests ---" << std::endl;
    RUN_TEST(nested_functions);
    RUN_TEST(complex_expression);
    RUN_TEST(method_chaining);
    RUN_TEST(nested_calls);
    RUN_TEST(multiple_statements);
    
    std::cout << std::endl;
    std::cout << "--- Tokenizer Tests ---" << std::endl;
    RUN_TEST(tokenizer_basic);
    RUN_TEST(tokenizer_keywords);
    RUN_TEST(tokenizer_operators);
    RUN_TEST(tokenizer_indentation);
    
    std::cout << std::endl;
    std::cout << "=== All tests passed! ===" << std::endl;
    
    return 0;
}
