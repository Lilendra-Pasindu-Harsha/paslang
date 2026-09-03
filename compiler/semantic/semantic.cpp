#include "semantic.hpp"
#include "../diagnostics/diagnostics.hpp"

namespace paslang {

SemanticAnalyzer::SemanticAnalyzer() {
    // Register built-in functions
    m_builtins["add"] = 2;
    m_builtins["sub"] = 2;
    m_builtins["mul"] = 2;
    m_builtins["div"] = 2;
}

bool SemanticAnalyzer::analyze(const ProgramAST& program) {
    bool hasErrors = false;
    for (const auto& stmt : program.statements) {
        if (stmt) {
            if (!analyzeStmt(*stmt)) {
                hasErrors = true;
            }
        }
    }
    return !hasErrors;
}

bool SemanticAnalyzer::analyzeStmt(const StmtAST& stmt) {
    switch (stmt.getType()) {
        case ASTNodeType::VarDeclStmt: {
            const auto& varDecl = static_cast<const VarDeclStmtAST&>(stmt);
            if (varDecl.initializer) {
                if (!analyzeExpr(*varDecl.initializer)) return false;
            }
            m_definedVars.insert(varDecl.varName);
            return true;
        }
        case ASTNodeType::SayStmt: {
            const auto& sayStmt = static_cast<const SayStmtAST&>(stmt);
            if (sayStmt.expression) {
                return analyzeExpr(*sayStmt.expression);
            }
            return true;
        }
        default:
            return true;
    }
}

bool SemanticAnalyzer::analyzeExpr(const ExprAST& expr) {
    switch (expr.getType()) {
        case ASTNodeType::NumberExpr:
        case ASTNodeType::StringExpr:
            return true;

        case ASTNodeType::VariableExpr: {
            const auto& varExpr = static_cast<const VariableExprAST&>(expr);
            if (m_definedVars.find(varExpr.name) == m_definedVars.end()) {
                Diagnostics::error(varExpr.location,
                    "Variable '" + varExpr.name + "' was not defined.",
                    "Define the variable using 'let " + varExpr.name + " = ...' before using it.");
                return false;
            }
            return true;
        }

        case ASTNodeType::BinaryExpr: {
            const auto& binExpr = static_cast<const BinaryExprAST&>(expr);
            bool ok = true;
            if (binExpr.lhs) ok = analyzeExpr(*binExpr.lhs) && ok;
            if (binExpr.rhs) ok = analyzeExpr(*binExpr.rhs) && ok;
            return ok;
        }

        case ASTNodeType::CallExpr: {
            const auto& callExpr = static_cast<const CallExprAST&>(expr);
            auto it = m_builtins.find(callExpr.callee);
            if (it == m_builtins.end()) {
                Diagnostics::error(callExpr.location,
                    "Unknown function '" + callExpr.callee + "'.",
                    "Check the spelling of the function name.");
                return false;
            }

            int expectedArgs = it->second;
            if (expectedArgs != -1 && static_cast<int>(callExpr.args.size()) != expectedArgs) {
                Diagnostics::error(callExpr.location,
                    "Function '" + callExpr.callee + "' expects " + std::to_string(expectedArgs) +
                    " arguments, but received " + std::to_string(callExpr.args.size()) + ".");
                return false;
            }

            bool ok = true;
            for (const auto& arg : callExpr.args) {
                if (arg) ok = analyzeExpr(*arg) && ok;
            }
            return ok;
        }

        default:
            return true;
    }
}

} // namespace paslang
