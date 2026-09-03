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
    if (match(TokenType::KwFor)) {
        return parseForInStatement();
    }
    if (match(TokenType::KwFunction)) {
        return parseFunctionDeclaration();
    }
    if (match(TokenType::KwReturn)) {
        return parseReturnStatement();
    }
    if (match(TokenType::KwClass)) {
        return parseClassDeclaration();
    }
    if (!check(TokenType::Newline) && !check(TokenType::Eof)) {
        SourceLocation loc = peek().location;
        if (auto expr = parseExpression()) {
            if (match(TokenType::Assign)) {
                auto val = parseExpression();
                return std::make_shared<AssignmentStmtAST>(std::move(expr), std::move(val), loc);
            }
            return std::make_shared<SayStmtAST>(std::move(expr), loc);
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

    return std::make_shared<VarDeclStmtAST>(varNameTok.text, std::move(initExpr), loc);
}

// say "Hello World" or say y
StmtASTPtr Parser::parseSayStatement() {
    SourceLocation loc = previous().location;
    ExprASTPtr expr = parseExpression();
    if (!expr) {
        Diagnostics::error(loc, "Expected expression after 'say'");
        return nullptr;
    }

    return std::make_shared<SayStmtAST>(std::move(expr), loc);
}

// return <expr>
StmtASTPtr Parser::parseReturnStatement() {
    SourceLocation loc = previous().location;
    ExprASTPtr val = nullptr;
    if (!check(TokenType::Newline) && !check(TokenType::Eof)) {
        val = parseExpression();
    }
    return std::make_shared<ReturnStmtAST>(std::move(val), loc);
}

// function name(param1, param2): ...
StmtASTPtr Parser::parseFunctionDeclaration() {
    SourceLocation loc = previous().location;
    Token nameTok = consume(TokenType::Identifier, "Expected function name after 'function'");

    consume(TokenType::LParen, "Expected '(' after function name");
    std::vector<std::string> params;
    if (!check(TokenType::RParen)) {
        while (true) {
            Token pTok = consume(TokenType::Identifier, "Expected parameter name");
            params.push_back(pTok.text);
            if (!match(TokenType::Comma)) break;
        }
    }
    consume(TokenType::RParen, "Expected ')' after parameter list");
    match(TokenType::Colon);
    match(TokenType::Newline);

    StmtASTPtr body = parseBlockStatement();
    return std::make_shared<FunctionDeclStmtAST>(nameTok.text, std::move(params), std::move(body), loc);
}

// class Person: ...
StmtASTPtr Parser::parseClassDeclaration() {
    SourceLocation loc = previous().location;
    Token classNameTok = consume(TokenType::Identifier, "Expected class name after 'class'");
    match(TokenType::Colon);
    match(TokenType::Newline);

    std::vector<std::string> fields;
    std::vector<std::shared_ptr<FunctionDeclStmtAST>> methods;

    while (!isAtEnd() && !check(TokenType::KwClass) && !check(TokenType::Eof)) {
        if (match(TokenType::Newline)) continue;

        if (match(TokenType::KwFunction)) {
            auto func = parseFunctionDeclaration();
            if (func && func->getType() == ASTNodeType::FunctionDeclStmt) {
                methods.push_back(std::static_pointer_cast<FunctionDeclStmtAST>(func));
            }
        } else if (check(TokenType::Identifier)) {
            Token fieldTok = advance();
            fields.push_back(fieldTok.text);
            match(TokenType::Newline);
        } else {
            break;
        }
    }

    return std::make_shared<ClassDeclStmtAST>(classNameTok.text, std::move(fields), std::move(methods), loc);
}

// for item in items: ...
StmtASTPtr Parser::parseForInStatement() {
    SourceLocation loc = previous().location;
    Token varTok = consume(TokenType::Identifier, "Expected loop variable name after 'for'");
    consume(TokenType::KwIn, "Expected 'in' after loop variable");
    ExprASTPtr iterable = parseExpression();

    match(TokenType::Colon);
    match(TokenType::Newline);

    StmtASTPtr body = parseBlockStatement();
    return std::make_shared<ForInStmtAST>(varTok.text, std::move(iterable), std::move(body), loc);
}

// if condition: ... else: ...
StmtASTPtr Parser::parseIfStatement() {
    SourceLocation loc = previous().location;
    ExprASTPtr condition = parseExpression();
    if (!condition) {
        Diagnostics::error(loc, "Expected condition after 'if'");
        return nullptr;
    }

    match(TokenType::Colon);
    match(TokenType::Newline);

    StmtASTPtr thenBranch = parseBlockStatement();
    StmtASTPtr elseBranch = nullptr;

    while (check(TokenType::Newline)) advance();

    if (match(TokenType::KwElse)) {
        match(TokenType::Colon);
        match(TokenType::Newline);
        elseBranch = parseBlockStatement();
    }

    return std::make_shared<IfStmtAST>(std::move(condition), std::move(thenBranch), std::move(elseBranch), loc);
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
    return std::make_shared<RepeatStmtAST>(std::move(countExpr), std::move(body), loc);
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
    return std::make_shared<WhileStmtAST>(std::move(condition), std::move(body), loc);
}

// Block statement parsing
StmtASTPtr Parser::parseBlockStatement() {
    auto block = std::make_shared<BlockStmtAST>();
    block->location = peek().location;

    while (!isAtEnd() && !check(TokenType::KwElse) && !check(TokenType::Eof)) {
        if (match(TokenType::Newline)) continue;
        auto stmt = parseStatement();
        if (stmt) {
            block->statements.push_back(std::move(stmt));
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
        return std::make_shared<UnaryExprAST>(opTok.text, std::move(operand), opTok.location);
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

        lhs = std::make_shared<BinaryExprAST>(opStr, std::move(lhs), std::move(rhs), opTok.location);
    }
}

ExprASTPtr Parser::parsePrimary() {
    if (isAtEnd()) return nullptr;

    ExprASTPtr expr = nullptr;

    if (check(TokenType::NumberInt) || check(TokenType::NumberFloat)) {
        Token tok = advance();
        double val = std::stod(tok.text);
        bool isFloat = (tok.type == TokenType::NumberFloat);
        expr = std::make_shared<NumberExprAST>(val, isFloat, tok.location);
    }
    else if (check(TokenType::StringLit)) {
        Token tok = advance();
        expr = std::make_shared<StringExprAST>(tok.text, tok.location);
    }
    else if (check(TokenType::BoolLit)) {
        Token tok = advance();
        bool val = (tok.text == "true");
        expr = std::make_shared<BoolExprAST>(val, tok.location);
    }
    else if (check(TokenType::KwNull)) {
        Token tok = advance();
        expr = std::make_shared<NullExprAST>(tok.location);
    }
    else if (check(TokenType::LBracket)) {
        expr = parseArrayExpr();
    }
    else if (check(TokenType::LBrace)) {
        expr = parseMapExpr();
    }
    else if (check(TokenType::Identifier)) {
        expr = parseIdentifierExpr();
    }
    else if (match(TokenType::LParen)) {
        SourceLocation loc = previous().location;
        expr = parseExpression();
        consume(TokenType::RParen, "Expected ')' after expression");
    }

    if (!expr) return nullptr;

    // Handle postfix index arr[i] and member access obj.member
    while (true) {
        if (match(TokenType::LBracket)) {
            SourceLocation loc = previous().location;
            auto indexExpr = parseExpression();
            consume(TokenType::RBracket, "Expected ']' after index expression");
            expr = std::make_shared<IndexExprAST>(std::move(expr), std::move(indexExpr), loc);
        } else if (match(TokenType::Dot)) {
            SourceLocation loc = previous().location;
            Token memberTok = consume(TokenType::Identifier, "Expected field or method name after '.'");
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
                auto memAccess = std::make_shared<MemberAccessExprAST>(std::move(expr), memberTok.text, loc);
                expr = std::make_shared<CallExprAST>(std::move(memAccess), std::move(args), loc);
            } else {
                expr = std::make_shared<MemberAccessExprAST>(std::move(expr), memberTok.text, loc);
            }
        } else {
            break;
        }
    }

    return expr;
}

// Array expression [1, 2, 3]
ExprASTPtr Parser::parseArrayExpr() {
    SourceLocation loc = peek().location;
    consume(TokenType::LBracket, "Expected '['");
    std::vector<ExprASTPtr> elements;
    if (!check(TokenType::RBracket)) {
        while (true) {
            if (auto elem = parseExpression()) {
                elements.push_back(std::move(elem));
            }
            if (!match(TokenType::Comma)) break;
        }
    }
    consume(TokenType::RBracket, "Expected ']' after array elements");
    return std::make_shared<ArrayExprAST>(std::move(elements), loc);
}

// Map expression { name: "Alex", age: 25 }
ExprASTPtr Parser::parseMapExpr() {
    SourceLocation loc = peek().location;
    consume(TokenType::LBrace, "Expected '{'");
    std::vector<std::string> keys;
    std::vector<ExprASTPtr> values;

    if (!check(TokenType::RBrace)) {
        while (true) {
            Token keyTok = consume(TokenType::Identifier, "Expected key identifier in map");
            match(TokenType::Colon);
            auto valExpr = parseExpression();
            keys.push_back(keyTok.text);
            values.push_back(std::move(valExpr));
            if (!match(TokenType::Comma)) break;
        }
    }
    consume(TokenType::RBrace, "Expected '}' after map entries");
    return std::make_shared<MapExprAST>(std::move(keys), std::move(values), loc);
}

ExprASTPtr Parser::parseIdentifierExpr() {
    Token idTok = advance();
    SourceLocation loc = idTok.location;

    // Parenthesized function call syntax: func(a, b)
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
        return std::make_shared<CallExprAST>(idTok.text, std::move(args), loc);
    }

    // Space-separated call syntax for built-in math functions: add x 5, sub a b, mul a b, div a b, mod a b, pow x 2, sqrt x
    bool isSpaceCallFunc = (idTok.text == "add" || idTok.text == "sub" || idTok.text == "mul" || idTok.text == "div" || idTok.text == "mod" || idTok.text == "pow" || idTok.text == "min" || idTok.text == "max" || idTok.text == "sqrt" || idTok.text == "abs");
    if (isSpaceCallFunc && !check(TokenType::Newline) && !check(TokenType::Eof) &&
        (check(TokenType::Identifier) || check(TokenType::NumberInt) || check(TokenType::NumberFloat) || check(TokenType::StringLit))) {
        
        std::vector<ExprASTPtr> args;
        while (!check(TokenType::Newline) && !check(TokenType::Eof) &&
               getTokPrecedence() == -1 && !check(TokenType::RParen) && !check(TokenType::Comma) && !check(TokenType::Colon) && !check(TokenType::LBracket)) {
            
            auto arg = parsePrimary();
            if (!arg) break;
            args.push_back(std::move(arg));
        }

        if (!args.empty()) {
            return std::make_shared<CallExprAST>(idTok.text, std::move(args), loc);
        }
    }

    // Single variable reference
    return std::make_shared<VariableExprAST>(idTok.text, loc);
}

} // namespace paslang
