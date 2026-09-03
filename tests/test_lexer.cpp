#include <cassert>
#include <iostream>
#include "../compiler/lexer/lexer.hpp"

void runLexerTests() {
    std::string source = "say \"Hello World\"\nlet x = 10\nlet y = add x 5\nsay y";
    paslang::Lexer lexer(source, "test.pas");
    auto tokens = lexer.tokenize();

    assert(tokens.size() > 0);
    assert(tokens[0].type == paslang::TokenType::KwSay);
    assert(tokens[1].type == paslang::TokenType::StringLit);
    assert(tokens[1].text == "Hello World");

    // line 2: let x = 10
    // find let token
    size_t idx = 0;
    while (idx < tokens.size() && tokens[idx].type != paslang::TokenType::KwLet) idx++;
    assert(idx < tokens.size());
    assert(tokens[idx].type == paslang::TokenType::KwLet);
    assert(tokens[idx + 1].type == paslang::TokenType::Identifier);
    assert(tokens[idx + 1].text == "x");
    assert(tokens[idx + 2].type == paslang::TokenType::Assign);
    assert(tokens[idx + 3].type == paslang::TokenType::NumberInt);
    assert(tokens[idx + 3].text == "10");
}
