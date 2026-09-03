#include <cassert>
#include <iostream>
#include <sstream>
#include "../compiler/lexer/lexer.hpp"
#include "../compiler/parser/parser.hpp"
#include "../compiler/semantic/semantic.hpp"
#include "../compiler/evaluator/evaluator.hpp"

void runEvaluatorTests() {
    // 1. Math and built-ins test
    std::string source = "let x = 10\nlet y = add x 5\nlet z = mul y 2\nlet p = pow 2 3";
    paslang::Lexer lexer(source, "test.pas");
    auto tokens = lexer.tokenize();

    paslang::Parser parser(tokens);
    auto ast = parser.parse();
    assert(ast != nullptr);

    paslang::SemanticAnalyzer semantic;
    assert(semantic.analyze(*ast));

    paslang::Evaluator evaluator;
    assert(evaluator.execute(*ast));

    paslang::Value yVal = evaluator.getVariable("y", {"test.pas", 1, 1});
    assert(yVal.type == paslang::ValueType::Int);
    assert(yVal.intVal == 15);

    paslang::Value zVal = evaluator.getVariable("z", {"test.pas", 1, 1});
    assert(zVal.intVal == 30);

    paslang::Value pVal = evaluator.getVariable("p", {"test.pas", 1, 1});
    assert(pVal.floatVal == 8.0);
}
