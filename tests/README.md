# Parser Tests

This directory contains comprehensive tests for the Python parser implementation.

## Test Organization

Tests are organized by feature type:

### Main Directory
- **Operators**: `test_arithmetic_operators.py`, `test_bitwise_operators.py`, `test_comparison_operators.py`, `test_logical_operators.py`, `test_unary_operators.py`
- **Data Structures**: `test_list_literals.py`, `test_dict_literals.py`
- **Control Flow**: `test_for_loop.py`, `test_while_loop.py`
- **Combined Features**: `test_combined_features.py`, `test_combined_new_features.py`

### Feature Directories

#### `try_except/`
Tests for exception handling:
- `test_try_except.py` - Basic try/except blocks
- `test_try_else.py` - Try/except/else combinations
- `test_try_finally.py` - Try/finally and try/except/finally

#### `classes/`
Tests for class definitions:
- `test_class_def.py` - Class definitions with methods and inheritance

#### `imports/`
Tests for import statements:
- `test_import.py` - Import and from import statements

#### `statements/`
Tests for simple statements:
- `test_pass.py` - Pass statement (no-op)
- `test_raise.py` - Raise statement (exception raising)
- `test_del.py` - Del statement (deletion)
- `test_assert.py` - Assert statement (assertions)
- `test_global.py` - Global statement (global variable declarations)
- `test_nonlocal.py` - Nonlocal statement (nonlocal variable declarations)

## Running Tests

### Run All Tests
```bash
./tests/run_all_tests.sh
```

### Run Individual Test
```bash
./cpp_project.exe tests/test_arithmetic_operators.py
./cpp_project.exe tests/try_except/test_try_except.py
./cpp_project.exe tests/statements/test_pass.py
```

### Run Tests for a Specific Feature
```bash
# Run all try/except tests
./cpp_project.exe tests/try_except/test_*.py

# Run all statement tests
./cpp_project.exe tests/statements/test_*.py
```

## Test Results

All tests should pass. The test runner provides a summary:
- Total number of tests
- Number of passed tests
- Number of failed tests
- Detailed error output for any failures

## Adding New Tests

When adding tests for new features:

1. **If it's a new feature type**: Create a new directory under `tests/` (e.g., `tests/lambda/`)
2. **If it fits an existing category**: Add to the appropriate directory
3. **If it's a general test**: Add to the main `tests/` directory

Follow the naming convention: `test_<feature_name>.py`

## Current Test Coverage

- ✅ All operators (arithmetic, bitwise, comparison, logical, unary)
- ✅ Data structures (lists, dictionaries)
- ✅ Control flow (if/elif/else, while, for)
- ✅ Exception handling (try/except/finally)
- ✅ Class definitions
- ✅ Import statements
- ✅ Simple statements (pass, raise, del, assert, global, nonlocal)
