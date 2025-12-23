# Development Guide: Working on Independent Modules

This guide shows how to work on CPython modules independently in the C++ project.

## Quick Start: Reference Counting Module

### Step 1: Create the Module Structure

```bash
mkdir -p src/core
# Create refcount.hpp (already created)
```

### Step 2: Implement Basic Functionality

The `RefCounted` class in `src/core/refcount.hpp` provides:
- Thread-safe reference counting
- Increment/decrement operations
- Automatic deallocation when refcount reaches 0

### Step 3: Write Tests

Create `tests/test_refcount.cpp`:

```cpp
#include "src/core/refcount.hpp"
#include <cassert>
#include <iostream>

class TestObject : public cpython_cpp::core::RefCounted {
public:
    TestObject(int val) : value_(val) {}
    int value() const { return value_; }

private:
    int value_;
};

void test_basic_refcounting() {
    auto* obj = new TestObject(42);
    assert(obj->get_ref_count() == 1);

    obj->incref();
    assert(obj->get_ref_count() == 2);

    bool should_dealloc = obj->decref();
    assert(!should_dealloc);
    assert(obj->get_ref_count() == 1);

    should_dealloc = obj->decref();
    assert(should_dealloc);
    // Object is deallocated
}

int main() {
    test_basic_refcounting();
    std::cout << "Reference counting tests passed!" << std::endl;
    return 0;
}
```

### Step 4: Build and Test

```bash
# Compile test
clang++ -std=c++11 -I. tests/test_refcount.cpp -o test_refcount

# Run test
./test_refcount
```

---

## Quick Start: Parser Module

### Step 1: Create Tokenizer

The tokenizer (`src/parser/tokenizer.hpp`) can be implemented independently.

### Step 2: Implement Tokenizer

Create `src/parser/tokenizer.cpp`:

```cpp
#include "tokenizer.hpp"
#include <cctype>
#include <sstream>

namespace cpython_cpp {
namespace parser {

Tokenizer::Tokenizer(const std::string& source)
    : source_(source), position_(0), line_(1), column_(1) {}

Token Tokenizer::next_token() {
    skip_whitespace();
    skip_comment();

    if (position_ >= source_.length()) {
        return Token(TokenType::EOF, "", line_, column_);
    }

    char c = source_[position_];

    // Handle numbers
    if (std::isdigit(c)) {
        return read_number();
    }

    // Handle strings
    if (c == '"' || c == '\'') {
        return read_string();
    }

    // Handle identifiers and keywords
    if (std::isalpha(c) || c == '_') {
        return read_identifier_or_keyword();
    }

    // Handle operators
    // ... implement operator parsing

    // Default: single character token
    position_++;
    return Token(TokenType::ENDMARKER, std::string(1, c), line_, column_);
}

// Implement other methods...

} // namespace parser
} // namespace cpython_cpp
```

### Step 3: Test Tokenizer

```cpp
#include "src/parser/tokenizer.hpp"
#include <iostream>

int main() {
    std::string code = "def hello():\n    print('world')";
    cpython_cpp::parser::Tokenizer tokenizer(code);

    auto tokens = tokenizer.tokenize();
    for (const auto& token : tokens) {
        std::cout << "Token: " << token.value
                  << " (line " << token.line << ")" << std::endl;
    }

    return 0;
}
```

---

## Quick Start: Opcodes Module

### Step 1: Define Opcodes

The opcodes are defined in `src/opcodes/opcodes.hpp`.

### Step 2: Implement Simple VM

Create `src/opcodes/vm.hpp`:

```cpp
#ifndef CPYTHON_CPP_VM_HPP
#define CPYTHON_CPP_VM_HPP

#include "opcodes.hpp"
#include <vector>
#include <stack>
#include <map>

namespace cpython_cpp {
namespace opcodes {

class VirtualMachine {
public:
    VirtualMachine();

    // Execute bytecode
    void execute(const std::vector<uint8_t>& bytecode,
                 const std::vector<int>& constants);

    // Stack operations
    void push(int value);
    int pop();

private:
    std::stack<int> stack_;
    std::map<std::string, int> globals_;
    std::vector<int> constants_;
    size_t pc_;  // Program counter

    void execute_opcode(Opcode opcode, int arg);
};

} // namespace opcodes
} // namespace cpython_cpp

#endif
```

### Step 3: Implement Opcode Handlers

```cpp
void VirtualMachine::execute_opcode(Opcode opcode, int arg) {
    switch (opcode) {
        case Opcode::LOAD_CONST:
            push(constants_[arg]);
            break;

        case Opcode::BINARY_ADD: {
            int b = pop();
            int a = pop();
            push(a + b);
            break;
        }

        case Opcode::RETURN_VALUE:
            // Return top of stack
            break;

        // ... implement other opcodes
    }
}
```

---

## Module Development Workflow

### 1. Choose a Module

Start with modules that have minimal dependencies:
- ✅ Parser (no dependencies)
- ✅ Reference Counting (minimal dependencies)
- ✅ Opcodes (needs object model)
- ✅ Compiler (needs parser)

### 2. Study CPython Source

Read the corresponding CPython files:
- `Objects/object.c` for reference counting
- `Parser/tokenizer/` for tokenizer
- `Include/opcode.h` for opcodes

### 3. Implement Incrementally

- Start with basic functionality
- Add features one at a time
- Write tests as you go

### 4. Test Independently

- Unit tests for each function
- Integration tests for module interactions
- Compare behavior with CPython

### 5. Integrate

- Once a module works, integrate with others
- Define clean interfaces between modules
- Refactor as needed

---

## Recommended Development Order

1. **Parser** (no dependencies)
   - Tokenizer
   - Parser
   - AST nodes

2. **Core Object Model** (minimal dependencies)
   - Reference counting
   - Base object class
   - Type system basics

3. **Opcodes** (needs object model)
   - Opcode definitions
   - Simple VM
   - Stack operations

4. **Compiler** (needs parser)
   - AST to bytecode
   - Symbol table
   - Code generation

5. **Interpreter** (needs all above)
   - Frame management
   - Execution loop
   - Runtime initialization

---

## Tips

1. **Use CPython as Reference**: Read the actual CPython source code
2. **Start Simple**: Implement minimal functionality first
3. **Test Frequently**: Write tests for each feature
4. **Document**: Comment your code, explain design decisions
5. **Iterate**: Refine and improve as you learn more

---

## Resources

- CPython Source: `/workspaces/cpython/`
- Internal Docs: `/workspaces/cpython/InternalDocs/`
- Module Overview: `MODULES_OVERVIEW.md`
- Grammar: `/workspaces/cpython/Grammar/python.gram`
- Opcodes: `/workspaces/cpython/Include/opcode.h`







