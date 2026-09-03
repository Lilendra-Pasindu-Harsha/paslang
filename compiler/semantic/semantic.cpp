#include "semantic.hpp"
#include "../diagnostics/diagnostics.hpp"

namespace paslang {

SemanticAnalyzer::SemanticAnalyzer() {
    // Basic Arithmetic & Power Math
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

    // Advanced Scientific & Trigonometric Math
    m_builtins["sin"] = 1;
    m_builtins["cos"] = 1;
    m_builtins["tan"] = 1;
    m_builtins["asin"] = 1;
    m_builtins["acos"] = 1;
    m_builtins["atan"] = 1;
    m_builtins["atan2"] = 2;
    m_builtins["exp"] = 1;
    m_builtins["log"] = 1;
    m_builtins["log10"] = 1;
    m_builtins["floor"] = 1;
    m_builtins["ceil"] = 1;
    m_builtins["round"] = 1;

    // Matrix & Vector Operations
    m_builtins["matrix"] = 3;
    m_builtins["matmul"] = 2;
    m_builtins["transpose"] = 1;
    m_builtins["dot"] = 2;
    m_builtins["norm"] = 1;
    m_builtins["det"] = 1;
    m_builtins["inv"] = 1;

    // Equation Solvers
    m_builtins["solve_quadratic"] = 3;
    m_builtins["solve_linear"] = 2;

    // Machine Learning & Activation Functions
    m_builtins["relu"] = 1;
    m_builtins["sigmoid"] = 1;
    m_builtins["predict_linear"] = 3;
    m_builtins["train_linear_step"] = 5;

    // Utility & Container Built-ins
    m_builtins["len"] = 1;
    m_builtins["push"] = 2;
    m_builtins["pop"] = 1;
    m_builtins["str"] = 1;
    m_builtins["int"] = 1;
    m_builtins["float"] = 1;

    // Embedded Microcontroller Functions (ESP32, RPi, Arduino)
    m_builtins["pinMode"] = 2;
    m_builtins["digitalWrite"] = 2;
    m_builtins["analogRead"] = 1;
    m_builtins["delay"] = 1;
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
        case ASTNodeType::AssignmentStmt: {
            const auto& assignStmt = static_cast<const AssignmentStmtAST&>(stmt);
            bool ok = true;
            if (assignStmt.target) ok = analyzeExpr(*assignStmt.target) && ok;
            if (assignStmt.value) ok = analyzeExpr(*assignStmt.value) && ok;
            return ok;
        }
        case ASTNodeType::SayStmt: {
            const auto& sayStmt = static_cast<const SayStmtAST&>(stmt);
            if (sayStmt.expression) {
                return analyzeExpr(*sayStmt.expression);
            }
            return true;
        }
        case ASTNodeType::ReturnStmt: {
            const auto& retStmt = static_cast<const ReturnStmtAST&>(stmt);
            if (retStmt.value) return analyzeExpr(*retStmt.value);
            return true;
        }
        case ASTNodeType::FunctionDeclStmt: {
            const auto& funcStmt = static_cast<const FunctionDeclStmtAST&>(stmt);
            m_definedFunctions.insert(funcStmt.name);
            m_definedVars.insert(funcStmt.name);

            // Create temporary scope with params for body checking
            auto savedVars = m_definedVars;
            for (const auto& p : funcStmt.params) {
                m_definedVars.insert(p);
            }

            bool ok = true;
            if (funcStmt.body) ok = analyzeStmt(*funcStmt.body);

            m_definedVars = savedVars;
            m_definedVars.insert(funcStmt.name);
            return ok;
        }
        case ASTNodeType::ClassDeclStmt: {
            const auto& clsStmt = static_cast<const ClassDeclStmtAST&>(stmt);
            m_definedClasses.insert(clsStmt.name);
            m_definedVars.insert(clsStmt.name);
            return true;
        }
        case ASTNodeType::ForInStmt: {
            const auto& forStmt = static_cast<const ForInStmtAST&>(stmt);
            bool ok = true;
            if (forStmt.iterable) ok = analyzeExpr(*forStmt.iterable) && ok;
            
            auto savedVars = m_definedVars;
            m_definedVars.insert(forStmt.varName);

            if (forStmt.body) ok = analyzeStmt(*forStmt.body) && ok;

            m_definedVars = savedVars;
            return ok;
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
        case ASTNodeType::NullExpr:
            return true;

        case ASTNodeType::ArrayExpr: {
            const auto& arrExpr = static_cast<const ArrayExprAST&>(expr);
            bool ok = true;
            for (const auto& elem : arrExpr.elements) {
                if (elem) ok = analyzeExpr(*elem) && ok;
            }
            return ok;
        }

        case ASTNodeType::MapExpr: {
            const auto& mapExpr = static_cast<const MapExprAST&>(expr);
            bool ok = true;
            for (const auto& val : mapExpr.values) {
                if (val) ok = analyzeExpr(*val) && ok;
            }
            return ok;
        }

        case ASTNodeType::IndexExpr: {
            const auto& idxExpr = static_cast<const IndexExprAST&>(expr);
            bool ok = true;
            if (idxExpr.target) ok = analyzeExpr(*idxExpr.target) && ok;
            if (idxExpr.index) ok = analyzeExpr(*idxExpr.index) && ok;
            return ok;
        }

        case ASTNodeType::MemberAccessExpr: {
            const auto& memExpr = static_cast<const MemberAccessExprAST&>(expr);
            if (memExpr.object) return analyzeExpr(*memExpr.object);
            return true;
        }

        case ASTNodeType::VariableExpr: {
            const auto& varExpr = static_cast<const VariableExprAST&>(expr);
            if (m_definedVars.find(varExpr.name) == m_definedVars.end() && varExpr.name != "this") {
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
            if (callExpr.calleeExpr) {
                return analyzeExpr(*callExpr.calleeExpr);
            }

            // Check if builtin, defined function, or defined variable (class/constructor)
            bool isBuiltin = (m_builtins.find(callExpr.callee) != m_builtins.end());
            bool isUserFunc = (m_definedFunctions.find(callExpr.callee) != m_definedFunctions.end() || m_definedVars.find(callExpr.callee) != m_definedVars.end());

            if (!isBuiltin && !isUserFunc) {
                Diagnostics::error(callExpr.location,
                    "Unknown function or class '" + callExpr.callee + "'.",
                    "Check the spelling of the function or class name.");
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
