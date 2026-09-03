#ifndef PASLANG_SEMANTIC_HPP
#define PASLANG_SEMANTIC_HPP

#include <string>
#include <unordered_set>
#include <unordered_map>
#include "../ast/ast.hpp"

namespace paslang {

class SemanticAnalyzer {
public:
    SemanticAnalyzer();
    bool analyze(const ProgramAST& program);

private:
    bool analyzeStmt(const StmtAST& stmt);
    bool analyzeExpr(const ExprAST& expr);

    std::unordered_set<std::string> m_definedVars;
    std::unordered_map<std::string, int> m_builtins; // Builtin name -> expected argument count (-1 for variadic)
};

} // namespace paslang

#endif // PASLANG_SEMANTIC_HPP
