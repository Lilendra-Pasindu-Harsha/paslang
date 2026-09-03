#include "semantic.hpp"
#include "../diagnostics/diagnostics.hpp"

namespace paslang {

SemanticAnalyzer::SemanticAnalyzer() {
    // Register built-in math functions
    m_builtins["add"] = 2;
    m_builtins["sub"] = 2;
    m_builtins["mul"] = 2;
    m_builtins["div"] = 2;
    m_builtins["mod"] = 2;
    m_builtins["pow"] = 2;
    m_builtins["min"] = 2;
    m_builtins["max"] = 2;
    m_builtins["sqrt"] = 1;
    m_builtins["abs"] = 1;
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
        case ASTNodeType::BlockStmt: {
            const auto& blockStmt = static_cast<const BlockStmtAST&>(stmt);
            bool ok = true;
            for (const auto& childStmt : blockStmt.statements) {
                if (childStmt) ok = analyzeStmt(*childStmt) && ok;
            }
            return ok;
        }
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
        case ASTNodeType::IfStmt: {
            const auto& ifStmt = static_cast<const IfStmtAST&>(stmt);
            bool ok = true;
            if (ifStmt.condition) ok = analyzeExpr(*ifStmt.condition) && ok;
            if (ifStmt.thenBranch) ok = analyzeStmt(*ifStmt.thenBranch) && ok;
            if (ifStmt.elseBranch) ok = analyzeStmt(*ifStmt.elseBranch) && ok;
            return ok;
        }
        case ASTNodeType::RepeatStmt: {
            const auto& repStmt = static_cast<const RepeatStmtAST&>(stmt);
            bool ok = true;
            if (repStmt.countExpr) ok = analyzeExpr(*repStmt.countExpr) && ok;
            if (repStmt.body) ok = analyzeStmt(*repStmt.body) && ok;
            return ok;
        }
        case ASTNodeType::WhileStmt: {
            const auto& whileStmt = static_cast<const WhileStmtAST&>(stmt);
            bool ok = true;
            if (whileStmt.condition) ok = analyzeExpr(*whileStmt.condition) && ok;
            if (whileStmt.body) ok = analyzeStmt(*whileStmt.body) && ok;
            return ok;
        }
        default:
            return true;
    }
}

bool SemanticAnalyzer::analyzeExpr(const ExprAST& expr) {
    switch (expr.getType()) {
        case ASTNodeType::NumberExpr:
        case ASTNodeType::StringExpr:
        case ASTNodeType::BoolExpr:
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

        case ASTNodeType::UnaryExpr: {
            const auto& unExpr = static_cast<const UnaryExprAST&>(expr);
            if (unExpr.operand) return analyzeExpr(*unExpr.operand);
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
