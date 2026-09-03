#include <cassert>
#include <iostream>
#include "../compiler/lexer/lexer.hpp"
#include "../compiler/parser/parser.hpp"

void runParserTests() {
    std::string source = "say \"Hello World\"\nlet x = 10\nlet y = add x 5\nsay y";
    paslang::Lexer lexer(source, "test.pas");
    auto tokens = lexer.tokenize();

    paslang::Parser parser(tokens);
    auto ast = parser.parse();

    assert(ast != nullptr);
    assert(ast->statements.size() == 4);

    // Stmt 0: SayStmt
    assert(ast->statements[0]->getType() == paslang::ASTNodeType::SayStmt);

    // Stmt 1: VarDeclStmt (x = 10)
    assert(ast->statements[1]->getType() == paslang::ASTNodeType::VarDeclStmt);
    auto* varX = static_cast<paslang::VarDeclStmtAST*>(ast->statements[1].get());
    assert(varX->varName == "x");

    // Stmt 2: VarDeclStmt (y = add x 5)
    assert(ast->statements[2]->getType() == paslang::ASTNodeType::VarDeclStmt);
    auto* varY = static_cast<paslang::VarDeclStmtAST*>(ast->statements[2].get());
    assert(varY->varName == "y");
    assert(varY->initializer->getType() == paslang::ASTNodeType::CallExpr);

    auto* callExpr = static_cast<paslang::CallExprAST*>(varY->initializer.get());
    assert(callExpr->callee == "add");
    assert(callExpr->args.size() == 2);
}
