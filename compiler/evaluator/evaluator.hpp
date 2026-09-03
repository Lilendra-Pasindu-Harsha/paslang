#ifndef PASLANG_EVALUATOR_HPP
#define PASLANG_EVALUATOR_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include <variant>
#include <iostream>
#include "../ast/ast.hpp"

namespace paslang {

enum class ValueType {
    Int,
    Float,
    String,
    Bool,
    Null
};

struct Value {
    ValueType type = ValueType::Null;
    long long intVal = 0;
    double floatVal = 0.0;
    std::string strVal;
    bool boolVal = false;

    static Value makeInt(long long v) {
        Value val; val.type = ValueType::Int; val.intVal = v; return val;
    }
    static Value makeFloat(double v) {
        Value val; val.type = ValueType::Float; val.floatVal = v; return val;
    }
    static Value makeString(std::string v) {
        Value val; val.type = ValueType::String; val.strVal = std::move(v); return val;
    }
    static Value makeBool(bool v) {
        Value val; val.type = ValueType::Bool; val.boolVal = v; return val;
    }
    static Value makeNull() {
        Value val; val.type = ValueType::Null; return val;
    }

    void print(std::ostream& os = std::cout) const {
        switch (type) {
            case ValueType::Int: os << intVal; break;
            case ValueType::Float: os << floatVal; break;
            case ValueType::String: os << strVal; break;
            case ValueType::Bool: os << (boolVal ? "true" : "false"); break;
            case ValueType::Null: os << "null"; break;
        }
    }
};

class Evaluator {
public:
    Evaluator();
    bool execute(const ProgramAST& program);
    Value evalExpr(const ExprAST& expr);

    // Environment getter/setter
    void setVariable(const std::string& name, Value val);
    Value getVariable(const std::string& name, SourceLocation loc);

private:
    void executeStmt(const StmtAST& stmt);
    Value evalCallExpr(const CallExprAST& callExpr);
    Value evalBinaryExpr(const BinaryExprAST& binExpr);

    std::unordered_map<std::string, Value> m_environment;
};

} // namespace paslang

#endif // PASLANG_EVALUATOR_HPP
