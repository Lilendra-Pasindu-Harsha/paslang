#include "codegen_c.hpp"
#include <iostream>

namespace paslang {

void CodeGenC::emitHeader(std::ostringstream& out) {
    out << "/* Generated automatically by PasLang Compiler */\n";
    out << "#include <stdio.h>\n";
    out << "#include <stdlib.h>\n";
    out << "#include <string.h>\n";
    out << "#include <math.h>\n";
    out << "#include <stdbool.h>\n\n";

    out << "/* PasLang Runtime Support Header */\n";
    out << "typedef enum { PAS_INT, PAS_FLOAT, PAS_STRING, PAS_BOOL, PAS_NULL, PAS_MATRIX } PasType;\n\n";

    out << "typedef struct {\n";
    out << "    size_t rows;\n";
    out << "    size_t cols;\n";
    out << "    double* data;\n";
    out << "} PasMatrix;\n\n";

    out << "typedef struct {\n";
    out << "    PasType type;\n";
    out << "    long long intVal;\n";
    out << "    double floatVal;\n";
    out << "    char* strVal;\n";
    out << "    bool boolVal;\n";
    out << "    PasMatrix* matrixVal;\n";
    out << "} PasValue;\n\n";

    // Helper functions
    out << "PasValue pas_make_int(long long v) { PasValue val = {PAS_INT, v, 0.0, NULL, false, NULL}; return val; }\n";
    out << "PasValue pas_make_float(double v) { PasValue val = {PAS_FLOAT, 0, v, NULL, false, NULL}; return val; }\n";
    out << "PasValue pas_make_string(const char* s) { PasValue val = {PAS_STRING, 0, 0.0, strdup(s), false, NULL}; return val; }\n";
    out << "PasValue pas_make_bool(bool v) { PasValue val = {PAS_BOOL, 0, 0.0, NULL, v, NULL}; return val; }\n";
    out << "PasValue pas_make_null() { PasValue val = {PAS_NULL, 0, 0.0, NULL, false, NULL}; return val; }\n\n";

    out << "PasMatrix* pas_matrix_create(size_t r, size_t c, double fill) {\n";
    out << "    PasMatrix* m = (PasMatrix*)malloc(sizeof(PasMatrix));\n";
    out << "    m->rows = r; m->cols = c;\n";
    out << "    m->data = (double*)malloc(r * c * sizeof(double));\n";
    out << "    for (size_t i = 0; i < r * c; ++i) m->data[i] = fill;\n";
    out << "    return m;\n";
    out << "}\n\n";

    out << "PasValue pas_make_matrix(size_t r, size_t c, double fill) {\n";
    out << "    PasValue val = {PAS_MATRIX, 0, 0.0, NULL, false, pas_matrix_create(r, c, fill)};\n";
    out << "    return val;\n";
    out << "}\n\n";

    out << "void pas_print(PasValue v) {\n";
    out << "    if (v.type == PAS_INT) printf(\"%lld\", v.intVal);\n";
    out << "    else if (v.type == PAS_FLOAT) printf(\"%g\", v.floatVal);\n";
    out << "    else if (v.type == PAS_STRING) printf(\"%s\", v.strVal ? v.strVal : \"\");\n";
    out << "    else if (v.type == PAS_BOOL) printf(\"%s\", v.boolVal ? \"true\" : \"false\");\n";
    out << "    else if (v.type == PAS_MATRIX && v.matrixVal) {\n";
    out << "        printf(\"Matrix(%zu, %zu) [\", v.matrixVal->rows, v.matrixVal->cols);\n";
    out << "        for (size_t r = 0; r < v.matrixVal->rows; ++r) {\n";
    out << "            printf(\"[\");\n";
    out << "            for (size_t c = 0; c < v.matrixVal->cols; ++c) {\n";
    out << "                printf(\"%g%s\", v.matrixVal->data[r * v.matrixVal->cols + c], c + 1 < v.matrixVal->cols ? \", \" : \"\");\n";
    out << "            }\n";
    out << "            printf(\"]%s\", r + 1 < v.matrixVal->rows ? \", \" : \"\");\n";
    out << "        }\n";
    out << "        printf(\"]\");\n";
    out << "    } else printf(\"null\");\n";
    out << "}\n\n";

    // Microcontroller stubs (ESP32 / Raspberry Pi / Arduino)
    out << "/* Microcontroller / Embedded Hardware Stubs (ESP32, RPi, Arduino) */\n";
    out << "#ifndef OUTPUT\n#define OUTPUT 1\n#endif\n";
    out << "#ifndef INPUT\n#define INPUT 0\n#endif\n";
    out << "#ifndef HIGH\n#define HIGH 1\n#endif\n";
    out << "#ifndef LOW\n#define LOW 0\n#endif\n";
    out << "void pinMode(int pin, int mode) { printf(\"[Embedded HW] pinMode(%d, %d)\\n\", pin, mode); }\n";
    out << "void digitalWrite(int pin, int state) { printf(\"[Embedded HW] digitalWrite(%d, %d)\\n\", pin, state); }\n";
    out << "int analogRead(int pin) { printf(\"[Embedded HW] analogRead(%d)\\n\", pin); return 512; }\n";
    out << "void delay(int ms) { printf(\"[Embedded HW] delay(%d ms)\\n\", ms); }\n\n";

    // Math & ML functions in C
    out << "double pas_to_double(PasValue v) {\n";
    out << "    if (v.type == PAS_FLOAT) return v.floatVal;\n";
    out << "    if (v.type == PAS_INT) return (double)v.intVal;\n";
    out << "    return 0.0;\n";
    out << "}\n\n";

    out << "double pas_relu(double x) { return x > 0.0 ? x : 0.0; }\n";
    out << "double pas_sigmoid(double x) { return 1.0 / (1.0 + exp(-x)); }\n\n";
}

std::string CodeGenC::emitExpr(const ExprAST& expr) {
    switch (expr.getType()) {
        case ASTNodeType::NumberExpr: {
            const auto& num = static_cast<const NumberExprAST&>(expr);
            if (num.isFloat) return "pas_make_float(" + std::to_string(num.value) + ")";
            return "pas_make_int(" + std::to_string((long long)num.value) + ")";
        }
        case ASTNodeType::StringExpr: {
            const auto& str = static_cast<const StringExprAST&>(expr);
            return "pas_make_string(\"" + str.value + "\")";
        }
        case ASTNodeType::BoolExpr: {
            const auto& b = static_cast<const BoolExprAST&>(expr);
            return b.value ? "pas_make_bool(true)" : "pas_make_bool(false)";
        }
        case ASTNodeType::NullExpr: {
            return "pas_make_null()";
        }
        case ASTNodeType::VariableExpr: {
            const auto& var = static_cast<const VariableExprAST&>(expr);
            return var.name;
        }
        case ASTNodeType::BinaryExpr: {
            const auto& bin = static_cast<const BinaryExprAST&>(expr);
            std::string lhs = emitExpr(*bin.lhs);
            std::string rhs = emitExpr(*bin.rhs);

            if (bin.op == "+") {
                return "pas_make_float(pas_to_double(" + lhs + ") + pas_to_double(" + rhs + "))";
            } else if (bin.op == "-") {
                return "pas_make_float(pas_to_double(" + lhs + ") - pas_to_double(" + rhs + "))";
            } else if (bin.op == "*") {
                return "pas_make_float(pas_to_double(" + lhs + ") * pas_to_double(" + rhs + "))";
            } else if (bin.op == "/") {
                return "pas_make_float(pas_to_double(" + lhs + ") / pas_to_double(" + rhs + "))";
            } else if (bin.op == "==") {
                return "pas_make_bool(pas_to_double(" + lhs + ") == pas_to_double(" + rhs + "))";
            } else if (bin.op == "!=") {
                return "pas_make_bool(pas_to_double(" + lhs + ") != pas_to_double(" + rhs + "))";
            } else if (bin.op == "<") {
                return "pas_make_bool(pas_to_double(" + lhs + ") < pas_to_double(" + rhs + "))";
            } else if (bin.op == ">") {
                return "pas_make_bool(pas_to_double(" + lhs + ") > pas_to_double(" + rhs + "))";
            } else if (bin.op == "<=") {
                return "pas_make_bool(pas_to_double(" + lhs + ") <= pas_to_double(" + rhs + "))";
            } else if (bin.op == ">=") {
                return "pas_make_bool(pas_to_double(" + lhs + ") >= pas_to_double(" + rhs + "))";
            }
            return "pas_make_null()";
        }
        case ASTNodeType::UnaryExpr: {
            const auto& un = static_cast<const UnaryExprAST&>(expr);
            std::string sub = emitExpr(*un.operand);
            if (un.op == "-") return "pas_make_float(-pas_to_double(" + sub + "))";
            if (un.op == "not") return "pas_make_bool(!(" + sub + ".boolVal))";
            return "pas_make_null()";
        }
        case ASTNodeType::CallExpr: {
            const auto& call = static_cast<const CallExprAST&>(expr);
            if (call.callee == "add" && call.args.size() == 2) {
                return "pas_make_float(pas_to_double(" + emitExpr(*call.args[0]) + ") + pas_to_double(" + emitExpr(*call.args[1]) + "))";
            } else if (call.callee == "sub" && call.args.size() == 2) {
                return "pas_make_float(pas_to_double(" + emitExpr(*call.args[0]) + ") - pas_to_double(" + emitExpr(*call.args[1]) + "))";
            } else if (call.callee == "mul" && call.args.size() == 2) {
                return "pas_make_float(pas_to_double(" + emitExpr(*call.args[0]) + ") * pas_to_double(" + emitExpr(*call.args[1]) + "))";
            } else if (call.callee == "div" && call.args.size() == 2) {
                return "pas_make_float(pas_to_double(" + emitExpr(*call.args[0]) + ") / pas_to_double(" + emitExpr(*call.args[1]) + "))";
            } else if (call.callee == "sqrt" && call.args.size() == 1) {
                return "pas_make_float(sqrt(pas_to_double(" + emitExpr(*call.args[0]) + ")))";
            } else if (call.callee == "sin" && call.args.size() == 1) {
                return "pas_make_float(sin(pas_to_double(" + emitExpr(*call.args[0]) + ")))";
            } else if (call.callee == "cos" && call.args.size() == 1) {
                return "pas_make_float(cos(pas_to_double(" + emitExpr(*call.args[0]) + ")))";
            } else if (call.callee == "tan" && call.args.size() == 1) {
                return "pas_make_float(tan(pas_to_double(" + emitExpr(*call.args[0]) + ")))";
            } else if (call.callee == "relu" && call.args.size() == 1) {
                return "pas_make_float(pas_relu(pas_to_double(" + emitExpr(*call.args[0]) + ")))";
            } else if (call.callee == "sigmoid" && call.args.size() == 1) {
                return "pas_make_float(pas_sigmoid(pas_to_double(" + emitExpr(*call.args[0]) + ")))";
            } else if (call.callee == "matrix" && call.args.size() == 3) {
                return "pas_make_matrix((size_t)pas_to_double(" + emitExpr(*call.args[0]) + "), (size_t)pas_to_double(" + emitExpr(*call.args[1]) + "), pas_to_double(" + emitExpr(*call.args[2]) + "))";
            }

            // User function call
            std::string code = call.callee + "(";
            for (size_t i = 0; i < call.args.size(); ++i) {
                code += emitExpr(*call.args[i]);
                if (i + 1 < call.args.size()) code += ", ";
            }
            code += ")";
            return code;
        }
        default:
            return "pas_make_null()";
    }
}

void CodeGenC::emitStmt(const StmtAST& stmt, std::ostringstream& out, int indent) {
    std::string ind(indent, ' ');

    switch (stmt.getType()) {
        case ASTNodeType::BlockStmt: {
            const auto& block = static_cast<const BlockStmtAST&>(stmt);
            for (const auto& s : block.statements) {
                if (s) emitStmt(*s, out, indent);
            }
            break;
        }
        case ASTNodeType::VarDeclStmt: {
            const auto& var = static_cast<const VarDeclStmtAST&>(stmt);
            std::string valStr = var.initializer ? emitExpr(*var.initializer) : "pas_make_null()";
            out << ind << "PasValue " << var.varName << " = " << valStr << ";\n";
            break;
        }
        case ASTNodeType::AssignmentStmt: {
            const auto& assign = static_cast<const AssignmentStmtAST&>(stmt);
            if (assign.target && assign.target->getType() == ASTNodeType::VariableExpr) {
                const auto& var = static_cast<const VariableExprAST&>(*assign.target);
                std::string valStr = assign.value ? emitExpr(*assign.value) : "pas_make_null()";
                out << ind << var.name << " = " << valStr << ";\n";
            }
            break;
        }
        case ASTNodeType::SayStmt: {
            const auto& say = static_cast<const SayStmtAST&>(stmt);
            if (say.expression) {
                std::string valStr = emitExpr(*say.expression);
                out << ind << "pas_print(" << valStr << "); printf(\"\\n\");\n";
            }
            break;
        }
        case ASTNodeType::ReturnStmt: {
            const auto& ret = static_cast<const ReturnStmtAST&>(stmt);
            std::string valStr = ret.value ? emitExpr(*ret.value) : "pas_make_null()";
            out << ind << "return " << valStr << ";\n";
            break;
        }
        case ASTNodeType::IfStmt: {
            const auto& ifStmt = static_cast<const IfStmtAST&>(stmt);
            std::string condStr = emitExpr(*ifStmt.condition);
            out << ind << "if ((" << condStr << ").boolVal || pas_to_double(" << condStr << ") != 0.0) {\n";
            if (ifStmt.thenBranch) emitStmt(*ifStmt.thenBranch, out, indent + 4);
            if (ifStmt.elseBranch) {
                out << ind << "} else {\n";
                emitStmt(*ifStmt.elseBranch, out, indent + 4);
            }
            out << ind << "}\n";
            break;
        }
        case ASTNodeType::WhileStmt: {
            const auto& whileStmt = static_cast<const WhileStmtAST&>(stmt);
            std::string condStr = emitExpr(*whileStmt.condition);
            out << ind << "while ((" << condStr << ").boolVal || pas_to_double(" << condStr << ") != 0.0) {\n";
            if (whileStmt.body) emitStmt(*whileStmt.body, out, indent + 4);
            out << ind << "}\n";
            break;
        }
        case ASTNodeType::FunctionDeclStmt: {
            const auto& func = static_cast<const FunctionDeclStmtAST&>(stmt);
            out << ind << "PasValue " << func.name << "(";
            for (size_t i = 0; i < func.params.size(); ++i) {
                out << "PasValue " << func.params[i];
                if (i + 1 < func.params.size()) out << ", ";
            }
            out << ") {\n";
            if (func.body) emitStmt(*func.body, out, indent + 4);
            out << ind << "    return pas_make_null();\n";
            out << ind << "}\n\n";
            break;
        }
        default:
            break;
    }
}

std::string CodeGenC::generate(const ProgramAST& program) {
    std::ostringstream out;
    emitHeader(out);

    // 1. Emit top-level functions first
    for (const auto& stmt : program.statements) {
        if (stmt && stmt->getType() == ASTNodeType::FunctionDeclStmt) {
            emitStmt(*stmt, out, 0);
        }
    }

    // 2. Emit main entry point
    out << "int main(int argc, char** argv) {\n";
    out << "    printf(\"=== Running PasLang Compiled Executable (Target: Embedded/C) ===\\n\\n\");\n";

    for (const auto& stmt : program.statements) {
        if (stmt && stmt->getType() != ASTNodeType::FunctionDeclStmt) {
            emitStmt(*stmt, out, 4);
        }
    }

    out << "\n    return 0;\n";
    out << "}\n";

    return out.str();
}

} // namespace paslang
