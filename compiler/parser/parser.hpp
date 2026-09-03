#ifndef PASLANG_PARSER_HPP
#define PASLANG_PARSER_HPP

#include <vector>
#include <memory>
#include "../lexer/lexer.hpp"
#include "../ast/ast.hpp"

namespace paslang {

class Parser {
public:
    explicit Parser(std::vector<Token> tokens);
    std::unique_ptr<ProgramAST> parse();

private:
    const Token& peek() const;
    const Token& previous() const;
    const Token& advance();
    bool isAtEnd() const;
    bool check(TokenType type) const;
    bool match(TokenType type);
    Token consume(TokenType type, const std::string& message, const std::string& hint = "");

    // Parsing methods
    StmtASTPtr parseStatement();
    StmtASTPtr parseLetStatement();
    StmtASTPtr parseSayStatement();
    StmtASTPtr parseIfStatement();
    StmtASTPtr parseRepeatStatement();
    StmtASTPtr parseWhileStatement();
    StmtASTPtr parseBlockStatement();

    ExprASTPtr parseExpression();
    ExprASTPtr parseBinaryExpr(int exprPrecedence, ExprASTPtr lhs);
    ExprASTPtr parseUnaryExpr();
    ExprASTPtr parsePrimary();
    ExprASTPtr parseIdentifierExpr();

    int getTokPrecedence() const;

    std::vector<Token> m_tokens;
    size_t m_current = 0;
};

} // namespace paslang

#endif // PASLANG_PARSER_HPP
