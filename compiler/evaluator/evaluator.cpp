#include "evaluator.hpp"
#include "../diagnostics/diagnostics.hpp"
#include <cmath>
#include <algorithm>

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
        case ASTNodeType::BlockStmt: {
            const auto& block = static_cast<const BlockStmtAST&>(stmt);
            for (const auto& childStmt : block.statements) {
                if (childStmt) executeStmt(*childStmt);
            }
            break;
        }
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
        case ASTNodeType::IfStmt: {
            const auto& ifStmt = static_cast<const IfStmtAST&>(stmt);
            Value condVal = Value::makeBool(false);
            if (ifStmt.condition) {
                condVal = evalExpr(*ifStmt.condition);
            }
            if (condVal.isTruthy()) {
                if (ifStmt.thenBranch) executeStmt(*ifStmt.thenBranch);
            } else {
                if (ifStmt.elseBranch) executeStmt(*ifStmt.elseBranch);
            }
            break;
        }
        case ASTNodeType::RepeatStmt: {
            const auto& repStmt = static_cast<const RepeatStmtAST&>(stmt);
            Value countVal = Value::makeInt(0);
            if (repStmt.countExpr) {
                countVal = evalExpr(*repStmt.countExpr);
            }
            long long count = countVal.intVal;
            for (long long i = 0; i < count; ++i) {
                if (repStmt.body) executeStmt(*repStmt.body);
            }
            break;
        }
        case ASTNodeType::WhileStmt: {
            const auto& whileStmt = static_cast<const WhileStmtAST&>(stmt);
            while (true) {
                Value condVal = Value::makeBool(false);
                if (whileStmt.condition) {
                    condVal = evalExpr(*whileStmt.condition);
                }
                if (!condVal.isTruthy()) break;
                if (whileStmt.body) executeStmt(*whileStmt.body);
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
        case ASTNodeType::BoolExpr: {
            const auto& boolExpr = static_cast<const BoolExprAST&>(expr);
            return Value::makeBool(boolExpr.value);
        }
        case ASTNodeType::VariableExpr: {
            const auto& varExpr = static_cast<const VariableExprAST&>(expr);
            return getVariable(varExpr.name, varExpr.location);
        }
        case ASTNodeType::UnaryExpr: {
            const auto& unExpr = static_cast<const UnaryExprAST&>(expr);
            return evalUnaryExpr(unExpr);
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

Value Evaluator::evalUnaryExpr(const UnaryExprAST& unExpr) {
    Value val = evalExpr(*unExpr.operand);
    if (unExpr.op == "not") {
        return Value::makeBool(!val.isTruthy());
    } else if (unExpr.op == "-") {
        if (val.type == ValueType::Float) return Value::makeFloat(-val.floatVal);
        return Value::makeInt(-val.intVal);
    }
    return Value::makeNull();
}

Value Evaluator::evalBinaryExpr(const BinaryExprAST& binExpr) {
    Value left = evalExpr(*binExpr.lhs);
    Value right = evalExpr(*binExpr.rhs);

    // Arithmetic
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
    else if (binExpr.op == "%") {
        if (right.intVal == 0) {
            Diagnostics::error(binExpr.location, "Runtime Error: Modulo by zero");
            return Value::makeNull();
        }
        return Value::makeInt(left.intVal % right.intVal);
    }

    // Comparison Operators
    else if (binExpr.op == "==") {
        if (left.type == ValueType::Int && right.type == ValueType::Int) return Value::makeBool(left.intVal == right.intVal);
        if (left.type == ValueType::String && right.type == ValueType::String) return Value::makeBool(left.strVal == right.strVal);
        double l = (left.type == ValueType::Float) ? left.floatVal : left.intVal;
        double r = (right.type == ValueType::Float) ? right.floatVal : right.intVal;
        return Value::makeBool(l == r);
    }
    else if (binExpr.op == "!=") {
        Value eq = evalBinaryExpr(BinaryExprAST("==", std::make_unique<NumberExprAST>(0, false, binExpr.location), std::make_unique<NumberExprAST>(0, false, binExpr.location), binExpr.location));
        // Simple inequality check
        if (left.type == ValueType::Int && right.type == ValueType::Int) return Value::makeBool(left.intVal != right.intVal);
        if (left.type == ValueType::String && right.type == ValueType::String) return Value::makeBool(left.strVal != right.strVal);
        double l = (left.type == ValueType::Float) ? left.floatVal : left.intVal;
        double r = (right.type == ValueType::Float) ? right.floatVal : right.intVal;
        return Value::makeBool(l != r);
    }
    else if (binExpr.op == "<") {
        double l = (left.type == ValueType::Float) ? left.floatVal : left.intVal;
        double r = (right.type == ValueType::Float) ? right.floatVal : right.intVal;
        return Value::makeBool(l < r);
    }
    else if (binExpr.op == ">") {
        double l = (left.type == ValueType::Float) ? left.floatVal : left.intVal;
        double r = (right.type == ValueType::Float) ? right.floatVal : right.intVal;
        return Value::makeBool(l > r);
    }
    else if (binExpr.op == "<=") {
        double l = (left.type == ValueType::Float) ? left.floatVal : left.intVal;
        double r = (right.type == ValueType::Float) ? right.floatVal : right.intVal;
        return Value::makeBool(l <= r);
    }
    else if (binExpr.op == ">=") {
        double l = (left.type == ValueType::Float) ? left.floatVal : left.intVal;
        double r = (right.type == ValueType::Float) ? right.floatVal : right.intVal;
        return Value::makeBool(l >= r);
    }

    // Logic
    else if (binExpr.op == "and") {
        return Value::makeBool(left.isTruthy() && right.isTruthy());
    }
    else if (binExpr.op == "or") {
        return Value::makeBool(left.isTruthy() || right.isTruthy());
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
    } else if (callExpr.callee == "mod") {
        if (argVals.size() == 2 && argVals[1].intVal != 0) {
            return Value::makeInt(argVals[0].intVal % argVals[1].intVal);
        }
    } else if (callExpr.callee == "pow") {
        if (argVals.size() == 2) {
            double base = (argVals[0].type == ValueType::Float) ? argVals[0].floatVal : argVals[0].intVal;
            double exp = (argVals[1].type == ValueType::Float) ? argVals[1].floatVal : argVals[1].intVal;
            return Value::makeFloat(std::pow(base, exp));
        }
    } else if (callExpr.callee == "sqrt") {
        if (argVals.size() == 1) {
            double val = (argVals[0].type == ValueType::Float) ? argVals[0].floatVal : argVals[0].intVal;
            return Value::makeFloat(std::sqrt(val));
        }
    } else if (callExpr.callee == "abs") {
        if (argVals.size() == 1) {
            if (argVals[0].type == ValueType::Float) return Value::makeFloat(std::abs(argVals[0].floatVal));
            return Value::makeInt(std::abs(argVals[0].intVal));
        }
    } else if (callExpr.callee == "min") {
        if (argVals.size() == 2) {
            double a = (argVals[0].type == ValueType::Float) ? argVals[0].floatVal : argVals[0].intVal;
            double b = (argVals[1].type == ValueType::Float) ? argVals[1].floatVal : argVals[1].intVal;
            return Value::makeFloat(std::min(a, b));
        }
    } else if (callExpr.callee == "max") {
        if (argVals.size() == 2) {
            double a = (argVals[0].type == ValueType::Float) ? argVals[0].floatVal : argVals[0].intVal;
            double b = (argVals[1].type == ValueType::Float) ? argVals[1].floatVal : argVals[1].intVal;
            return Value::makeFloat(std::max(a, b));
        }
    }

    Diagnostics::error(callExpr.location, "Runtime Error: Function evaluation failed for '" + callExpr.callee + "'");
    return Value::makeNull();
}

} // namespace paslang
