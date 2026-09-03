#ifndef PASLANG_CODEGEN_C_HPP
#define PASLANG_CODEGEN_C_HPP

#include <string>
#include <sstream>
#include <memory>
#include "../ast/ast.hpp"

namespace paslang {

class CodeGenC {
public:
    CodeGenC() = default;

    // Generates complete, standalone ISO C99 code targeting microcontrollers (ESP32/RPi/Arduino), Web (Wasm), or Desktop.
    std::string generate(const ProgramAST& program);

private:
    void emitHeader(std::ostringstream& out);
    void emitStmt(const StmtAST& stmt, std::ostringstream& out, int indent = 0);
    std::string emitExpr(const ExprAST& expr);

    int m_tempVarCounter = 0;
    std::string newTempVar() {
        return "_pas_tmp_" + std::to_string(++m_tempVarCounter);
    }
};

} // namespace paslang

#endif // PASLANG_CODEGEN_C_HPP
