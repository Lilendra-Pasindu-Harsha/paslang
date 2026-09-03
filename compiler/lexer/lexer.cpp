#include "lexer.hpp"
#include <cctype>

namespace paslang {

std::string tokenTypeToString(TokenType type) {
    switch (type) {
        case TokenType::KwLet: return "KW_LET";
        case TokenType::KwSay: return "KW_SAY";
        case TokenType::Identifier: return "IDENTIFIER";
        case TokenType::NumberInt: return "NUMBER_INT";
        case TokenType::NumberFloat: return "NUMBER_FLOAT";
        case TokenType::StringLit: return "STRING_LIT";
        case TokenType::BoolLit: return "BOOL_LIT";
        case TokenType::Assign: return "ASSIGN";
        case TokenType::Plus: return "PLUS";
        case TokenType::Minus: return "MINUS";
        case TokenType::Star: return "STAR";
        case TokenType::Slash: return "SLASH";
        case TokenType::LParen: return "LPAREN";
        case TokenType::RParen: return "RPAREN";
        case TokenType::Comma: return "COMMA";
        case TokenType::Newline: return "NEWLINE";
        case TokenType::Eof: return "EOF";
        default: return "UNKNOWN";
    }
}

Lexer::Lexer(std::string source, std::string filename)
    : m_source(std::move(source)), m_filename(std::move(filename)) {}

char Lexer::peek() const {
    if (isAtEnd()) return '\0';
    return m_source[m_cursor];
}

char Lexer::advance() {
    if (isAtEnd()) return '\0';
    char c = m_source[m_cursor++];
    if (c == '\n') {
        m_line++;
        m_column = 1;
    } else {
        m_column++;
    }
    return c;
}

bool Lexer::isAtEnd() const {
    return m_cursor >= m_source.size();
}

void Lexer::skipWhitespace() {
    while (!isAtEnd()) {
        char c = peek();
        if (c == ' ' || c == '\t' || c == '\r') {
            advance();
        } else if (c == '#' || (c == '/' && m_cursor + 1 < m_source.size() && m_source[m_cursor + 1] == '/')) {
            // Line comment
            while (!isAtEnd() && peek() != '\n') {
                advance();
            }
        } else {
            break;
        }
    }
}

Token Lexer::makeToken(TokenType type, std::string text) {
    return Token{type, std::move(text), {m_filename, m_line, m_column}};
}

Token Lexer::lexNumber() {
    size_t startCol = m_column;
    std::string numStr;
    bool isFloat = false;

    while (!isAtEnd() && (std::isdigit(peek()) || peek() == '.')) {
        if (peek() == '.') {
            if (isFloat) break; // second dot
            isFloat = true;
        }
        numStr += advance();
    }

    TokenType type = isFloat ? TokenType::NumberFloat : TokenType::NumberInt;
    return Token{type, numStr, {m_filename, m_line, startCol}};
}

Token Lexer::lexIdentifier() {
    size_t startCol = m_column;
    std::string text;

    while (!isAtEnd() && (std::isalnum(peek()) || peek() == '_')) {
        text += advance();
    }

    TokenType type = TokenType::Identifier;
    if (text == "let") type = TokenType::KwLet;
    else if (text == "say") type = TokenType::KwSay;
    else if (text == "true" || text == "false") type = TokenType::BoolLit;

    return Token{type, text, {m_filename, m_line, startCol}};
}

Token Lexer::lexString() {
    size_t startCol = m_column;
    advance(); // skip opening quote '"'
    std::string str;

    while (!isAtEnd() && peek() != '"') {
        if (peek() == '\\' && m_cursor + 1 < m_source.size()) {
            advance(); // skip '\\'
            char next = advance();
            if (next == 'n') str += '\n';
            else if (next == 't') str += '\t';
            else str += next;
        } else {
            str += advance();
        }
    }

    if (!isAtEnd() && peek() == '"') {
        advance(); // skip closing quote
    } else {
        Diagnostics::error({m_filename, m_line, startCol}, "Unterminated string literal");
    }

    return Token{TokenType::StringLit, str, {m_filename, m_line, startCol}};
}

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;

    while (!isAtEnd()) {
        skipWhitespace();
        if (isAtEnd()) break;

        char c = peek();
        size_t startCol = m_column;

        if (c == '\n') {
            advance();
            // collapse consecutive newlines
            if (tokens.empty() || tokens.back().type != TokenType::Newline) {
                tokens.push_back(Token{TokenType::Newline, "\\n", {m_filename, m_line - 1, startCol}});
            }
            continue;
        }

        if (std::isdigit(c)) {
            tokens.push_back(lexNumber());
        } else if (std::isalpha(c) || c == '_') {
            tokens.push_back(lexIdentifier());
        } else if (c == '"') {
            tokens.push_back(lexString());
        } else {
            advance();
            switch (c) {
                case '=': tokens.push_back(Token{TokenType::Assign, "=", {m_filename, m_line, startCol}}); break;
                case '+': tokens.push_back(Token{TokenType::Plus, "+", {m_filename, m_line, startCol}}); break;
                case '-': tokens.push_back(Token{TokenType::Minus, "-", {m_filename, m_line, startCol}}); break;
                case '*': tokens.push_back(Token{TokenType::Star, "*", {m_filename, m_line, startCol}}); break;
                case '/': tokens.push_back(Token{TokenType::Slash, "/", {m_filename, m_line, startCol}}); break;
                case '(': tokens.push_back(Token{TokenType::LParen, "(", {m_filename, m_line, startCol}}); break;
                case ')': tokens.push_back(Token{TokenType::RParen, ")", {m_filename, m_line, startCol}}); break;
                case ',': tokens.push_back(Token{TokenType::Comma, ",", {m_filename, m_line, startCol}}); break;
                default:
                    Diagnostics::error({m_filename, m_line, startCol}, std::string("Unexpected character: ") + c);
                    tokens.push_back(Token{TokenType::Unknown, std::string(1, c), {m_filename, m_line, startCol}});
                    break;
            }
        }
    }

    tokens.push_back(Token{TokenType::Eof, "", {m_filename, m_line, m_column}});
    return tokens;
}

} // namespace paslang
