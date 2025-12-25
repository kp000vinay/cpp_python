# cpp_python - A CPython Parser Implementation in C++

This repository contains a C++ implementation of a parser for the Python programming language, designed to be as close as possible to CPython's own parser. The implementation has been refactored to be a header-only, template-based PEG (Parsing Expression Grammar) parser, inspired by CPython's `pegen`.

## Project Goals

The primary goals of this project are to:

1.  **Replicate CPython's Parser**: Create a parser that understands Python's grammar and produces an Abstract Syntax Tree (AST) compatible with CPython's.
2.  **High Performance**: Optimize the parser for speed and efficiency.
3.  **Modern C++**: Utilize modern C++20 features for a clean, maintainable, and header-only implementation.
4.  **Extensibility**: Design the parser to be easily extensible with new grammar rules and features from future Python versions.

## New Architecture: Template-based PEG Parser

The original recursive descent parser has been replaced with a more powerful and flexible template-based PEG parser. This new architecture offers several advantages:

*   **Header-Only**: The entire parser is implemented in header files, eliminating the need for separate compilation units and simplifying integration.
*   **Memoization**: The parser uses memoization (packrat parsing) to cache the results of parsing rules, avoiding redundant computations and achieving linear-time parsing complexity for most grammars.
*   **Grammar as Code**: Python's grammar rules are directly translated into C++ template structures, making the implementation closely follow the official Python grammar file (`python.gram`).
*   **Extensibility**: Adding new grammar rules is as simple as defining a new parser struct or function, which can be easily composed with existing ones.

### Core Components

The new parser is organized into the following key components:

| File | Description |
| :--- | :--- |
| `src/parser/peg_parser.hpp` | The core PEG parser engine, including the `ParserState`, `ParseResult`, and memoization table. |
| `src/parser/tokenizer_v2.hpp` | A new, more efficient tokenizer that generates the token stream for the parser. |
| `src/parser/grammar/expressions.hpp` | Implements the grammar rules for all Python expressions. |
| `src/parser/grammar/statements.hpp` | Implements the grammar rules for all Python statements. |
| `src/parser/grammar/advanced_expressions.hpp` | Implements advanced grammar rules for features like lambdas, comprehensions, and starred expressions. |
| `src/parser/peg_parser_impl.hpp` | The main parser interface that ties everything together and provides the public API. |

## How to Use the New Parser

The new parser can be used through the simple, high-level API provided in `peg_parser_impl.hpp`.

### Parsing a Module

To parse a full Python source file or a string containing multiple statements, use the `parse()` function:

```cpp
#include "src/parser/peg_parser_impl.hpp"
#include <iostream>

int main() {
    std::string source = "x = 42\nprint(x)";
    auto module = cpython_cpp::parser::parse(source);
    
    if (module) {
        std::cout << "Parsed module with " << module->body().size() << " statements." << std::endl;
    }
    
    return 0;
}
```

### Parsing an Expression

To parse a single Python expression, use the `parse_expr()` function:

```cpp
#include "src/parser/peg_parser_impl.hpp"
#include <iostream>

int main() {
    std::string source = "1 + 2 * 3";
    auto expr = cpython_cpp::parser::parse_expr(source);
    
    if (expr) {
        std::cout << "Successfully parsed expression." << std::endl;
    }
    
    return 0;
}
```

## Added Functionality

In addition to refactoring the core parser, the following features from CPython have been added:

*   **Lambda Expressions**: `lambda x: x + 1`
*   **List, Set, and Dict Comprehensions**: `[x for x in range(10) if x % 2 == 0]`
*   **Generator Expressions**: `(x for x in range(10))`
*   **Starred Expressions**: `*args`, `**kwargs`
*   **Yield and Yield From**: `yield x`, `yield from g`
*   **Slice Expressions**: `a[1:10:2]`
*   **Named Expressions (Walrus Operator)**: `(x := 10)`

## Building and Running Tests

The project includes two test suites to verify the correctness of the parser.

### Dependencies

*   A C++20 compatible compiler (e.g., g++ 11 or later)

### Compiling and Running Tests

From the root of the repository, you can compile and run the tests as follows:

```bash
# Compile and run the main parser test suite
g++ -std=c++20 -I. tests/test_peg_parser.cpp -o tests/test_peg_parser
./tests/test_peg_parser

# Compile and run the advanced expressions test suite
g++ -std=c++20 -I. tests/test_advanced_expressions.cpp -o tests/test_advanced_expressions
./tests/test_advanced_expressions
```

Both test suites should compile and run without errors, indicating that the parser is working correctly.
