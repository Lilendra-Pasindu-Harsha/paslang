#include "evaluator.hpp"
#include "../diagnostics/diagnostics.hpp"
#include <cmath>
#include <algorithm>
#include <sstream>

namespace paslang {

Evaluator::Evaluator() {
    m_env = std::make_shared<Environment>();
}

void Evaluator::setVariable(const std::string& name, Value val) {
    m_env->set(name, std::move(val));
}

Value Evaluator::getVariable(const std::string& name, SourceLocation loc) {
    bool found = false;
    Value val = m_env->get(name, found);
    if (found) return val;

    Diagnostics::error(loc, "Runtime Error: Undefined variable '" + name + "'");
    return Value::makeNull();
}

bool Evaluator::execute(const ProgramAST& program) {
    try {
        for (const auto& stmt : program.statements) {
            if (stmt) {
                executeStmt(*stmt);
            }
        }
    } catch (const ReturnSignal& sig) {
        // Top level return ignored
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
            m_env->set(varDecl.varName, val);
            break;
        }
        case ASTNodeType::AssignmentStmt: {
            const auto& assignStmt = static_cast<const AssignmentStmtAST&>(stmt);
            Value rhsVal = Value::makeNull();
            if (assignStmt.value) rhsVal = evalExpr(*assignStmt.value);

            if (assignStmt.target) {
                if (assignStmt.target->getType() == ASTNodeType::VariableExpr) {
                    const auto& varExpr = static_cast<const VariableExprAST&>(*assignStmt.target);
                    if (!m_env->update(varExpr.name, rhsVal)) {
                        m_env->set(varExpr.name, rhsVal);
                    }
                } else if (assignStmt.target->getType() == ASTNodeType::MemberAccessExpr) {
                    const auto& memExpr = static_cast<const MemberAccessExprAST&>(*assignStmt.target);
                    Value objVal = evalExpr(*memExpr.object);
                    if (objVal.type == ValueType::Instance && objVal.instanceVal) {
                        objVal.instanceVal->fields[memExpr.member] = rhsVal;
                    }
                } else if (assignStmt.target->getType() == ASTNodeType::IndexExpr) {
                    const auto& idxExpr = static_cast<const IndexExprAST&>(*assignStmt.target);
                    Value targetVal = evalExpr(*idxExpr.target);
                    Value indexVal = evalExpr(*idxExpr.index);
                    if (targetVal.type == ValueType::Array && targetVal.arrayVal && indexVal.type == ValueType::Int) {
                        size_t idx = static_cast<size_t>(indexVal.intVal);
                        if (idx < targetVal.arrayVal->size()) {
                            (*targetVal.arrayVal)[idx] = rhsVal;
                        }
                    } else if (targetVal.type == ValueType::Map && targetVal.mapVal && indexVal.type == ValueType::String) {
                        (*targetVal.mapVal)[indexVal.strVal] = rhsVal;
                    }
                }
            }
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
        case ASTNodeType::ReturnStmt: {
            const auto& retStmt = static_cast<const ReturnStmtAST&>(stmt);
            Value val = Value::makeNull();
            if (retStmt.value) val = evalExpr(*retStmt.value);
            throw ReturnSignal{val};
        }
        case ASTNodeType::FunctionDeclStmt: {
            const auto& funcStmt = static_cast<const FunctionDeclStmtAST&>(stmt);
            Value funcVal;
            funcVal.type = ValueType::Function;
            funcVal.funcVal = std::make_shared<FunctionDeclStmtAST>(
                funcStmt.name, funcStmt.params, funcStmt.body, funcStmt.location
            );
            m_env->set(funcStmt.name, funcVal);
            break;
        }
        case ASTNodeType::ClassDeclStmt: {
            const auto& clsStmt = static_cast<const ClassDeclStmtAST&>(stmt);
            auto clsVal = std::make_shared<ClassValue>();
            clsVal->name = clsStmt.name;
            clsVal->fields = clsStmt.fields;
            for (const auto& method : clsStmt.methods) {
                if (method) clsVal->methods[method->name] = method;
            }

            Value val;
            val.type = ValueType::Class;
            val.classVal = clsVal;
            m_env->set(clsStmt.name, val);
            break;
        }
        case ASTNodeType::ForInStmt: {
            const auto& forStmt = static_cast<const ForInStmtAST&>(stmt);
            Value iterVal = Value::makeNull();
            if (forStmt.iterable) iterVal = evalExpr(*forStmt.iterable);

            if (iterVal.type == ValueType::Array && iterVal.arrayVal) {
                for (const auto& elem : *iterVal.arrayVal) {
                    m_env->set(forStmt.varName, elem);
                    if (forStmt.body) executeStmt(*forStmt.body);
                }
            } else if (iterVal.type == ValueType::String) {
                for (char c : iterVal.strVal) {
                    m_env->set(forStmt.varName, Value::makeString(std::string(1, c)));
                    if (forStmt.body) executeStmt(*forStmt.body);
                }
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
        case ASTNodeType::NullExpr: {
            return Value::makeNull();
        }
        case ASTNodeType::ArrayExpr: {
            const auto& arrExpr = static_cast<const ArrayExprAST&>(expr);
            std::vector<Value> elems;
            for (const auto& elem : arrExpr.elements) {
                if (elem) elems.push_back(evalExpr(*elem));
            }
            return Value::makeArray(std::move(elems));
        }
        case ASTNodeType::MapExpr: {
            const auto& mapExpr = static_cast<const MapExprAST&>(expr);
            std::unordered_map<std::string, Value> map;
            for (size_t i = 0; i < mapExpr.keys.size(); ++i) {
                Value v = Value::makeNull();
                if (i < mapExpr.values.size() && mapExpr.values[i]) {
                    v = evalExpr(*mapExpr.values[i]);
                }
                map[mapExpr.keys[i]] = v;
            }
            return Value::makeMap(std::move(map));
        }
        case ASTNodeType::IndexExpr: {
            const auto& idxExpr = static_cast<const IndexExprAST&>(expr);
            Value target = evalExpr(*idxExpr.target);
            Value index = evalExpr(*idxExpr.index);

            if (target.type == ValueType::Array && target.arrayVal && index.type == ValueType::Int) {
                size_t idx = static_cast<size_t>(index.intVal);
                if (idx < target.arrayVal->size()) return (*target.arrayVal)[idx];
            } else if (target.type == ValueType::Map && target.mapVal && index.type == ValueType::String) {
                auto it = target.mapVal->find(index.strVal);
                if (it != target.mapVal->end()) return it->second;
            }
            return Value::makeNull();
        }
        case ASTNodeType::MemberAccessExpr: {
            const auto& memExpr = static_cast<const MemberAccessExprAST&>(expr);
            Value objVal = evalExpr(*memExpr.object);
            if (objVal.type == ValueType::Instance && objVal.instanceVal) {
                auto it = objVal.instanceVal->fields.find(memExpr.member);
                if (it != objVal.instanceVal->fields.end()) return it->second;
            }
            return Value::makeNull();
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
    // 1. Evaluate arguments
    std::vector<Value> argVals;
    for (const auto& arg : callExpr.args) {
        argVals.push_back(evalExpr(*arg));
    }

    // Helper lambda to get double
    auto getNum = [](const Value& v) -> double {
        if (v.type == ValueType::Float) return v.floatVal;
        if (v.type == ValueType::Int) return static_cast<double>(v.intVal);
        return 0.0;
    };

    // 2. Check Math, Matrix, ML & Embedded Built-ins
    std::string fn = callExpr.callee;

    if (fn == "add" && argVals.size() == 2) {
        if (argVals[0].type == ValueType::Float || argVals[1].type == ValueType::Float)
            return Value::makeFloat(getNum(argVals[0]) + getNum(argVals[1]));
        return Value::makeInt(argVals[0].intVal + argVals[1].intVal);
    } else if (fn == "sub" && argVals.size() == 2) {
        return Value::makeFloat(getNum(argVals[0]) - getNum(argVals[1]));
    } else if (fn == "mul" && argVals.size() == 2) {
        return Value::makeFloat(getNum(argVals[0]) * getNum(argVals[1]));
    } else if (fn == "div" && argVals.size() == 2) {
        double denom = getNum(argVals[1]);
        if (denom == 0) return Value::makeNull();
        return Value::makeFloat(getNum(argVals[0]) / denom);
    } else if (fn == "mod" && argVals.size() == 2) {
        if (argVals[1].intVal == 0) return Value::makeNull();
        return Value::makeInt(argVals[0].intVal % argVals[1].intVal);
    } else if (fn == "pow" && argVals.size() == 2) {
        return Value::makeFloat(std::pow(getNum(argVals[0]), getNum(argVals[1])));
    } else if (fn == "sqrt" && argVals.size() == 1) {
        return Value::makeFloat(std::sqrt(getNum(argVals[0])));
    } else if (fn == "abs" && argVals.size() == 1) {
        return Value::makeFloat(std::abs(getNum(argVals[0])));
    } else if (fn == "min" && argVals.size() == 2) {
        return Value::makeFloat(std::min(getNum(argVals[0]), getNum(argVals[1])));
    } else if (fn == "max" && argVals.size() == 2) {
        return Value::makeFloat(std::max(getNum(argVals[0]), getNum(argVals[1])));
    } else if (fn == "sin" && argVals.size() == 1) {
        return Value::makeFloat(std::sin(getNum(argVals[0])));
    } else if (fn == "cos" && argVals.size() == 1) {
        return Value::makeFloat(std::cos(getNum(argVals[0])));
    } else if (fn == "tan" && argVals.size() == 1) {
        return Value::makeFloat(std::tan(getNum(argVals[0])));
    } else if (fn == "asin" && argVals.size() == 1) {
        return Value::makeFloat(std::asin(getNum(argVals[0])));
    } else if (fn == "acos" && argVals.size() == 1) {
        return Value::makeFloat(std::acos(getNum(argVals[0])));
    } else if (fn == "atan" && argVals.size() == 1) {
        return Value::makeFloat(std::atan(getNum(argVals[0])));
    } else if (fn == "atan2" && argVals.size() == 2) {
        return Value::makeFloat(std::atan2(getNum(argVals[0]), getNum(argVals[1])));
    } else if (fn == "exp" && argVals.size() == 1) {
        return Value::makeFloat(std::exp(getNum(argVals[0])));
    } else if (fn == "log" && argVals.size() == 1) {
        return Value::makeFloat(std::log(getNum(argVals[0])));
    } else if (fn == "log10" && argVals.size() == 1) {
        return Value::makeFloat(std::log10(getNum(argVals[0])));
    } else if (fn == "floor" && argVals.size() == 1) {
        return Value::makeFloat(std::floor(getNum(argVals[0])));
    } else if (fn == "ceil" && argVals.size() == 1) {
        return Value::makeFloat(std::ceil(getNum(argVals[0])));
    } else if (fn == "round" && argVals.size() == 1) {
        return Value::makeFloat(std::round(getNum(argVals[0])));
    }

    // --- Matrix & Tensor Primitives ---
    else if (fn == "matrix") {
        if (argVals.size() >= 2) {
            size_t r = static_cast<size_t>(getNum(argVals[0]));
            size_t c = static_cast<size_t>(getNum(argVals[1]));
            double fill = (argVals.size() >= 3) ? getNum(argVals[2]) : 0.0;
            return Value::makeMatrix(r, c, fill);
        }
    } else if (fn == "matmul" && argVals.size() == 2) {
        if (argVals[0].type == ValueType::Matrix && argVals[1].type == ValueType::Matrix) {
            const auto& A = *argVals[0].matrixVal;
            const auto& B = *argVals[1].matrixVal;
            if (A.cols == B.rows) {
                Value C = Value::makeMatrix(A.rows, B.cols, 0.0);
                for (size_t i = 0; i < A.rows; ++i) {
                    for (size_t j = 0; j < B.cols; ++j) {
                        double sum = 0.0;
                        for (size_t k = 0; k < A.cols; ++k) {
                            sum += A.get(i, k) * B.get(k, j);
                        }
                        C.matrixVal->set(i, j, sum);
                    }
                }
                return C;
            }
        }
    } else if (fn == "transpose" && argVals.size() == 1) {
        if (argVals[0].type == ValueType::Matrix) {
            const auto& A = *argVals[0].matrixVal;
            Value T = Value::makeMatrix(A.cols, A.rows, 0.0);
            for (size_t r = 0; r < A.rows; ++r) {
                for (size_t c = 0; c < A.cols; ++c) {
                    T.matrixVal->set(c, r, A.get(r, c));
                }
            }
            return T;
        }
    } else if (fn == "dot" && argVals.size() == 2) {
        if (argVals[0].type == ValueType::Array && argVals[1].type == ValueType::Array) {
            const auto& u = *argVals[0].arrayVal;
            const auto& v = *argVals[1].arrayVal;
            size_t n = std::min(u.size(), v.size());
            double sum = 0.0;
            for (size_t i = 0; i < n; ++i) {
                sum += getNum(u[i]) * getNum(v[i]);
            }
            return Value::makeFloat(sum);
        }
    } else if (fn == "norm" && argVals.size() == 1) {
        if (argVals[0].type == ValueType::Array) {
            const auto& v = *argVals[0].arrayVal;
            double sumSq = 0.0;
            for (const auto& elem : v) {
                double val = getNum(elem);
                sumSq += val * val;
            }
            return Value::makeFloat(std::sqrt(sumSq));
        }
    } else if (fn == "det" && argVals.size() == 1) {
        if (argVals[0].type == ValueType::Matrix) {
            const auto& M = *argVals[0].matrixVal;
            if (M.rows == 2 && M.cols == 2) {
                double d = M.get(0, 0) * M.get(1, 1) - M.get(0, 1) * M.get(1, 0);
                return Value::makeFloat(d);
            } else if (M.rows == 1 && M.cols == 1) {
                return Value::makeFloat(M.get(0, 0));
            }
        }
    } else if (fn == "inv" && argVals.size() == 1) {
        if (argVals[0].type == ValueType::Matrix) {
            const auto& M = *argVals[0].matrixVal;
            if (M.rows == 2 && M.cols == 2) {
                double detVal = M.get(0, 0) * M.get(1, 1) - M.get(0, 1) * M.get(1, 0);
                if (std::abs(detVal) > 1e-12) {
                    Value Inv = Value::makeMatrix(2, 2, 0.0);
                    Inv.matrixVal->set(0, 0, M.get(1, 1) / detVal);
                    Inv.matrixVal->set(0, 1, -M.get(0, 1) / detVal);
                    Inv.matrixVal->set(1, 0, -M.get(1, 0) / detVal);
                    Inv.matrixVal->set(1, 1, M.get(0, 0) / detVal);
                    return Inv;
                }
            }
        }
    }

    // --- Equation Solvers ---
    else if (fn == "solve_quadratic" && argVals.size() == 3) {
        double a = getNum(argVals[0]);
        double b = getNum(argVals[1]);
        double c = getNum(argVals[2]);
        std::vector<Value> roots;

        if (a == 0.0) {
            if (b != 0.0) roots.push_back(Value::makeFloat(-c / b));
        } else {
            double disc = b * b - 4 * a * c;
            if (disc >= 0.0) {
                double r1 = (-b + std::sqrt(disc)) / (2 * a);
                double r2 = (-b - std::sqrt(disc)) / (2 * a);
                roots.push_back(Value::makeFloat(r1));
                if (disc > 0.0) roots.push_back(Value::makeFloat(r2));
            }
        }
        return Value::makeArray(roots);
    } else if (fn == "solve_linear" && argVals.size() == 2) {
        // Solves A * x = b via Gaussian Elimination
        if (argVals[0].type == ValueType::Matrix && argVals[1].type == ValueType::Array) {
            const auto& A_mat = *argVals[0].matrixVal;
            const auto& b_vec = *argVals[1].arrayVal;
            size_t n = A_mat.rows;

            if (A_mat.cols == n && b_vec.size() == n) {
                // Copy matrix A and vector b
                std::vector<std::vector<double>> A(n, std::vector<double>(n));
                std::vector<double> b(n);
                for (size_t r = 0; r < n; ++r) {
                    b[r] = getNum(b_vec[r]);
                    for (size_t c = 0; c < n; ++c) A[r][c] = A_mat.get(r, c);
                }

                // Gaussian Elimination
                for (size_t i = 0; i < n; ++i) {
                    // Pivot
                    size_t maxRow = i;
                    for (size_t k = i + 1; k < n; ++k) {
                        if (std::abs(A[k][i]) > std::abs(A[maxRow][i])) maxRow = k;
                    }
                    std::swap(A[i], A[maxRow]);
                    std::swap(b[i], b[maxRow]);

                    if (std::abs(A[i][i]) < 1e-12) continue; // singular

                    for (size_t k = i + 1; k < n; ++k) {
                        double factor = A[k][i] / A[i][i];
                        for (size_t j = i; j < n; ++j) A[k][j] -= factor * A[i][j];
                        b[k] -= factor * b[i];
                    }
                }

                // Back Substitution
                std::vector<Value> x(n, Value::makeFloat(0.0));
                for (int i = (int)n - 1; i >= 0; --i) {
                    double sum = b[i];
                    for (size_t j = i + 1; j < n; ++j) {
                        sum -= A[i][j] * x[j].floatVal;
                    }
                    if (std::abs(A[i][i]) > 1e-12) x[i] = Value::makeFloat(sum / A[i][i]);
                }
                return Value::makeArray(x);
            }
        }
    }

    // --- Machine Learning Primitives ---
    else if (fn == "relu" && argVals.size() == 1) {
        if (argVals[0].type == ValueType::Array) {
            std::vector<Value> out;
            for (const auto& v : *argVals[0].arrayVal) {
                double val = getNum(v);
                out.push_back(Value::makeFloat(val > 0.0 ? val : 0.0));
            }
            return Value::makeArray(out);
        }
        double x = getNum(argVals[0]);
        return Value::makeFloat(x > 0.0 ? x : 0.0);
    } else if (fn == "sigmoid" && argVals.size() == 1) {
        if (argVals[0].type == ValueType::Array) {
            std::vector<Value> out;
            for (const auto& v : *argVals[0].arrayVal) {
                double val = getNum(v);
                out.push_back(Value::makeFloat(1.0 / (1.0 + std::exp(-val))));
            }
            return Value::makeArray(out);
        }
        double x = getNum(argVals[0]);
        return Value::makeFloat(1.0 / (1.0 + std::exp(-x)));
    } else if (fn == "predict_linear" && argVals.size() == 3) {
        // predict_linear(weights, bias, x) -> w * x + b
        if (argVals[0].type == ValueType::Array && argVals[2].type == ValueType::Array) {
            const auto& w = *argVals[0].arrayVal;
            const auto& x = *argVals[2].arrayVal;
            double b = getNum(argVals[1]);
            double pred = b;
            for (size_t i = 0; i < std::min(w.size(), x.size()); ++i) {
                pred += getNum(w[i]) * getNum(x[i]);
            }
            return Value::makeFloat(pred);
        }
    } else if (fn == "train_linear_step" && argVals.size() == 5) {
        // train_linear_step(weights, bias, x, y, lr)
        if (argVals[0].type == ValueType::Array && argVals[2].type == ValueType::Array) {
            auto w = *argVals[0].arrayVal;
            double bias = getNum(argVals[1]);
            const auto& x = *argVals[2].arrayVal;
            double targetY = getNum(argVals[3]);
            double lr = getNum(argVals[4]);

            // Prediction
            double pred = bias;
            for (size_t i = 0; i < std::min(w.size(), x.size()); ++i) pred += getNum(w[i]) * getNum(x[i]);

            double err = pred - targetY;

            // Gradient update
            for (size_t i = 0; i < std::min(w.size(), x.size()); ++i) {
                double curW = getNum(w[i]);
                w[i] = Value::makeFloat(curW - lr * err * getNum(x[i]));
            }
            bias = bias - lr * err;

            std::unordered_map<std::string, Value> result;
            result["weights"] = Value::makeArray(w);
            result["bias"] = Value::makeFloat(bias);
            result["loss"] = Value::makeFloat(err * err);
            return Value::makeMap(result);
        }
    }

    // --- Embedded Microcontroller Stubs (ESP32, RPi, Arduino) ---
    else if (fn == "pinMode" && argVals.size() == 2) {
        std::cout << "[Embedded HW] pinMode(pin: " << getNum(argVals[0]) << ", mode: " << getNum(argVals[1]) << ")\n";
        return Value::makeNull();
    } else if (fn == "digitalWrite" && argVals.size() == 2) {
        std::cout << "[Embedded HW] digitalWrite(pin: " << getNum(argVals[0]) << ", state: " << getNum(argVals[1]) << ")\n";
        return Value::makeNull();
    } else if (fn == "analogRead" && argVals.size() == 1) {
        std::cout << "[Embedded HW] analogRead(pin: " << getNum(argVals[0]) << ") -> 512\n";
        return Value::makeInt(512);
    } else if (fn == "delay" && argVals.size() == 1) {
        std::cout << "[Embedded HW] delay(" << getNum(argVals[0]) << " ms)\n";
        return Value::makeNull();
    }

    // --- Utility & Container Functions ---
    else if (fn == "len" && argVals.size() == 1) {
        if (argVals[0].type == ValueType::Array && argVals[0].arrayVal) return Value::makeInt(argVals[0].arrayVal->size());
        if (argVals[0].type == ValueType::String) return Value::makeInt(argVals[0].strVal.size());
        if (argVals[0].type == ValueType::Map && argVals[0].mapVal) return Value::makeInt(argVals[0].mapVal->size());
        if (argVals[0].type == ValueType::Matrix && argVals[0].matrixVal) return Value::makeInt(argVals[0].matrixVal->rows);
        return Value::makeInt(0);
    } else if (fn == "push" && argVals.size() == 2) {
        if (argVals[0].type == ValueType::Array && argVals[0].arrayVal) {
            argVals[0].arrayVal->push_back(argVals[1]);
            return Value::makeInt(argVals[0].arrayVal->size());
        }
    } else if (fn == "pop" && argVals.size() == 1) {
        if (argVals[0].type == ValueType::Array && argVals[0].arrayVal && !argVals[0].arrayVal->empty()) {
            Value last = argVals[0].arrayVal->back();
            argVals[0].arrayVal->pop_back();
            return last;
        }
    } else if (fn == "str" && argVals.size() == 1) {
        std::ostringstream ss;
        argVals[0].print(ss);
        return Value::makeString(ss.str());
    } else if (fn == "int" && argVals.size() == 1) {
        return Value::makeInt(static_cast<long long>(getNum(argVals[0])));
    } else if (fn == "float" && argVals.size() == 1) {
        return Value::makeFloat(getNum(argVals[0]));
    }

    // 3. Check variable or function in environment
    bool found = false;
    Value calleeVal = m_env->get(callExpr.callee, found);

    // Class constructor instantiation: e.g. Person()
    if (found && calleeVal.type == ValueType::Class && calleeVal.classVal) {
        auto inst = std::make_shared<InstanceValue>();
        inst->className = calleeVal.classVal->name;
        inst->methods = calleeVal.classVal->methods;
        for (const auto& field : calleeVal.classVal->fields) {
            inst->fields[field] = Value::makeNull();
        }

        Value val;
        val.type = ValueType::Instance;
        val.instanceVal = inst;
        return val;
    }

    // Method call on member access: e.g. p.introduce()
    if (callExpr.calleeExpr && callExpr.calleeExpr->getType() == ASTNodeType::MemberAccessExpr) {
        const auto& memExpr = static_cast<const MemberAccessExprAST&>(*callExpr.calleeExpr);
        Value objVal = evalExpr(*memExpr.object);
        if (objVal.type == ValueType::Instance && objVal.instanceVal) {
            auto it = objVal.instanceVal->methods.find(memExpr.member);
            if (it != objVal.instanceVal->methods.end()) {
                auto methodStmt = it->second;
                auto callEnv = std::make_shared<Environment>(m_env);

                // Bind 'this' instance
                callEnv->set("this", objVal);

                // Bind fields directly to local scope for convenience (e.g. say name)
                for (const auto& fPair : objVal.instanceVal->fields) {
                    callEnv->set(fPair.first, fPair.second);
                }

                // Bind method parameters
                for (size_t i = 0; i < methodStmt->params.size() && i < argVals.size(); ++i) {
                    callEnv->set(methodStmt->params[i], argVals[i]);
                }

                auto previousEnv = m_env;
                m_env = callEnv;
                Value retVal = Value::makeNull();
                try {
                    if (methodStmt->body) executeStmt(*methodStmt->body);
                } catch (const ReturnSignal& sig) {
                    retVal = sig.value;
                }
                m_env = previousEnv;
                return retVal;
            }
        }
    }

    // User-defined function call
    if (found && calleeVal.type == ValueType::Function && calleeVal.funcVal) {
        auto funcStmt = calleeVal.funcVal;
        auto callEnv = std::make_shared<Environment>(m_env);

        // Bind parameters
        for (size_t i = 0; i < funcStmt->params.size() && i < argVals.size(); ++i) {
            callEnv->set(funcStmt->params[i], argVals[i]);
        }

        auto previousEnv = m_env;
        m_env = callEnv;
        Value retVal = Value::makeNull();
        try {
            if (funcStmt->body) executeStmt(*funcStmt->body);
        } catch (const ReturnSignal& sig) {
            retVal = sig.value;
        }
        m_env = previousEnv;
        return retVal;
    }

    Diagnostics::error(callExpr.location, "Runtime Error: Function or class evaluation failed for '" + callExpr.callee + "'");
    return Value::makeNull();
}

} // namespace paslang
