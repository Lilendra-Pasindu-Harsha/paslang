#include <cassert>
#include <iostream>
#include <sstream>
#include <cmath>
#include "../compiler/lexer/lexer.hpp"
#include "../compiler/parser/parser.hpp"
#include "../compiler/semantic/semantic.hpp"
#include "../compiler/evaluator/evaluator.hpp"

void runEvaluatorTests() {
    // 1. Math and built-ins test
    std::string source = 
        "let x = 10\n"
        "let y = add x 5\n"
        "let z = mul y 2\n"
        "let p = pow 2 3\n"
        "let s = sin 0\n"
        "let c = cos 0\n";

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

    paslang::Value cVal = evaluator.getVariable("c", {"test.pas", 1, 1});
    assert(cVal.floatVal == 1.0);

    // 2. Matrix, Equation Solver and ML Test
    std::string mlSource =
        "let A = matrix 2 2 1.0\n"
        "let quad = solve_quadratic 1 -5 6\n"
        "let act = relu -5.5\n"
        "let act2 = relu 10.0\n"
        "let sig = sigmoid 0.0\n";

    paslang::Lexer lexer2(mlSource, "test_ml.pas");
    auto tokens2 = lexer2.tokenize();
    paslang::Parser parser2(tokens2);
    auto ast2 = parser2.parse();
    assert(ast2 != nullptr);
    assert(semantic.analyze(*ast2));

    paslang::Evaluator evaluator2;
    assert(evaluator2.execute(*ast2));

    paslang::Value matVal = evaluator2.getVariable("A", {"test_ml.pas", 1, 1});
    assert(matVal.type == paslang::ValueType::Matrix);
    assert(matVal.matrixVal->rows == 2 && matVal.matrixVal->cols == 2);

    paslang::Value quadVal = evaluator2.getVariable("quad", {"test_ml.pas", 1, 1});
    assert(quadVal.type == paslang::ValueType::Array);
    assert(quadVal.arrayVal->size() == 2);
    assert((*quadVal.arrayVal)[0].floatVal == 3.0);
    assert((*quadVal.arrayVal)[1].floatVal == 2.0);

    paslang::Value actVal = evaluator2.getVariable("act", {"test_ml.pas", 1, 1});
    assert(actVal.floatVal == 0.0);

    paslang::Value sigVal = evaluator2.getVariable("sig", {"test_ml.pas", 1, 1});
    assert(sigVal.floatVal == 0.5);
}
