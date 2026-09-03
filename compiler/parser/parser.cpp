#include "parser.hpp"
#include "../diagnostics/diagnostics.hpp"
#include <map>
#include <iostream>

namespace paslang {

static const std::map<TokenType, int> OperatorPrecedence = {
    {TokenType::KwOr, 5},
    {TokenType::KwAnd, 10},
    {TokenType::EqualEqual, 15},
    {TokenType::NotEqual, 15},
    {TokenType::Less, 20},
    {TokenType::Greater, 20},
    {TokenType::LessEqual, 20},
    {TokenType::GreaterEqual, 20},
    {TokenType::Plus, 30},
    {TokenType::Minus, 30},
    {TokenType::Star, 40},
    {TokenType::Slash, 40},
    {TokenType::Percent, 40}
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
            continue; // Skip blank lines
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
    if (match(TokenType::KwIf)) {
        return parseIfStatement();
    }
    if (match(TokenType::KwRepeat)) {
        return parseRepeatStatement();
    }
    if (match(TokenType::KwWhile)) {
        return parseWhileStatement();
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

// if condition: ... else: ...
StmtASTPtr Parser::parseIfStatement() {
    SourceLocation loc = previous().location;
    ExprASTPtr condition = parseExpression();
    if (!condition) {
        Diagnostics::error(loc, "Expected condition after 'if'");
        return nullptr;
    }

    match(TokenType::Colon); // optional colon
    match(TokenType::Newline);

    StmtASTPtr thenBranch = parseBlockStatement();
    StmtASTPtr elseBranch = nullptr;

    while (check(TokenType::Newline)) advance();

    if (match(TokenType::KwElse)) {
        match(TokenType::Colon);
        match(TokenType::Newline);
        elseBranch = parseBlockStatement();
    }

    return std::make_unique<IfStmtAST>(std::move(condition), std::move(thenBranch), std::move(elseBranch), loc);
}

// repeat count: ...
StmtASTPtr Parser::parseRepeatStatement() {
    SourceLocation loc = previous().location;
    ExprASTPtr countExpr = parseExpression();
    if (!countExpr) {
        Diagnostics::error(loc, "Expected repeat count expression");
        return nullptr;
    }

    match(TokenType::Colon);
    match(TokenType::Newline);

    StmtASTPtr body = parseBlockStatement();
    return std::make_unique<RepeatStmtAST>(std::move(countExpr), std::move(body), loc);
}

// while condition: ...
StmtASTPtr Parser::parseWhileStatement() {
    SourceLocation loc = previous().location;
    ExprASTPtr condition = parseExpression();
    if (!condition) {
        Diagnostics::error(loc, "Expected condition after 'while'");
        return nullptr;
    }

    match(TokenType::Colon);
    match(TokenType::Newline);

    StmtASTPtr body = parseBlockStatement();
    return std::make_unique<WhileStmtAST>(std::move(condition), std::move(body), loc);
}

// Block statement parsing
StmtASTPtr Parser::parseBlockStatement() {
    auto block = std::make_unique<BlockStmtAST>();
    block->location = peek().location;

    // Single statement or body statements until blank/dedent/else
    while (!isAtEnd() && !check(TokenType::KwElse) && !check(TokenType::Eof)) {
        if (match(TokenType::Newline)) continue;
        auto stmt = parseStatement();
        if (stmt) {
            block->statements.push_back(std::move(stmt));
            // In single line block or after statement, break if at newline/eof
            if (check(TokenType::Newline) || check(TokenType::Eof) || check(TokenType::KwElse)) break;
        } else {
            break;
        }
    }

    return block;
}

ExprASTPtr Parser::parseExpression() {
    auto lhs = parseUnaryExpr();
    if (!lhs) return nullptr;
    return parseBinaryExpr(0, std::move(lhs));
}

ExprASTPtr Parser::parseUnaryExpr() {
    if (match(TokenType::KwNot) || match(TokenType::Minus)) {
        Token opTok = previous();
        auto operand = parseUnaryExpr();
        if (!operand) return nullptr;
        return std::make_unique<UnaryExprAST>(opTok.text, std::move(operand), opTok.location);
    }
    return parsePrimary();
}

ExprASTPtr Parser::parseBinaryExpr(int exprPrecedence, ExprASTPtr lhs) {
    while (true) {
        int tokPrecedence = getTokPrecedence();
        if (tokPrecedence < exprPrecedence) {
            return lhs;
        }

        Token opTok = advance();
        std::string opStr = opTok.text;

        auto rhs = parseUnaryExpr();
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

    if (check(TokenType::BoolLit)) {
        Token tok = advance();
        bool val = (tok.text == "true");
        return std::make_unique<BoolExprAST>(val, tok.location);
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

    // Check space-separated call syntax for built-in functions: add x 5, sub a b, mul a b, div a b, mod a b, pow x 2
    bool isSpaceCallFunc = (idTok.text == "add" || idTok.text == "sub" || idTok.text == "mul" || idTok.text == "div" || idTok.text == "mod" || idTok.text == "pow" || idTok.text == "min" || idTok.text == "max" || idTok.text == "sqrt" || idTok.text == "abs");
    if (isSpaceCallFunc && !check(TokenType::Newline) && !check(TokenType::Eof) &&
        (check(TokenType::Identifier) || check(TokenType::NumberInt) || check(TokenType::NumberFloat) || check(TokenType::StringLit))) {
        
        std::vector<ExprASTPtr> args;
        while (!check(TokenType::Newline) && !check(TokenType::Eof) &&
               getTokPrecedence() == -1 && !check(TokenType::RParen) && !check(TokenType::Comma) && !check(TokenType::Colon)) {
            
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
