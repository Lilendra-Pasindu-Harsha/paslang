#include "parser.hpp"
#include "../diagnostics/diagnostics.hpp"
#include <map>
#include <iostream>

namespace paslang {

static const std::map<TokenType, int> OperatorPrecedence = {
    {TokenType::Plus, 20},
    {TokenType::Minus, 20},
    {TokenType::Star, 40},
    {TokenType::Slash, 40}
};

Parser::Parser(std::vector<Token> tokens) : m_tokens(std::move(tokens)) {}

const Token& Parser::peek() const {
    if (m_current >= m_tokens.size()) return m_tokens.back();
    return m_tokens[m_current];
}

const Token& Parser::previous() const {
    if (m_current == 0) return m_tokens[0];
    if (m_current - 1 >= m_tokens.size()) return m_tokens.back();
    return m_tokens[m_current - 1];
}

const Token& Parser::advance() {
    if (!isAtEnd()) m_current++;
    return previous();
}

bool Parser::isAtEnd() const {
    if (m_current >= m_tokens.size()) return true;
    return m_tokens[m_current].type == TokenType::Eof;
}

bool Parser::check(TokenType type) const {
    if (isAtEnd()) return false;
    return peek().type == type;
}

bool Parser::match(TokenType type) {
    if (check(type)) {
        advance();
        return true;
    }
    return false;
}

Token Parser::consume(TokenType type, const std::string& message, const std::string& hint) {
    if (check(type)) return advance();
    Diagnostics::error(peek().location, message, hint);
    return peek();
}

int Parser::getTokPrecedence() const {
    auto it = OperatorPrecedence.find(peek().type);
    if (it != OperatorPrecedence.end()) {
        return it->second;
    }
    return -1;
}

std::unique_ptr<ProgramAST> Parser::parse() {
    auto program = std::make_unique<ProgramAST>();
    program->location = peek().location;

    while (!isAtEnd()) {
        if (match(TokenType::Newline)) {
            continue; // Skip extra blank lines
        }
        auto stmt = parseStatement();
        if (stmt) {
            program->statements.push_back(std::move(stmt));
        } else {
            if (!isAtEnd()) advance();
        }
    }

    return program;
}

StmtASTPtr Parser::parseStatement() {
    if (isAtEnd()) return nullptr;
    if (match(TokenType::KwLet)) {
        return parseLetStatement();
    }
    if (match(TokenType::KwSay)) {
        return parseSayStatement();
    }
    if (!check(TokenType::Newline) && !check(TokenType::Eof)) {
        if (auto expr = parseExpression()) {
            return std::make_unique<SayStmtAST>(std::move(expr), previous().location);
        }
    }
    return nullptr;
}

// let x = 10 or let y = add x 5
StmtASTPtr Parser::parseLetStatement() {
    SourceLocation loc = previous().location;
    Token varNameTok = consume(TokenType::Identifier, "Expected variable name after 'let'", "e.g. let name = \"PasLang\"");
    
    consume(TokenType::Assign, "Expected '=' in variable declaration", "e.g. let " + varNameTok.text + " = value");
    
    ExprASTPtr initExpr = parseExpression();
    if (!initExpr) {
        Diagnostics::error(loc, "Expected expression after '='");
        return nullptr;
    }

    return std::make_unique<VarDeclStmtAST>(varNameTok.text, std::move(initExpr), loc);
}

// say "Hello World" or say y
StmtASTPtr Parser::parseSayStatement() {
    SourceLocation loc = previous().location;
    ExprASTPtr expr = parseExpression();
    if (!expr) {
        Diagnostics::error(loc, "Expected expression after 'say'");
        return nullptr;
    }

    return std::make_unique<SayStmtAST>(std::move(expr), loc);
}

ExprASTPtr Parser::parseExpression() {
    auto lhs = parsePrimary();
    if (!lhs) return nullptr;
    return parseBinaryExpr(0, std::move(lhs));
}

ExprASTPtr Parser::parseBinaryExpr(int exprPrecedence, ExprASTPtr lhs) {
    while (true) {
        int tokPrecedence = getTokPrecedence();
        if (tokPrecedence < exprPrecedence) {
            return lhs;
        }

        Token opTok = advance();
        std::string opStr = opTok.text;

        auto rhs = parsePrimary();
        if (!rhs) return nullptr;

        int nextPrecedence = getTokPrecedence();
        if (tokPrecedence < nextPrecedence) {
            rhs = parseBinaryExpr(tokPrecedence + 1, std::move(rhs));
            if (!rhs) return nullptr;
        }

        lhs = std::make_unique<BinaryExprAST>(opStr, std::move(lhs), std::move(rhs), opTok.location);
    }
}

ExprASTPtr Parser::parsePrimary() {
    if (isAtEnd()) return nullptr;

    if (check(TokenType::NumberInt) || check(TokenType::NumberFloat)) {
        Token tok = advance();
        double val = std::stod(tok.text);
        bool isFloat = (tok.type == TokenType::NumberFloat);
        return std::make_unique<NumberExprAST>(val, isFloat, tok.location);
    }

    if (check(TokenType::StringLit)) {
        Token tok = advance();
        return std::make_unique<StringExprAST>(tok.text, tok.location);
    }

    if (check(TokenType::Identifier)) {
        return parseIdentifierExpr();
    }

    if (match(TokenType::LParen)) {
        SourceLocation loc = previous().location;
        auto expr = parseExpression();
        consume(TokenType::RParen, "Expected ')' after expression");
        return expr;
    }

    Diagnostics::error(peek().location, "Unexpected token '" + peek().text + "' in expression");
    return nullptr;
}

ExprASTPtr Parser::parseIdentifierExpr() {
    Token idTok = advance();
    SourceLocation loc = idTok.location;

    // Check parenthesized call syntax: function_name(arg1, arg2)
    if (match(TokenType::LParen)) {
        std::vector<ExprASTPtr> args;
        if (!check(TokenType::RParen)) {
            while (true) {
                if (auto arg = parseExpression()) {
                    args.push_back(std::move(arg));
                }
                if (!match(TokenType::Comma)) break;
            }
        }
        consume(TokenType::RParen, "Expected ')' after argument list");
        return std::make_unique<CallExprAST>(idTok.text, std::move(args), loc);
    }

    // Check space-separated call syntax for built-in functions: add x 5
    bool isSpaceCallFunc = (idTok.text == "add" || idTok.text == "sub" || idTok.text == "mul" || idTok.text == "div");
    if (isSpaceCallFunc && !check(TokenType::Newline) && !check(TokenType::Eof) &&
        (check(TokenType::Identifier) || check(TokenType::NumberInt) || check(TokenType::NumberFloat) || check(TokenType::StringLit))) {
        
        std::vector<ExprASTPtr> args;
        while (!check(TokenType::Newline) && !check(TokenType::Eof) &&
               getTokPrecedence() == -1 && !check(TokenType::RParen) && !check(TokenType::Comma)) {
            
            auto arg = parsePrimary();
            if (!arg) break;
            args.push_back(std::move(arg));
        }

        if (!args.empty()) {
            return std::make_unique<CallExprAST>(idTok.text, std::move(args), loc);
        }
    }

    // Single variable reference
    return std::make_unique<VariableExprAST>(idTok.text, loc);
}

} // namespace paslang
