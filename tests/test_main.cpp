#include <iostream>
#include <cassert>
#include <string>

// Test declarations
void runLexerTests();
void runParserTests();
void runEvaluatorTests();

int main() {
    std::cout << "========================================\n";
    std::cout << "   Running PasLang Compiler v0.1 Tests  \n";
    std::cout << "========================================\n\n";

    try {
        std::cout << "[1/3] Running Lexer Tests...\n";
        runLexerTests();
        std::cout << "      --> Lexer Tests PASSED!\n\n";

        std::cout << "[2/3] Running Parser Tests...\n";
        runParserTests();
        std::cout << "      --> Parser Tests PASSED!\n\n";

        std::cout << "[3/3] Running Evaluator Tests...\n";
        runEvaluatorTests();
        std::cout << "      --> Evaluator Tests PASSED!\n\n";

        std::cout << "ALL TESTS PASSED SUCCESSFULLY! (3/3)\n";
        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "TEST FAILURE: " << ex.what() << "\n";
        return 1;
    }
}
