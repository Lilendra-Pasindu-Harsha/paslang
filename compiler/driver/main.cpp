#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include "../lexer/lexer.hpp"
#include "../parser/parser.hpp"
#include "../semantic/semantic.hpp"
#include "../evaluator/evaluator.hpp"

void printUsage() {
    std::cout << "PasLang Compiler v0.1\n"
              << "Usage: paslang [options] <source_file.pas>\n\n"
              << "Options:\n"
              << "  --ast        Dump Abstract Syntax Tree (AST)\n"
              << "  --version    Display PasLang version\n"
              << "  --help       Show this help message\n";
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printUsage();
        return 1;
    }

    std::string filename;
    bool dumpAST = false;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--help" || arg == "-h") {
            printUsage();
            return 0;
        } else if (arg == "--version" || arg == "-v") {
            std::cout << "PasLang Compiler v0.1 (Modern C++ Architecture)\n";
            return 0;
        } else if (arg == "--ast") {
            dumpAST = true;
        } else if (arg[0] != '-') {
            filename = arg;
        } else {
            std::cerr << "Unknown flag: " << arg << "\n";
            printUsage();
            return 1;
        }
    }

    if (filename.empty()) {
        std::cerr << "Error: No input source file specified.\n";
        printUsage();
        return 1;
    }

    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file '" << filename << "'\n";
        return 1;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string source = buffer.str();
    file.close();

    // 1. Lexical Analysis
    paslang::Lexer lexer(source, filename);
    std::vector<paslang::Token> tokens = lexer.tokenize();

    // 2. Syntactic Analysis (Parsing)
    paslang::Parser parser(tokens);
    auto ast = parser.parse();

    if (!ast) {
        std::cerr << "Compilation failed during parsing.\n";
        return 1;
    }

    if (dumpAST) {
        ast->dump();
        return 0;
    }

    // 3. Semantic Analysis
    paslang::SemanticAnalyzer semantic;
    if (!semantic.analyze(*ast)) {
        std::cerr << "Compilation failed during semantic analysis.\n";
        return 1;
    }

    // 4. Execution / Evaluation
    paslang::Evaluator evaluator;
    if (!evaluator.execute(*ast)) {
        std::cerr << "Runtime evaluation error.\n";
        return 1;
    }

    return 0;
}
