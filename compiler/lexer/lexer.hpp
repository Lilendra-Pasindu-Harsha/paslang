#ifndef PASLANG_LEXER_HPP
#define PASLANG_LEXER_HPP

#include <string>
#include <vector>
#include "../diagnostics/diagnostics.hpp"

namespace paslang {

enum class TokenType {
    // Keywords
    KwLet,
    KwSay,

    // Identifiers and Literals
    Identifier,
    NumberInt,
    NumberFloat,
    StringLit,
    BoolLit,

    // Operators and Symbols
    Assign,       // =
    Plus,         // +
    Minus,        // -
    Star,         // *
    Slash,        // /
    LParen,       // (
    RParen,       // )
    Comma,        // ,

    // Formatting
    Newline,
    Eof,
    Unknown
};

struct Token {
    TokenType type;
    std::string text;
    SourceLocation location;
};

class Lexer {
public:
    explicit Lexer(std::string source, std::string filename = "main.pas");
    std::vector<Token> tokenize();

private:
    char peek() const;
    char advance();
    bool isAtEnd() const;
    void skipWhitespace();

    Token makeToken(TokenType type, std::string text);
    Token lexNumber();
    Token lexIdentifier();
    Token lexString();

    std::string m_source;
    std::string m_filename;
    size_t m_cursor = 0;
    size_t m_line = 1;
    size_t m_column = 1;
};

std::string tokenTypeToString(TokenType type);

} // namespace paslang

#endif // PASLANG_LEXER_HPP
