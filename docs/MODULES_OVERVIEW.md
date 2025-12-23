# CPython Logical Modules Overview

This document provides a high-level overview of CPython's logical modules and how to work on them independently in the C++ project.

## Core Logical Modules

### 1. **Object Model & Reference Counting** (`Objects/`, `Include/object.h`)
**Purpose**: Core object system, reference counting, and built-in types

**Key Files**:
- `Objects/object.c` - Base object implementation
- `Objects/abstract.c` - Abstract object protocol
- `Include/object.h` - Object type definitions
- `Include/refcount.h` - Reference counting macros

**What it does**:
- Manages Python object lifecycle (creation, reference counting, deallocation)
- Implements the object model (type system, attribute access)
- Handles reference counting (increment/decrement, cycle detection)

**Independent Work**: Can be implemented as a standalone module with:
- `Object` base class with reference counting
- Type system infrastructure
- Basic object operations (hash, comparison, string representation)

---

### 2. **Grammar & Parsing** (`Grammar/`, `Parser/`)
**Purpose**: Parse Python source code into Abstract Syntax Trees (AST)

**Key Files**:
- `Grammar/python.gram` - PEG grammar definition
- `Grammar/Tokens` - Token definitions
- `Parser/parser.c` - Generated parser
- `Parser/tokenizer/` - Tokenizer implementation
- `Parser/lexer/` - Lexer implementation

**What it does**:
- Tokenizes source code (converts text to tokens)
- Parses tokens into AST nodes
- Validates syntax according to Python grammar

**Independent Work**: Can be implemented as:
- Tokenizer module (text → tokens)
- Parser module (tokens → AST)
- AST node classes (Module, Function, Statement, Expression, etc.)

---

### 3. **Opcodes & Bytecode** (`Include/opcode.h`, `Python/bytecodes.c`)
**Purpose**: Define and execute Python bytecode instructions

**Key Files**:
- `Include/opcode.h` - Opcode definitions and IDs
- `Include/opcode_ids.h` - Opcode ID constants
- `Python/bytecodes.c` - Opcode implementations (DSL)
- `Python/ceval.c` - Bytecode evaluation engine

**What it does**:
- Defines bytecode instructions (LOAD_FAST, CALL_FUNCTION, etc.)
- Executes bytecode in evaluation loop
- Manages execution stack and frame state

**Independent Work**: Can be implemented as:
- Opcode enum/constants
- Virtual machine with opcode dispatch
- Stack-based execution engine
- Frame management

---

### 4. **Compiler** (`Python/compile.c`, `Python/flowgraph.c`)
**Purpose**: Compile AST to bytecode

**Key Files**:
- `Python/compile.c` - AST to bytecode compilation
- `Python/flowgraph.c` - Control flow graph construction
- `Python/assemble.c` - Bytecode assembly
- `Python/symtable.c` - Symbol table generation
- `Python/codegen.c` - Code generation

**What it does**:
- Transforms AST into instruction sequences
- Builds control flow graphs
- Optimizes bytecode
- Generates code objects

**Independent Work**: Can be implemented as:
- AST visitor pattern for compilation
- Symbol table builder
- Control flow graph construction
- Bytecode emitter

---

### 5. **Memory Management** (`Objects/obmalloc.c`, `Objects/mimalloc/`)
**Purpose**: Memory allocation and garbage collection

**Key Files**:
- `Objects/obmalloc.c` - Object allocator
- `Objects/mimalloc/` - Alternative allocator (mimalloc)
- `Python/gc.c` - Garbage collector
- `Python/gc_free_threading.c` - Free-threaded GC

**What it does**:
- Allocates/deallocates Python objects
- Manages memory pools
- Detects and collects cyclic garbage
- Handles memory fragmentation

**Independent Work**: Can be implemented as:
- Memory allocator interface
- Reference counting system
- Cycle detector (for cyclic GC)
- Memory pool manager

---

### 6. **Interpreter Core** (`Python/ceval.c`, `Python/pythonrun.c`)
**Purpose**: Execute Python code and manage runtime

**Key Files**:
- `Python/ceval.c` - Bytecode evaluation loop
- `Python/pythonrun.c` - Python runtime initialization
- `Python/pylifecycle.c` - Lifecycle management
- `Python/pystate.c` - Interpreter state management

**What it does**:
- Runs the main evaluation loop
- Manages interpreter state
- Handles execution context
- Coordinates between modules

**Independent Work**: Can be implemented as:
- Main interpreter loop
- Frame execution engine
- Runtime initialization
- State management

---

### 7. **Built-in Types** (`Objects/*object.c`)
**Purpose**: Implementations of Python built-in types

**Key Files**:
- `Objects/longobject.c` - int type
- `Objects/unicodeobject.c` - str type
- `Objects/listobject.c` - list type
- `Objects/dictobject.c` - dict type
- `Objects/tupleobject.c` - tuple type
- `Objects/setobject.c` - set type

**What it does**:
- Implements type-specific operations
- Handles type-specific memory layout
- Provides type methods and operators

**Independent Work**: Each type can be a separate module:
- `IntObject`, `StrObject`, `ListObject`, etc.
- Type-specific operations and methods
- Type-specific memory management

---

### 8. **Frame & Execution Context** (`Objects/frameobject.c`, `Python/frame.c`)
**Purpose**: Manage execution frames and call stack

**Key Files**:
- `Objects/frameobject.c` - Frame object implementation
- `Python/frame.c` - Frame management
- `Include/frameobject.h` - Frame definitions

**What it does**:
- Tracks execution state (locals, globals, bytecode position)
- Manages call stack
- Handles frame creation/destruction

**Independent Work**: Can be implemented as:
- `Frame` class with execution state
- Call stack management
- Local/global namespace handling

---

### 9. **Import System** (`Python/import.c`, `Python/importdl.c`)
**Purpose**: Module loading and import mechanism

**Key Files**:
- `Python/import.c` - Import system core
- `Python/importdl.c` - Dynamic loading
- `Objects/moduleobject.c` - Module objects

**What it does**:
- Resolves module paths
- Loads and caches modules
- Handles import statements

**Independent Work**: Can be implemented as:
- Module finder and loader
- Import path resolution
- Module cache management

---

### 10. **Exception Handling** (`Objects/exceptions.c`, `Python/errors.c`)
**Purpose**: Exception objects and error handling

**Key Files**:
- `Objects/exceptions.c` - Exception types
- `Python/errors.c` - Error handling utilities
- `Include/pyerrors.h` - Error definitions

**What it does**:
- Defines exception types
- Manages exception state
- Handles exception propagation

**Independent Work**: Can be implemented as:
- Exception class hierarchy
- Exception state management
- Exception propagation mechanism

---

## Module Dependencies

```
┌─────────────────────────────────────────┐
│      Interpreter Core (ceval.c)        │
│  ┌───────────────────────────────────┐  │
│  │  Frame Management (frame.c)      │  │
│  │  ┌─────────────────────────────┐  │  │
│  │  │ Opcodes (bytecodes.c)      │  │  │
│  │  └─────────────────────────────┘  │  │
│  └───────────────────────────────────┘  │
└─────────────────────────────────────────┘
           │
           ├─── Object Model (object.c)
           │    ├─── Reference Counting
           │    └─── Built-in Types
           │
           ├─── Memory Management (obmalloc.c, gc.c)
           │
           └─── Exception Handling (exceptions.c)
```

```
┌─────────────────────────────────────────┐
│      Compiler (compile.c)                │
│  ┌───────────────────────────────────┐  │
│  │  AST → Bytecode                   │  │
│  └───────────────────────────────────┘  │
└─────────────────────────────────────────┘
           │
           └─── Parser (parser.c)
                └─── Tokenizer (tokenizer/)
```

---

## Suggested C++ Project Structure

```
cpp_project/
├── src/
│   ├── core/
│   │   ├── object.hpp          # Base object class
│   │   ├── object.cpp
│   │   ├── refcount.hpp        # Reference counting
│   │   └── refcount.cpp
│   │
│   ├── parser/
│   │   ├── tokenizer.hpp       # Tokenizer
│   │   ├── tokenizer.cpp
│   │   ├── parser.hpp           # Parser
│   │   └── parser.cpp
│   │
│   ├── ast/
│   │   ├── node.hpp            # AST base class
│   │   ├── nodes/              # AST node types
│   │   │   ├── module.hpp
│   │   │   ├── function.hpp
│   │   │   └── ...
│   │
│   ├── compiler/
│   │   ├── compiler.hpp        # AST → Bytecode
│   │   ├── compiler.cpp
│   │   ├── symtable.hpp        # Symbol table
│   │   └── flowgraph.hpp       # Control flow
│   │
│   ├── opcodes/
│   │   ├── opcodes.hpp         # Opcode definitions
│   │   ├── vm.hpp              # Virtual machine
│   │   └── vm.cpp
│   │
│   ├── types/
│   │   ├── int_object.hpp      # Built-in types
│   │   ├── str_object.hpp
│   │   ├── list_object.hpp
│   │   └── ...
│   │
│   ├── memory/
│   │   ├── allocator.hpp       # Memory management
│   │   └── gc.hpp              # Garbage collector
│   │
│   ├── runtime/
│   │   ├── frame.hpp           # Execution frames
│   │   ├── interpreter.hpp     # Main interpreter
│   │   └── interpreter.cpp
│   │
│   └── exceptions/
│       ├── exceptions.hpp       # Exception types
│       └── exceptions.cpp
│
├── tests/
│   ├── test_refcount.cpp
│   ├── test_parser.cpp
│   ├── test_opcodes.cpp
│   └── ...
│
└── main.cpp                    # Entry point
```

---

## Working on Independent Modules

### Example 1: Reference Counting Module

**Isolation Strategy**:
- Create `src/core/refcount.hpp` and `refcount.cpp`
- Define `RefCounted` base class
- Implement increment/decrement operations
- Test independently with simple objects

**Dependencies**: Minimal (just memory management)

**Test**: Create objects, verify reference counts, test deallocation

---

### Example 2: Grammar & Parser Module

**Isolation Strategy**:
- Create `src/parser/` directory
- Implement tokenizer first (text → tokens)
- Then parser (tokens → AST)
- Use grammar file from CPython as reference

**Dependencies**: None (can work standalone)

**Test**: Parse simple Python code, verify AST structure

---

### Example 3: Opcodes Module

**Isolation Strategy**:
- Create `src/opcodes/` directory
- Define opcode enum from `Include/opcode.h`
- Implement simple VM with stack
- Add opcode handlers one by one

**Dependencies**: Object model (for stack values), Frame (for execution state)

**Test**: Execute simple bytecode sequences, verify stack operations

---

## Development Workflow

1. **Start with Independent Modules**:
   - Parser (no dependencies)
   - Reference counting (minimal dependencies)
   - Basic object model

2. **Build Up Dependencies**:
   - Add opcodes after object model exists
   - Add compiler after parser exists
   - Add interpreter after all components exist

3. **Test Each Module**:
   - Unit tests for each module
   - Integration tests for module interactions
   - End-to-end tests for complete functionality

4. **Iterate**:
   - Refine module interfaces
   - Add missing functionality
   - Optimize performance

---

## Key Design Principles

1. **Modularity**: Each module should have clear boundaries
2. **Interfaces**: Define clean interfaces between modules
3. **Testability**: Each module should be testable independently
4. **Incremental Development**: Build one module at a time
5. **Reference Implementation**: Use CPython source as reference

---

## Next Steps

1. Choose a module to start with (recommended: Parser or Reference Counting)
2. Create the module structure in `src/`
3. Implement basic functionality
4. Write tests
5. Integrate with other modules as needed







