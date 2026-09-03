#include <iostream>
#include <cassert>
#include <string>

// Test declarations
void runLexerTests();
void runParserTests();
void runEvaluatorTests();
void runCodeGenTests();

int main() {
    std::cout << "========================================\n";
    std::cout << "   Running PasLang Compiler v1.0 Tests  \n";
    std::cout << "========================================\n\n";

    try {
        std::cout << "[1/4] Running Lexer Tests...\n";
        runLexerTests();
        std::cout << "      --> Lexer Tests PASSED!\n\n";

        std::cout << "[2/4] Running Parser Tests...\n";
        runParserTests();
        std::cout << "      --> Parser Tests PASSED!\n\n";

        std::cout << "[3/4] Running Evaluator (Math, Equation Solver & ML) Tests...\n";
        runEvaluatorTests();
        std::cout << "      --> Evaluator Tests PASSED!\n\n";

        std::cout << "[4/4] Running C Code Generator (Embedded / ESP32 Target) Tests...\n";
        runCodeGenTests();
        std::cout << "      --> CodeGen Tests PASSED!\n\n";

        std::cout << "ALL TESTS PASSED SUCCESSFULLY! (4/4)\n";
        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "TEST FAILURE: " << ex.what() << "\n";
        return 1;
    }
}
