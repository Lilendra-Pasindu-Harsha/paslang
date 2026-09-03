#ifndef PASLANG_AST_HPP
#define PASLANG_AST_HPP

#include <string>
#include <vector>
#include <memory>
#include <iostream>
#include "../diagnostics/diagnostics.hpp"

namespace paslang {

enum class ASTNodeType {
    Program,
    VarDeclStmt,
    SayStmt,
    ExprStmt,
    NumberExpr,
    StringExpr,
    VariableExpr,
    BinaryExpr,
    CallExpr
};

class ASTVisitor;

class ASTNode {
public:
    virtual ~ASTNode() = default;
    virtual ASTNodeType getType() const = 0;
    virtual void dump(int indent = 0) const = 0;
    SourceLocation location;
};

using ASTNodePtr = std::unique_ptr<ASTNode>;

class ExprAST : public ASTNode {};
using ExprASTPtr = std::unique_ptr<ExprAST>;

class StmtAST : public ASTNode {};
using StmtASTPtr = std::unique_ptr<StmtAST>;

// Literal number (int or float)
class NumberExprAST : public ExprAST {
public:
    double value;
    bool isFloat;

    NumberExprAST(double val, bool isF, SourceLocation loc)
        : value(val), isFloat(isF) { location = loc; }

    ASTNodeType getType() const override { return ASTNodeType::NumberExpr; }
    void dump(int indent = 0) const override {
        std::cout << std::string(indent, ' ') << "NumberExpr(" << value << ")\n";
    }
};

// Literal string
class StringExprAST : public ExprAST {
public:
    std::string value;

    StringExprAST(std::string val, SourceLocation loc)
        : value(std::move(val)) { location = loc; }

    ASTNodeType getType() const override { return ASTNodeType::StringExpr; }
    void dump(int indent = 0) const override {
        std::cout << std::string(indent, ' ') << "StringExpr(\"" << value << "\")\n";
    }
};

// Variable reference (e.g. x)
class VariableExprAST : public ExprAST {
public:
    std::string name;

    VariableExprAST(std::string n, SourceLocation loc)
        : name(std::move(n)) { location = loc; }

    ASTNodeType getType() const override { return ASTNodeType::VariableExpr; }
    void dump(int indent = 0) const override {
        std::cout << std::string(indent, ' ') << "VariableExpr(" << name << ")\n";
    }
};

// Binary operation (+, -, *, /)
class BinaryExprAST : public ExprAST {
public:
    std::string op;
    ExprASTPtr lhs;
    ExprASTPtr rhs;

    BinaryExprAST(std::string operatorChar, ExprASTPtr l, ExprASTPtr r, SourceLocation loc)
        : op(std::move(operatorChar)), lhs(std::move(l)), rhs(std::move(r)) { location = loc; }

    ASTNodeType getType() const override { return ASTNodeType::BinaryExpr; }
    void dump(int indent = 0) const override {
        std::cout << std::string(indent, ' ') << "BinaryExpr(" << op << "):\n";
        if (lhs) lhs->dump(indent + 2);
        if (rhs) rhs->dump(indent + 2);
    }
};

// Function call (e.g. add x 5)
class CallExprAST : public ExprAST {
public:
    std::string callee;
    std::vector<ExprASTPtr> args;

    CallExprAST(std::string func, std::vector<ExprASTPtr> a, SourceLocation loc)
        : callee(std::move(func)), args(std::move(a)) { location = loc; }

    ASTNodeType getType() const override { return ASTNodeType::CallExpr; }
    void dump(int indent = 0) const override {
        std::cout << std::string(indent, ' ') << "CallExpr(" << callee << "):\n";
        for (const auto& arg : args) {
            if (arg) arg->dump(indent + 2);
        }
    }
};

// Variable declaration statement: let x = 10 or let y = add x 5
class VarDeclStmtAST : public StmtAST {
public:
    std::string varName;
    ExprASTPtr initializer;

    VarDeclStmtAST(std::string name, ExprASTPtr init, SourceLocation loc)
        : varName(std::move(name)), initializer(std::move(init)) { location = loc; }

    ASTNodeType getType() const override { return ASTNodeType::VarDeclStmt; }
    void dump(int indent = 0) const override {
        std::cout << std::string(indent, ' ') << "VarDeclStmt(let " << varName << " =):\n";
        if (initializer) initializer->dump(indent + 2);
    }
};

// Say statement: say "Hello World" or say y
class SayStmtAST : public StmtAST {
public:
    ExprASTPtr expression;

    SayStmtAST(ExprASTPtr expr, SourceLocation loc)
        : expression(std::move(expr)) { location = loc; }

    ASTNodeType getType() const override { return ASTNodeType::SayStmt; }
    void dump(int indent = 0) const override {
        std::cout << std::string(indent, ' ') << "SayStmt:\n";
        if (expression) expression->dump(indent + 2);
    }
};

// Program AST Node (root)
class ProgramAST : public ASTNode {
public:
    std::vector<StmtASTPtr> statements;

    ASTNodeType getType() const override { return ASTNodeType::Program; }
    void dump(int indent = 0) const override {
        std::cout << std::string(indent, ' ') << "ProgramAST:\n";
        for (const auto& stmt : statements) {
            if (stmt) stmt->dump(indent + 2);
        }
    }
};

} // namespace paslang

#endif // PASLANG_AST_HPP
