# Comprehensive Test Script

The `comprehensive_test.py` file is a 498-line Python script designed to test various Python opcodes and language constructs. It includes:

## Arithmetic Operations
- Addition (`+`), Subtraction (`-`), Multiplication (`*`), Division (`/`)
- Modulo (`%`), Exponentiation (`**`), Floor Division (`//`)

## Comparison Operations
- Equal (`==`), Not Equal (`!=`)
- Less Than (`<`), Greater Than (`>`)
- Less or Equal (`<=`), Greater or Equal (`>=`)

## Logical Operations
- `and`, `or`, `not`

## Bitwise Operations
- Bitwise OR (`|`), AND (`&`), XOR (`^`), NOT (`~`)
- Left Shift (`<<`), Right Shift (`>>`)

## Control Flow
- `if`, `elif`, `else` statements
- `for` loops
- `while` loops
- `break`, `continue`
- Nested loops

## Functions
- Simple functions
- Functions with arguments
- Functions with default parameters
- Functions with multiple return statements
- Nested functions
- Lambda functions

## Data Structures
- Lists (creation, indexing, slicing)
- Dictionaries (creation, access, methods)
- Sets (creation, operations)
- Tuples

## Comprehensions
- List comprehensions
- Dictionary comprehensions
- Set comprehensions
- Generator expressions

## Exception Handling
- `try`, `except`, `finally`
- `raise` statements
- Multiple exception types

## Classes
- Class definitions
- `__init__` methods
- Instance methods
- Inheritance
- Method calls

## Advanced Features
- `with` statements (context managers)
- `import` statements
- `from ... import ...`
- Decorators
- Attribute access
- Method chaining
- String operations
- F-strings
- Unpacking
- Starred expressions
- Keyword-only arguments
- Positional-only arguments
- `match`/`case` statements (Python 3.10+)
- Walrus operator (`:=`) (Python 3.8+)

## Usage

```bash
./cpp_project.exe comprehensive_test.py
```

## Current Parser Limitations

The current parser implementation supports:
- ✅ Basic arithmetic operations (+, -, *, /, %, **, //)
- ✅ Comparison operations (==, !=, <, >, <=, >=)
- ✅ Function definitions
- ✅ Variable assignments
- ✅ If statements (basic)
- ✅ Function calls
- ✅ String and number literals

The parser is still being developed and may not handle all constructs in this comprehensive test file. As the parser evolves, more features will be supported.

## What This Tests

This comprehensive test file helps verify:
1. Tokenizer can recognize all operators and keywords
2. Parser can handle complex expressions
3. AST generation works for various constructs
4. The compiler can analyze different code patterns






