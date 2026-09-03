#include <cassert>
#include <iostream>
#include <string>
#include "../compiler/lexer/lexer.hpp"
#include "../compiler/parser/parser.hpp"
#include "../compiler/semantic/semantic.hpp"
#include "../compiler/codegen/codegen_c.hpp"

void runCodeGenTests() {
    std::string source = 
        "let pin = 13\n"
        "pinMode pin 1\n"
        "digitalWrite pin 1\n"
        "let val = sin 0.5\n"
        "say val\n";

    paslang::Lexer lexer(source, "codegen_test.pas");
    auto tokens = lexer.tokenize();

    paslang::Parser parser(tokens);
    auto ast = parser.parse();
    assert(ast != nullptr);

    paslang::SemanticAnalyzer semantic;
    assert(semantic.analyze(*ast));

    paslang::CodeGenC codegen;
    std::string cCode = codegen.generate(*ast);

    assert(!cCode.empty());
    assert(cCode.find("pinMode") != std::string::npos);
    assert(cCode.find("digitalWrite") != std::string::npos);
    assert(cCode.find("sin(") != std::string::npos);
    assert(cCode.find("int main") != std::string::npos);
}
