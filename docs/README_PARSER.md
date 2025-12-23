# AST Parser and Compiler

This module implements a Python AST parser and compiler that can break down Python scripts into their component parts.

## Features

- **Tokenizer**: Converts Python source code into tokens
- **Parser**: Parses tokens into Abstract Syntax Tree (AST)
- **AST Nodes**: Represents Python code structure (functions, statements, expressions)
- **Compiler**: Analyzes and breaks down the AST structure

## Building

```bash
cd /workspaces/cpython/cpp_project
make
```

## Usage

### Run with example code:
```bash
./cpp_project.exe
```

### Parse a Python file:
```bash
./cpp_project.exe example.py
```

## What It Does

The parser takes a Python script and:

1. **Tokenizes** the source code (breaks it into tokens like keywords, identifiers, operators)
2. **Parses** the tokens into an AST (Abstract Syntax Tree)
3. **Compiles** the AST to show:
   - Function definitions
   - Statements (assignments, returns, conditionals)
   - Expressions (operations, function calls, literals)
   - Statistics (function count, statement count, etc.)

## Example Output

For the example code, you'll see:

```
=== AST Structure ===
Module(
  body=[
    FunctionDef(
      name='hello',
      args=['name'],
      body=[
        Expr(...),
        Return(...)
      ]
    ),
    Assign(...),
    If(...),
    ...
  ]
)

=== Python Code Breakdown ===

Statistics:
  Functions: 2
  Statements: 8
  Expressions: 12
  Function names: hello, add

AST Structure:
  Module
    FunctionDef: hello
      Args: name
      Body:
        ...
```

## Architecture

```
Source Code
    ↓
Tokenizer (tokenizer.cpp)
    ↓
Tokens
    ↓
Parser (parser.cpp)
    ↓
AST (ast/*.cpp)
    ↓
Compiler (compiler.cpp)
    ↓
Breakdown/Statistics
```

## Module Structure

- `src/parser/` - Tokenizer and parser
- `src/ast/` - AST node classes
- `src/compiler/` - Compiler that analyzes AST

## Limitations

This is a simplified implementation. A full parser would:
- Handle indentation properly (INDENT/DEDENT tokens)
- Support all Python syntax
- Generate actual bytecode
- Handle error recovery better

## Next Steps

- Add more AST node types (classes, loops, etc.)
- Implement proper indentation handling
- Generate bytecode from AST
- Add error reporting with line numbers







