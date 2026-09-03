<<<<<<< HEAD
# paslang
paslang
=======
# PasLang Compiler v0.1

**PasLang** is an independent, general-purpose programming language designed for ultimate simplicity, modern syntax, and cross-platform high performance.

---

## 🚀 Quick Start (Phase 1: Compiler v0.1)

### PasLang Syntax Example (`examples/hello.pas`)

```paslang
say "Hello World"

let x = 10

let y = add x 5

say y
```

### Expected Output

```text
Hello World
15
```

---

## 🛠️ Building PasLang from Source

### Prerequisites

- **C++ Compiler**: GCC (MinGW-w64), Clang, or MSVC supporting C++17.
- **Build System**: CMake 3.16+ and Ninja/Make.

### Build Steps

#### On Windows (PowerShell / MSYS2 / MinGW / MSVC):

```powershell
# 1. Create build directory
cmake -B build -G "MinGW Makefiles"

# 2. Build the paslang compiler and test suite
cmake --build build
```

#### On Linux / macOS:

```bash
mkdir build && cd build
cmake ..
make -j4
```

---

## 🏃 Running PasLang Programs

Run the `hello.pas` program:

```powershell
./build/paslang examples/hello.pas
```

Inspect the Abstract Syntax Tree (AST):

```powershell
./build/paslang --ast examples/hello.pas
```

---

## 🧪 Running Automated Tests

```powershell
./build/paslang_tests
```

Output:

```text
========================================
   Running PasLang Compiler v0.1 Tests  
========================================

[1/3] Running Lexer Tests...
      --> Lexer Tests PASSED!

[2/3] Running Parser Tests...
      --> Parser Tests PASSED!

[3/3] Running Evaluator Tests...
      --> Evaluator Tests PASSED!

ALL TESTS PASSED SUCCESSFULLY! (3/3)
```

---

## 🏗️ Compiler Architecture

```text
Source Code (.pas)
        ↓
    [Lexer]       -> Tokens with Line/Column Locations
        ↓
   [Parser]       -> Abstract Syntax Tree (AST)
        ↓
[Semantic Check]  -> Symbol Table Validation & Type Check
        ↓
   [Evaluator]    -> PasLang Runtime Execution Engine
```
>>>>>>> f24e6a3 (feat: Initial release of PasLang Compiler v0.1)
