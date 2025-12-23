#include <iostream>
#include <fstream>
#include <sstream>
#include "src/parser/parser.hpp"
#include "src/compiler/compiler.hpp"

int main(int argc, char* argv[]) {
    std::cout << "CPython C++ Parser and Compiler\n";
    std::cout << "===============================\n\n";

    if (argc < 2) {
        std::cerr << "Error: No file provided" << std::endl;
        std::cerr << "Usage: " << argv[0] << " <python_script.py>" << std::endl;
        return 1;
    }

    // Read from file
    std::string filename = argv[1];
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file '" << filename << "'" << std::endl;
        return 1;
    }

    std::ostringstream buffer;
    buffer << file.rdbuf();
    std::string source_code = buffer.str();
    file.close();

    if (source_code.empty()) {
        std::cerr << "Warning: File '" << filename << "' is empty" << std::endl;
    }

    std::cout << "Parsing file: " << filename << "\n";
    std::cout << "Source code:\n";
    std::cout << "---\n" << source_code << "\n---\n\n";

    try {
        // Parse the source code
        cpython_cpp::parser::Parser parser(source_code);
        auto ast_module = parser.parse();

        std::cout << "=== AST Structure ===\n";
        ast_module->print();
        std::cout << "\n";

        // Compile and show breakdown
        cpython_cpp::compiler::Compiler compiler;
        std::string breakdown = compiler.compile(ast_module);
        std::cout << breakdown;

        // Get detailed breakdown
        auto stats = compiler.get_breakdown(ast_module);
        std::cout << "\n=== Summary ===\n";
        std::cout << "Total functions: " << stats.function_count << "\n";
        std::cout << "Total statements: " << stats.statement_count << "\n";
        std::cout << "Total expressions: " << stats.expression_count << "\n";

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
