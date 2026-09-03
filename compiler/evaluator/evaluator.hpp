#ifndef PASLANG_EVALUATOR_HPP
#define PASLANG_EVALUATOR_HPP

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <iostream>
#include "../ast/ast.hpp"

namespace paslang {

enum class ValueType {
    Int,
    Float,
    String,
    Bool,
    Null,
    Array,
    Map,
    Matrix,
    Function,
    Class,
    Instance
};

struct Value;

struct MatrixValue {
    size_t rows = 0;
    size_t cols = 0;
    std::vector<double> data;

    double get(size_t r, size_t c) const {
        if (r >= rows || c >= cols) return 0.0;
        return data[r * cols + c];
    }
    void set(size_t r, size_t c, double val) {
        if (r < rows && c < cols) {
            data[r * cols + c] = val;
        }
    }
};

struct ClassValue {
    std::string name;
    std::vector<std::string> fields;
    std::unordered_map<std::string, std::shared_ptr<FunctionDeclStmtAST>> methods;
};

struct InstanceValue {
    std::string className;
    std::unordered_map<std::string, Value> fields;
    std::unordered_map<std::string, std::shared_ptr<FunctionDeclStmtAST>> methods;
};

struct Value {
    ValueType type = ValueType::Null;
    long long intVal = 0;
    double floatVal = 0.0;
    std::string strVal;
    bool boolVal = false;

    std::shared_ptr<std::vector<Value>> arrayVal;
    std::shared_ptr<std::unordered_map<std::string, Value>> mapVal;
    std::shared_ptr<MatrixValue> matrixVal;
    std::shared_ptr<FunctionDeclStmtAST> funcVal;
    std::shared_ptr<ClassValue> classVal;
    std::shared_ptr<InstanceValue> instanceVal;

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
    static Value makeArray(std::vector<Value> elems) {
        Value val; val.type = ValueType::Array;
        val.arrayVal = std::make_shared<std::vector<Value>>(std::move(elems));
        return val;
    }
    static Value makeMap(std::unordered_map<std::string, Value> map) {
        Value val; val.type = ValueType::Map;
        val.mapVal = std::make_shared<std::unordered_map<std::string, Value>>(std::move(map));
        return val;
    }
    static Value makeMatrix(size_t r, size_t c, double fill = 0.0) {
        Value val; val.type = ValueType::Matrix;
        auto mat = std::make_shared<MatrixValue>();
        mat->rows = r;
        mat->cols = c;
        mat->data.assign(r * c, fill);
        val.matrixVal = mat;
        return val;
    }

    bool isTruthy() const {
        switch (type) {
            case ValueType::Bool: return boolVal;
            case ValueType::Int: return intVal != 0;
            case ValueType::Float: return floatVal != 0.0;
            case ValueType::String: return !strVal.empty();
            case ValueType::Array: return arrayVal && !arrayVal->empty();
            case ValueType::Map: return mapVal && !mapVal->empty();
            case ValueType::Matrix: return matrixVal && matrixVal->rows > 0;
            case ValueType::Null: return false;
            default: return true;
        }
    }

    void print(std::ostream& os = std::cout) const {
        switch (type) {
            case ValueType::Int: os << intVal; break;
            case ValueType::Float: os << floatVal; break;
            case ValueType::String: os << strVal; break;
            case ValueType::Bool: os << (boolVal ? "true" : "false"); break;
            case ValueType::Null: os << "null"; break;
            case ValueType::Array: {
                os << "[";
                if (arrayVal) {
                    for (size_t i = 0; i < arrayVal->size(); ++i) {
                        (*arrayVal)[i].print(os);
                        if (i + 1 < arrayVal->size()) os << ", ";
                    }
                }
                os << "]";
                break;
            }
            case ValueType::Map: {
                os << "{";
                if (mapVal) {
                    size_t i = 0;
                    for (const auto& pair : *mapVal) {
                        os << pair.first << ": ";
                        pair.second.print(os);
                        if (++i < mapVal->size()) os << ", ";
                    }
                }
                os << "}";
                break;
            }
            case ValueType::Matrix: {
                os << "Matrix(" << (matrixVal ? matrixVal->rows : 0) << "x" << (matrixVal ? matrixVal->cols : 0) << ") [";
                if (matrixVal) {
                    for (size_t r = 0; r < matrixVal->rows; ++r) {
                        os << "[";
                        for (size_t c = 0; c < matrixVal->cols; ++c) {
                            os << matrixVal->get(r, c);
                            if (c + 1 < matrixVal->cols) os << ", ";
                        }
                        os << "]";
                        if (r + 1 < matrixVal->rows) os << ", ";
                    }
                }
                os << "]";
                break;
            }
            case ValueType::Class: os << "<class " << (classVal ? classVal->name : "") << ">"; break;
            case ValueType::Instance: os << "<instance " << (instanceVal ? instanceVal->className : "") << ">"; break;
            case ValueType::Function: os << "<function " << (funcVal ? funcVal->name : "") << ">"; break;
        }
    }
};

struct ReturnSignal {
    Value value;
};

class Environment {
public:
    explicit Environment(std::shared_ptr<Environment> parent = nullptr) : m_parent(std::move(parent)) {}

    void set(const std::string& name, Value val) {
        m_bindings[name] = std::move(val);
    }

    bool update(const std::string& name, Value val) {
        auto it = m_bindings.find(name);
        if (it != m_bindings.end()) {
            it->second = std::move(val);
            return true;
        }
        if (m_parent) return m_parent->update(name, std::move(val));
        return false;
    }

    Value get(const std::string& name, bool& found) const {
        auto it = m_bindings.find(name);
        if (it != m_bindings.end()) {
            found = true;
            return it->second;
        }
        if (m_parent) return m_parent->get(name, found);
        found = false;
        return Value::makeNull();
    }

private:
    std::unordered_map<std::string, Value> m_bindings;
    std::shared_ptr<Environment> m_parent;
};

class Evaluator {
public:
    Evaluator();
    bool execute(const ProgramAST& program);
    Value evalExpr(const ExprAST& expr);

    void setVariable(const std::string& name, Value val);
    Value getVariable(const std::string& name, SourceLocation loc);

private:
    void executeStmt(const StmtAST& stmt);
    Value evalCallExpr(const CallExprAST& callExpr);
    Value evalUnaryExpr(const UnaryExprAST& unExpr);
    Value evalBinaryExpr(const BinaryExprAST& binExpr);

    std::shared_ptr<Environment> m_env;
};

} // namespace paslang

#endif // PASLANG_EVALUATOR_HPP
