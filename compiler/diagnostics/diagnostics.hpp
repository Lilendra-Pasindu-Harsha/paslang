#ifndef PASLANG_DIAGNOSTICS_HPP
#define PASLANG_DIAGNOSTICS_HPP

#include <string>
#include <iostream>

namespace paslang {

struct SourceLocation {
    std::string filename;
    size_t line = 1;
    size_t column = 1;
};

enum class DiagnosticLevel {
    Error,
    Warning,
    Info
};

class Diagnostics {
public:
    static void report(DiagnosticLevel level, const SourceLocation& loc, const std::string& message, const std::string& hint = "") {
        std::string levelStr;
        switch (level) {
            case DiagnosticLevel::Error: levelStr = "Error"; break;
            case DiagnosticLevel::Warning: levelStr = "Warning"; break;
            case DiagnosticLevel::Info: levelStr = "Info"; break;
        }

        std::cerr << levelStr << ": " << message << "\n";
        if (!loc.filename.empty() || loc.line > 0) {
            std::cerr << "  --> " << (loc.filename.empty() ? "main.pas" : loc.filename) 
                      << ":" << loc.line << ":" << loc.column << "\n";
        }
        if (!hint.empty()) {
            std::cerr << "Hint: " << hint << "\n";
        }
        std::cerr << std::endl;
    }

    static void error(const SourceLocation& loc, const std::string& message, const std::string& hint = "") {
        report(DiagnosticLevel::Error, loc, message, hint);
    }
};

} // namespace paslang

#endif // PASLANG_DIAGNOSTICS_HPP
