# PasLang Advanced Mathematics & Equation Solver Demo
# Solves quadratic equations, systems of linear equations, vector norms, and trigonometric functions.

say "=================================================="
say "  PasLang Mathematics & Equation Solver Engine  "
say "=================================================="
say ""

# 1. Quadratic Equation Solver: ax^2 + bx + c = 0
say "--- 1. Solving Quadratic Equation: x^2 - 5x + 6 = 0 ---"
let quadRoots = solve_quadratic 1 -5 6
say "Roots of x^2 - 5x + 6 = 0:"
say quadRoots
say ""

# 2. System of Linear Equations Solver: A * x = b
# 2x + y = 5
# x + 3y = 10
say "--- 2. Solving System of 2 Linear Equations (A * x = b) ---"
let A = matrix 2 2 0
let b = [5.0, 10.0]

# Matrix values: A[0][0]=2, A[0][1]=1, A[1][0]=1, A[1][1]=3
say "Matrix A (2x2):"
say A
say "Vector b:"
say b

let x_solution = solve_linear A b
say "Solution vector x:"
say x_solution
say ""

# 3. Scientific Trigonometry & Exponentials
say "--- 3. Scientific & Trigonometric Functions ---"
let angle = 0.785398 # approx pi / 4
let sinVal = sin angle
let cosVal = cos angle
let expVal = exp 1.0

say "sin(pi/4):"
say sinVal
say "cos(pi/4):"
say cosVal
say "exp(1.0) [Euler's e]:"
say expVal
say ""
say "Mathematics & Equation Solver completed successfully!"
