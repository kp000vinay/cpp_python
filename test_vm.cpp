#include "src/parser/tokenizer.hpp"
#include "src/parser/parser.hpp"
#include "src/compiler/bytecode_compiler.hpp"
#include "src/vm/vm.hpp"
#include "src/vm/builtins.hpp"
#include <iostream>
#include <string>
#include <memory>

using namespace cpython_cpp;

void test_vm(const std::string& name, const std::string& source) {
    std::cout << "=== Test: " << name << " ===\n";
    std::cout << "Source:\n" << source << "\n\n";
    
    try {
        // Parse
        parser::Parser parser(source);
        auto module = parser.parse();
        
        // Compile
        compiler::BytecodeCompiler compiler;
        auto code = compiler.compile(*module, "<test>");
        
        // Disassemble (optional)
        std::cout << "Bytecode:\n" << code->disassemble() << "\n";
        
        // Execute
        vm::VirtualMachine vm;
        
        // Setup built-in print function
        vm.builtins()->set("print", std::string("<builtin print>"));
        
        std::cout << "Output:\n";
        auto result = vm.execute(code);
        
        std::cout << "\n✓ PASS\n\n";
        
    } catch (const std::exception& e) {
        std::cout << "\n✗ FAIL: " << e.what() << "\n\n";
    }
}

int main() {
    std::cout << "========================================\n";
    std::cout << "  VM Test Suite - Phase 1\n";
    std::cout << "========================================\n\n";
    
    // Test 1: Simple arithmetic
    test_vm("Simple Arithmetic", "x = 1 + 2");
    
    // Test 2: Multiple operations
    test_vm("Multiple Operations", "x = 1 + 2 * 3");
    
    // Test 3: Variable assignment and usage
    test_vm("Variable Usage", R"(
x = 10
y = 20
z = x + y
)");
    
    // Test 4: Print statement
    test_vm("Print Statement", R"(
x = 42
print(x)
)");
    
    // Test 5: Print multiple values
    test_vm("Print Multiple", "print(1, 2, 3)");
    
    // Test 6: String operations
    test_vm("String Concatenation", R"(
s = "Hello" + " " + "World"
print(s)
)");
    
    // Test 7: Boolean operations
    test_vm("Boolean Operations", R"(
a = True
b = False
c = not a
print(a, b, c)
)");
    
    // Test 8: Comparison operations
    test_vm("Comparisons", R"(
x = 10
y = 20
print(x < y)
print(x == y)
print(x > y)
)");
    
    // Test 9: Unary operations
    test_vm("Unary Operations", R"(
x = 42
y = -x
print(y)
)");
    
    // Test 10: Complex expression
    test_vm("Complex Expression", R"(
result = (10 + 5) * 2 - 3
print(result)
)");
    
    // Test 11: Multiple statements
    test_vm("Multiple Statements", R"(
a = 1
b = 2
c = 3
sum = a + b + c
print(sum)
)");
    
    // Test 12: Float operations
    test_vm("Float Operations", R"(
x = 3.14
y = 2.0
z = x * y
print(z)
)");
    
    std::cout << "========================================\n";
    std::cout << "  All tests completed!\n";
    std::cout << "========================================\n";
    
    return 0;
}
