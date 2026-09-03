#include "evaluator.hpp"
#include "../diagnostics/diagnostics.hpp"
#include <cmath>

namespace paslang {

Evaluator::Evaluator() {}

void Evaluator::setVariable(const std::string& name, Value val) {
    m_environment[name] = val;
}

Value Evaluator::getVariable(const std::string& name, SourceLocation loc) {
    auto it = m_environment.find(name);
    if (it != m_environment.end()) {
        return it->second;
    }
    Diagnostics::error(loc, "Runtime Error: Undefined variable '" + name + "'");
    return Value::makeNull();
}

bool Evaluator::execute(const ProgramAST& program) {
    for (const auto& stmt : program.statements) {
        if (stmt) {
            executeStmt(*stmt);
        }
    }
    return true;
}

void Evaluator::executeStmt(const StmtAST& stmt) {
    switch (stmt.getType()) {
        case ASTNodeType::VarDeclStmt: {
            const auto& varDecl = static_cast<const VarDeclStmtAST&>(stmt);
            Value val = Value::makeNull();
            if (varDecl.initializer) {
                val = evalExpr(*varDecl.initializer);
            }
            setVariable(varDecl.varName, val);
            break;
        }
        case ASTNodeType::SayStmt: {
            const auto& sayStmt = static_cast<const SayStmtAST&>(stmt);
            if (sayStmt.expression) {
                Value val = evalExpr(*sayStmt.expression);
                val.print(std::cout);
                std::cout << "\n";
            }
            break;
        }
        default:
            break;
    }
}

Value Evaluator::evalExpr(const ExprAST& expr) {
    switch (expr.getType()) {
        case ASTNodeType::NumberExpr: {
            const auto& numExpr = static_cast<const NumberExprAST&>(expr);
            if (numExpr.isFloat) {
                return Value::makeFloat(numExpr.value);
            } else {
                return Value::makeInt(static_cast<long long>(numExpr.value));
            }
        }
        case ASTNodeType::StringExpr: {
            const auto& strExpr = static_cast<const StringExprAST&>(expr);
            return Value::makeString(strExpr.value);
        }
        case ASTNodeType::VariableExpr: {
            const auto& varExpr = static_cast<const VariableExprAST&>(expr);
            return getVariable(varExpr.name, varExpr.location);
        }
        case ASTNodeType::BinaryExpr: {
            const auto& binExpr = static_cast<const BinaryExprAST&>(expr);
            return evalBinaryExpr(binExpr);
        }
        case ASTNodeType::CallExpr: {
            const auto& callExpr = static_cast<const CallExprAST&>(expr);
            return evalCallExpr(callExpr);
        }
        default:
            return Value::makeNull();
    }
}

Value Evaluator::evalBinaryExpr(const BinaryExprAST& binExpr) {
    Value left = evalExpr(*binExpr.lhs);
    Value right = evalExpr(*binExpr.rhs);

    if (binExpr.op == "+") {
        if (left.type == ValueType::String || right.type == ValueType::String) {
            std::string lStr = (left.type == ValueType::String) ? left.strVal : std::to_string(left.intVal);
            std::string rStr = (right.type == ValueType::String) ? right.strVal : std::to_string(right.intVal);
            return Value::makeString(lStr + rStr);
        }
        if (left.type == ValueType::Float || right.type == ValueType::Float) {
            double l = (left.type == ValueType::Float) ? left.floatVal : left.intVal;
            double r = (right.type == ValueType::Float) ? right.floatVal : right.intVal;
            return Value::makeFloat(l + r);
        }
        return Value::makeInt(left.intVal + right.intVal);
    }
    else if (binExpr.op == "-") {
        if (left.type == ValueType::Float || right.type == ValueType::Float) {
            double l = (left.type == ValueType::Float) ? left.floatVal : left.intVal;
            double r = (right.type == ValueType::Float) ? right.floatVal : right.intVal;
            return Value::makeFloat(l - r);
        }
        return Value::makeInt(left.intVal - right.intVal);
    }
    else if (binExpr.op == "*") {
        if (left.type == ValueType::Float || right.type == ValueType::Float) {
            double l = (left.type == ValueType::Float) ? left.floatVal : left.intVal;
            double r = (right.type == ValueType::Float) ? right.floatVal : right.intVal;
            return Value::makeFloat(l * r);
        }
        return Value::makeInt(left.intVal * right.intVal);
    }
    else if (binExpr.op == "/") {
        double l = (left.type == ValueType::Float) ? left.floatVal : left.intVal;
        double r = (right.type == ValueType::Float) ? right.floatVal : right.intVal;
        if (r == 0) {
            Diagnostics::error(binExpr.location, "Runtime Error: Division by zero");
            return Value::makeNull();
        }
        if (left.type == ValueType::Int && right.type == ValueType::Int && (left.intVal % right.intVal == 0)) {
            return Value::makeInt(left.intVal / right.intVal);
        }
        return Value::makeFloat(l / r);
    }

    return Value::makeNull();
}

Value Evaluator::evalCallExpr(const CallExprAST& callExpr) {
    std::vector<Value> argVals;
    for (const auto& arg : callExpr.args) {
        argVals.push_back(evalExpr(*arg));
    }

    if (callExpr.callee == "add") {
        if (argVals.size() == 2) {
            if (argVals[0].type == ValueType::Float || argVals[1].type == ValueType::Float) {
                double a = (argVals[0].type == ValueType::Float) ? argVals[0].floatVal : argVals[0].intVal;
                double b = (argVals[1].type == ValueType::Float) ? argVals[1].floatVal : argVals[1].intVal;
                return Value::makeFloat(a + b);
            }
            return Value::makeInt(argVals[0].intVal + argVals[1].intVal);
        }
    } else if (callExpr.callee == "sub") {
        if (argVals.size() == 2) {
            return Value::makeInt(argVals[0].intVal - argVals[1].intVal);
        }
    } else if (callExpr.callee == "mul") {
        if (argVals.size() == 2) {
            return Value::makeInt(argVals[0].intVal * argVals[1].intVal);
        }
    } else if (callExpr.callee == "div") {
        if (argVals.size() == 2 && argVals[1].intVal != 0) {
            return Value::makeInt(argVals[0].intVal / argVals[1].intVal);
        }
    }

    Diagnostics::error(callExpr.location, "Runtime Error: Function evaluation failed for '" + callExpr.callee + "'");
    return Value::makeNull();
}

} // namespace paslang
