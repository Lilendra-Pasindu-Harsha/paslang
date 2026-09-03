# PasLang Compiler & Multi-Target Language v1.0

**PasLang** is an independent, production-grade, multi-target programming language designed for **advanced mathematics, numeric equation solving, machine learning model training, embedded microcontroller control (ESP32 / Raspberry Pi / Arduino), and cross-platform web/mobile applications**.

---

## 🌟 Key Capabilities

### 1. 🧮 Mathematics & Equation Solver Engine
- **Equation Solvers**: Solves Quadratic equations (`solve_quadratic a b c`) and $N \times N$ Systems of Linear Equations (`solve_linear A b`) using Gaussian elimination.
- **Scientific Functions**: Trigonometric (`sin`, `cos`, `tan`, `asin`, `acos`, `atan`, `atan2`), Exponentials & Logarithms (`exp`, `log`, `log10`), and Rounding (`floor`, `ceil`, `round`).
- **Matrix & Vector Algebra**: Native 2D matrix allocation (`matrix`), matrix multiplication (`matmul`), transposition (`transpose`), vector dot product (`dot`), and vector Euclidean norm (`norm`).

### 2. 🧠 Machine Learning & Neural Network Primitives
- **Activations**: Built-in `relu` and `sigmoid` functions operating on scalars, vectors, and matrices.
- **Training & Gradient Descent**: `train_linear_step(weights, bias, x, y, lr)` updates weights and bias over training data with real-time loss tracking.
- **Inference**: `predict_linear(weights, bias, x)` for fast model evaluation.

### 3. ⚡ Embedded Microcontroller & Hardware Target (ESP32, Raspberry Pi, Arduino)
- **C Code Transpiler (`--target=c` or `--emit-c`)**: Translates any PasLang script into clean, portable, standalone ISO C code.
- **Hardware I/O Stubs**: Native hardware primitives (`pinMode`, `digitalWrite`, `analogRead`, `delay`) ready to compile directly with `xtensa-esp32-elf-gcc` (ESP32), `arm-linux-gnueabihf-gcc` (Raspberry Pi), or `emcc` (WebAssembly for Web apps).

---

## 🚀 Quick Examples

### 1. Mathematics & Equation Solver (`examples/equation_solver.pas`)

```paslang
let quadRoots = solve_quadratic 1 -5 6
say "Roots of x^2 - 5x + 6 = 0:"
say quadRoots

let A = matrix 2 2 0
let b = [5.0, 10.0]
let x_solution = solve_linear A b
say "Linear System Solution:"
say x_solution
```

### 2. Machine Learning Training & Inference (`examples/machine_learning.pas`)

```paslang
let weights = [0.1]
let bias = 0.0
let lr = 0.05
let train_x = [[1.0], [2.0], [3.0], [4.0]]

repeat 20:
    for sample_x in train_x:
        let step = train_linear_step weights bias sample_x 3.5 lr
        weights = step["weights"]
        bias = step["bias"]

let prediction = predict_linear weights bias [5.0]
say "Model Prediction for x=5.0:"
say prediction
```

### 3. ESP32 / Raspberry Pi Microcontroller Control (`examples/esp32_embedded.pas`)

```paslang
let LED_PIN = 13
let SENSOR_PIN = 34

pinMode LED_PIN 1
pinMode SENSOR_PIN 0

let raw_val = analogRead SENSOR_PIN
let voltage = mul raw_val 0.0048828

if voltage > 2.0:
    digitalWrite LED_PIN 1
else:
    digitalWrite LED_PIN 0
```

---

## 🛠️ Building PasLang from Source

### Prerequisites
- **C++ Compiler**: Any GCC, Clang, or MSVC supporting **C++17**.
- **Build Tool**: CMake 3.16+.

### Build Steps

#### On Windows (PowerShell / MinGW / MSVC):
```powershell
cmake -B build -G "MinGW Makefiles"
cmake --build build
```

#### On Linux / macOS:
```bash
mkdir build && cd build
cmake ..
make -j4
```

---

## 🏃 Compiler CLI Usage

```powershell
# 1. Direct High-Performance Interpreter Mode:
./build/paslang examples/equation_solver.pas
./build/paslang examples/machine_learning.pas
./build/paslang examples/esp32_embedded.pas

# 2. Transpile to Standalone C Code for ESP32 / Raspberry Pi / Web / Mobile:
./build/paslang --emit-c -o output.c examples/esp32_embedded.pas

# 3. Dump Abstract Syntax Tree (AST):
./build/paslang --ast examples/equation_solver.pas
```

---

## 🧪 Running Automated Tests

```powershell
./build/paslang_tests
```

Expected Output:

```text
========================================
   Running PasLang Compiler v1.0 Tests  
========================================

[1/4] Running Lexer Tests...
      --> Lexer Tests PASSED!

[2/4] Running Parser Tests...
      --> Parser Tests PASSED!

[3/4] Running Evaluator (Math, Equation Solver & ML) Tests...
      --> Evaluator Tests PASSED!

[4/4] Running C Code Generator (Embedded / ESP32 Target) Tests...
      --> CodeGen Tests PASSED!

ALL TESTS PASSED SUCCESSFULLY! (4/4)
```

---

## 🏗️ Compiler Architecture

```text
Source Code (.pas)
        ↓
    [Lexer]       -> Tokens with Line/Column Tracking
        ↓
   [Parser]       -> Abstract Syntax Tree (AST)
        ↓
[Semantic Check]  -> Symbol Table Validation & Type Check
        ↓
  ┌─────┴────────────────────────┐
  ↓                              ↓
[Evaluator]              [C Code Generator]
(High-Performance        (Transpiles to C for ESP32,
Interpreter & Engine)     Raspberry Pi, Wasm, Mobile)
```
