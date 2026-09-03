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
    BlockStmt,
    VarDeclStmt,
    AssignmentStmt,
    SayStmt,
    IfStmt,
    RepeatStmt,
    WhileStmt,
    ForInStmt,
    FunctionDeclStmt,
    ReturnStmt,
    ClassDeclStmt,
    ExprStmt,
    NumberExpr,
    StringExpr,
    BoolExpr,
    NullExpr,
    ArrayExpr,
    MapExpr,
    IndexExpr,
    MemberAccessExpr,
    VariableExpr,
    BinaryExpr,
    UnaryExpr,
    CallExpr
};

class ASTNode {
public:
    virtual ~ASTNode() = default;
    virtual ASTNodeType getType() const = 0;
    virtual void dump(int indent = 0) const = 0;
    SourceLocation location;
};

class ExprAST;
class StmtAST;

using ASTNodePtr = std::shared_ptr<ASTNode>;
using ExprASTPtr = std::shared_ptr<ExprAST>;
using StmtASTPtr = std::shared_ptr<StmtAST>;

class ExprAST : public ASTNode {};

class StmtAST : public ASTNode {};

// Block statement
class BlockStmtAST : public StmtAST {
public:
    std::vector<StmtASTPtr> statements;

    ASTNodeType getType() const override { return ASTNodeType::BlockStmt; }
    void dump(int indent = 0) const override {
        std::cout << std::string(indent, ' ') << "BlockStmt:\n";
        for (const auto& stmt : statements) {
            if (stmt) stmt->dump(indent + 2);
        }
    }
};

// Literal number
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

// Literal boolean
class BoolExprAST : public ExprAST {
public:
    bool value;

    BoolExprAST(bool val, SourceLocation loc)
        : value(val) { location = loc; }

    ASTNodeType getType() const override { return ASTNodeType::BoolExpr; }
    void dump(int indent = 0) const override {
        std::cout << std::string(indent, ' ') << "BoolExpr(" << (value ? "true" : "false") << ")\n";
    }
};

// Literal null/nil
class NullExprAST : public ExprAST {
public:
    explicit NullExprAST(SourceLocation loc) { location = loc; }
    ASTNodeType getType() const override { return ASTNodeType::NullExpr; }
    void dump(int indent = 0) const override {
        std::cout << std::string(indent, ' ') << "NullExpr()\n";
    }
};

// Array expression [1, 2, 3]
class ArrayExprAST : public ExprAST {
public:
    std::vector<ExprASTPtr> elements;

    ArrayExprAST(std::vector<ExprASTPtr> elems, SourceLocation loc)
        : elements(std::move(elems)) { location = loc; }

    ASTNodeType getType() const override { return ASTNodeType::ArrayExpr; }
    void dump(int indent = 0) const override {
        std::cout << std::string(indent, ' ') << "ArrayExpr:\n";
        for (const auto& elem : elements) {
            if (elem) elem->dump(indent + 2);
        }
    }
};

// Map expression { name: "Alex", age: 25 }
class MapExprAST : public ExprAST {
public:
    std::vector<std::string> keys;
    std::vector<ExprASTPtr> values;

    MapExprAST(std::vector<std::string> k, std::vector<ExprASTPtr> v, SourceLocation loc)
        : keys(std::move(k)), values(std::move(v)) { location = loc; }

    ASTNodeType getType() const override { return ASTNodeType::MapExpr; }
    void dump(int indent = 0) const override {
        std::cout << std::string(indent, ' ') << "MapExpr:\n";
        for (size_t i = 0; i < keys.size(); ++i) {
            std::cout << std::string(indent + 2, ' ') << keys[i] << ":\n";
            if (i < values.size() && values[i]) values[i]->dump(indent + 4);
        }
    }
};

// Index expression target[index]
class IndexExprAST : public ExprAST {
public:
    ExprASTPtr target;
    ExprASTPtr index;

    IndexExprAST(ExprASTPtr t, ExprASTPtr idx, SourceLocation loc)
        : target(std::move(t)), index(std::move(idx)) { location = loc; }

    ASTNodeType getType() const override { return ASTNodeType::IndexExpr; }
    void dump(int indent = 0) const override {
        std::cout << std::string(indent, ' ') << "IndexExpr:\n";
        if (target) target->dump(indent + 2);
        if (index) index->dump(indent + 2);
    }
};

// Member access obj.member
class MemberAccessExprAST : public ExprAST {
public:
    ExprASTPtr object;
    std::string member;

    MemberAccessExprAST(ExprASTPtr obj, std::string m, SourceLocation loc)
        : object(std::move(obj)), member(std::move(m)) { location = loc; }

    ASTNodeType getType() const override { return ASTNodeType::MemberAccessExpr; }
    void dump(int indent = 0) const override {
        std::cout << std::string(indent, ' ') << "MemberAccess(" << member << "):\n";
        if (object) object->dump(indent + 2);
    }
};

// Variable reference
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

// Unary expression
class UnaryExprAST : public ExprAST {
public:
    std::string op;
    ExprASTPtr operand;

    UnaryExprAST(std::string operatorStr, ExprASTPtr expr, SourceLocation loc)
        : op(std::move(operatorStr)), operand(std::move(expr)) { location = loc; }

    ASTNodeType getType() const override { return ASTNodeType::UnaryExpr; }
    void dump(int indent = 0) const override {
        std::cout << std::string(indent, ' ') << "UnaryExpr(" << op << "):\n";
        if (operand) operand->dump(indent + 2);
    }
};

// Binary expression
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

// Function call
class CallExprAST : public ExprAST {
public:
    std::string callee;
    ExprASTPtr calleeExpr; // for method calls like p.introduce()
    std::vector<ExprASTPtr> args;

    CallExprAST(std::string func, std::vector<ExprASTPtr> a, SourceLocation loc)
        : callee(std::move(func)), args(std::move(a)) { location = loc; }

    CallExprAST(ExprASTPtr funcExpr, std::vector<ExprASTPtr> a, SourceLocation loc)
        : calleeExpr(std::move(funcExpr)), args(std::move(a)) { location = loc; }

    ASTNodeType getType() const override { return ASTNodeType::CallExpr; }
    void dump(int indent = 0) const override {
        std::cout << std::string(indent, ' ') << "CallExpr(" << (calleeExpr ? "method" : callee) << "):\n";
        if (calleeExpr) calleeExpr->dump(indent + 2);
        for (const auto& arg : args) {
            if (arg) arg->dump(indent + 2);
        }
    }
};

// Variable declaration statement
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

// Assignment statement (e.g. x = 10, p.name = "Alex", arr[0] = 5)
class AssignmentStmtAST : public StmtAST {
public:
    ExprASTPtr target;
    ExprASTPtr value;

    AssignmentStmtAST(ExprASTPtr t, ExprASTPtr val, SourceLocation loc)
        : target(std::move(t)), value(std::move(val)) { location = loc; }

    ASTNodeType getType() const override { return ASTNodeType::AssignmentStmt; }
    void dump(int indent = 0) const override {
        std::cout << std::string(indent, ' ') << "AssignmentStmt:\n";
        if (target) target->dump(indent + 2);
        if (value) value->dump(indent + 2);
    }
};

// Say statement
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

// Return statement
class ReturnStmtAST : public StmtAST {
public:
    ExprASTPtr value;

    ReturnStmtAST(ExprASTPtr val, SourceLocation loc)
        : value(std::move(val)) { location = loc; }

    ASTNodeType getType() const override { return ASTNodeType::ReturnStmt; }
    void dump(int indent = 0) const override {
        std::cout << std::string(indent, ' ') << "ReturnStmt:\n";
        if (value) value->dump(indent + 2);
    }
};

// Function declaration statement: function add(a, b): ...
class FunctionDeclStmtAST : public StmtAST {
public:
    std::string name;
    std::vector<std::string> params;
    StmtASTPtr body;

    FunctionDeclStmtAST(std::string n, std::vector<std::string> p, StmtASTPtr b, SourceLocation loc)
        : name(std::move(n)), params(std::move(p)), body(std::move(b)) { location = loc; }

    ASTNodeType getType() const override { return ASTNodeType::FunctionDeclStmt; }
    void dump(int indent = 0) const override {
        std::cout << std::string(indent, ' ') << "FunctionDecl(" << name << "):\n";
        if (body) body->dump(indent + 2);
    }
};

// For In Loop statement: for item in items: ...
class ForInStmtAST : public StmtAST {
public:
    std::string varName;
    ExprASTPtr iterable;
    StmtASTPtr body;

    ForInStmtAST(std::string var, ExprASTPtr iter, StmtASTPtr b, SourceLocation loc)
        : varName(std::move(var)), iterable(std::move(iter)), body(std::move(b)) { location = loc; }

    ASTNodeType getType() const override { return ASTNodeType::ForInStmt; }
    void dump(int indent = 0) const override {
        std::cout << std::string(indent, ' ') << "ForInStmt(for " << varName << " in):\n";
        if (iterable) iterable->dump(indent + 2);
        if (body) body->dump(indent + 2);
    }
};

// Class Declaration Statement: class Person: ...
class ClassDeclStmtAST : public StmtAST {
public:
    std::string name;
    std::vector<std::string> fields;
    std::vector<std::shared_ptr<FunctionDeclStmtAST>> methods;

    ClassDeclStmtAST(std::string n, std::vector<std::string> f, std::vector<std::shared_ptr<FunctionDeclStmtAST>> m, SourceLocation loc)
        : name(std::move(n)), fields(std::move(f)), methods(std::move(m)) { location = loc; }

    ASTNodeType getType() const override { return ASTNodeType::ClassDeclStmt; }
    void dump(int indent = 0) const override {
        std::cout << std::string(indent, ' ') << "ClassDecl(" << name << "):\n";
        for (const auto& method : methods) {
            if (method) method->dump(indent + 2);
        }
    }
};

// If statement
class IfStmtAST : public StmtAST {
public:
    ExprASTPtr condition;
    StmtASTPtr thenBranch;
    StmtASTPtr elseBranch;

    IfStmtAST(ExprASTPtr cond, StmtASTPtr thenStmt, StmtASTPtr elseStmt, SourceLocation loc)
        : condition(std::move(cond)), thenBranch(std::move(thenStmt)), elseBranch(std::move(elseStmt)) { location = loc; }

    ASTNodeType getType() const override { return ASTNodeType::IfStmt; }
    void dump(int indent = 0) const override {
        std::cout << std::string(indent, ' ') << "IfStmt:\n";
        if (condition) condition->dump(indent + 2);
        std::cout << std::string(indent + 2, ' ') << "Then:\n";
        if (thenBranch) thenBranch->dump(indent + 4);
        if (elseBranch) {
            std::cout << std::string(indent + 2, ' ') << "Else:\n";
            elseBranch->dump(indent + 4);
        }
    }
};

// Repeat statement
class RepeatStmtAST : public StmtAST {
public:
    ExprASTPtr countExpr;
    StmtASTPtr body;

    RepeatStmtAST(ExprASTPtr count, StmtASTPtr bodyStmt, SourceLocation loc)
        : countExpr(std::move(count)), body(std::move(bodyStmt)) { location = loc; }

    ASTNodeType getType() const override { return ASTNodeType::RepeatStmt; }
    void dump(int indent = 0) const override {
        std::cout << std::string(indent, ' ') << "RepeatStmt:\n";
        if (countExpr) countExpr->dump(indent + 2);
        std::cout << std::string(indent + 2, ' ') << "Body:\n";
        if (body) body->dump(indent + 4);
    }
};

// While statement
class WhileStmtAST : public StmtAST {
public:
    ExprASTPtr condition;
    StmtASTPtr body;

    WhileStmtAST(ExprASTPtr cond, StmtASTPtr bodyStmt, SourceLocation loc)
        : condition(std::move(cond)), body(std::move(bodyStmt)) { location = loc; }

    ASTNodeType getType() const override { return ASTNodeType::WhileStmt; }
    void dump(int indent = 0) const override {
        std::cout << std::string(indent, ' ') << "WhileStmt:\n";
        if (condition) condition->dump(indent + 2);
        std::cout << std::string(indent + 2, ' ') << "Body:\n";
        if (body) body->dump(indent + 4);
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
