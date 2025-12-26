#include <iostream>
#include <fstream>
#include <sstream>
#include "src/parser/tokenizer.hpp"
#include "src/parser/parser.hpp"
#include "src/compiler/bytecode_compiler.hpp"

using namespace cpython_cpp;

void test_compile(const std::string& code, const std::string& name) {
    std::cout << "=== Test: " << name << " ===\n";
    std::cout << "Source:\n" << code << "\n\n";
    
    // Parse directly (parser handles tokenization internally)
    parser::Parser parser_obj(code);
    auto module = parser_obj.parse();
    
    if (!module) {
        std::cerr << "Parse error!\n\n";
        return;
    }
    
    // Compile
    compiler::BytecodeCompiler compiler;
    auto code_obj = compiler.compile(*module, "<test>");
    
    if (compiler.has_errors()) {
        std::cerr << "Compilation errors:\n";
        for (const auto& err : compiler.errors()) {
            std::cerr << "  " << err << "\n";
        }
    }
    
    // Disassemble
    std::cout << "Bytecode:\n";
    std::cout << code_obj->disassemble();
    std::cout << "\n";
}

int main() {
    // Test 1: Simple assignment
    test_compile(R"(
x = 42
)", "Simple Assignment");

    // Test 2: Arithmetic
    test_compile(R"(
x = 1 + 2 * 3
)", "Arithmetic Expression");

    // Test 3: Function definition
    test_compile(R"(
def add(a, b):
    return a + b
)", "Function Definition");

    // Test 4: If statement
    test_compile(R"(
x = 10
if x > 5:
    y = 1
else:
    y = 0
)", "If Statement");

    // Test 5: While loop
    test_compile(R"(
x = 0
while x < 10:
    x = x + 1
)", "While Loop");

    // Test 6: For loop
    test_compile(R"(
total = 0
for i in range(10):
    total = total + i
)", "For Loop");

    // Test 7: List and dict
    test_compile(R"(
lst = [1, 2, 3]
dct = {'a': 1, 'b': 2}
)", "List and Dict");

    // Test 8: Function call
    test_compile(R"(
print("Hello, World!")
)", "Function Call");

    // Test 9: Class definition
    test_compile(R"(
class Point:
    def __init__(self, x, y):
        self.x = x
        self.y = y
)", "Class Definition");

    // Test 10: Augmented assignment
    test_compile(R"(
x = 5
x += 3
x *= 2
)", "Augmented Assignment");

    // Test 11: Boolean operations
    test_compile(R"(
x = True and False
y = True or False
z = not x
)", "Boolean Operations");

    // Test 12: Comparison
    test_compile(R"(
x = 5
result = x > 3
)", "Comparison");

    // Test 13: Lambda
    test_compile(R"(
f = lambda x: x * 2
)", "Lambda");

    // Test 14: Conditional expression
    test_compile(R"(
x = 10
y = "big" if x > 5 else "small"
)", "Conditional Expression");

    // Test 15: Try/except
    test_compile(R"(
try:
    x = 1 / 0
except ZeroDivisionError:
    x = 0
)", "Try/Except");

    // Test 16: Import
    test_compile(R"(
import os
from sys import path
)", "Import");

    // Test 17: Global/nonlocal
    test_compile(R"(
x = 10
def foo():
    global x
    x = 20
)", "Global Declaration");

    // Test 18: Walrus operator
    test_compile(R"(
if (n := 10) > 5:
    print(n)
)", "Walrus Operator");

    // Test 19: f-string
    test_compile(R"(
name = "World"
msg = f"Hello, {name}!"
)", "F-String");

    // Test 20: Async function
    test_compile(R"(
async def fetch():
    return 42
)", "Async Function");

    // Test 21: With statement
    test_compile(R"(
with open("file.txt") as f:
    x = f.read()
)", "With Statement");

    // Test 22: With statement without as
    test_compile(R"(
with open("file.txt"):
    x = 1
)", "With Statement (no as)");

    // Test 23: Multiple context managers
    test_compile(R"(
with open("a.txt") as a, open("b.txt") as b:
    data = a.read() + b.read()
)", "Multiple Context Managers");

    // Test 24: Async for loop
    test_compile(R"(
async def process():
    async for item in async_iter():
        print(item)
)", "Async For Loop");

    // Test 25: Async for with else
    test_compile(R"(
async def process():
    async for x in items:
        print(x)
    else:
        print("done")
)", "Async For with Else");

    // Test 26: Simple yield
    test_compile(R"(
def gen():
    yield 1
    yield 2
    yield 3
)", "Simple Yield");

    // Test 27: Yield with value
    test_compile(R"(
def counter(n):
    i = 0
    while i < n:
        yield i
        i = i + 1
)", "Yield in Loop");

    // Test 28: Yield from
    test_compile(R"(
def delegator():
    yield from subgen()
)", "Yield From");

    // Test 29: Yield from with list
    test_compile(R"(
def flatten(items):
    for item in items:
        yield from item
)", "Yield From in Loop");

    // Test 30: Simple list comprehension
    test_compile(R"(
result = [x * 2 for x in range(10)]
)", "List Comprehension");

    // Test 31: List comprehension with condition
    test_compile(R"(
result = [x for x in range(10) if x % 2 == 0]
)", "List Comprehension with Condition");

    // Test 32: Set comprehension
    test_compile(R"(
result = {x * 2 for x in range(10)}
)", "Set Comprehension");

    // Test 33: Dict comprehension
    test_compile(R"(
result = {k: v for k, v in items}
)", "Dict Comprehension");

    // Test 34: Nested list comprehension
    test_compile(R"(
result = [[y for y in x] for x in matrix]
)", "Nested List Comprehension");

    // Test 35: Generator expression
    test_compile(R"(
result = sum(x * 2 for x in range(10))
)", "Generator Expression");

    // Test 36: Simple type alias
    test_compile(R"(
type Vector = list[int]
)", "Simple Type Alias");

    // Test 37: Generic type alias
    test_compile(R"(
type Mapping[K, V] = dict[K, V]
)", "Generic Type Alias");

    // Test 38: Type alias with union
    test_compile(R"(
type Number = int | float
)", "Type Alias with Union");

    std::cout << "=== All tests completed ===\n";
    return 0;
}
