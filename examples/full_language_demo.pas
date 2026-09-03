say "=================================================="
say "   PasLang Full Language Engine Demonstration    "
say "=================================================="

say "--- 1. User-Defined Functions & Recursion ---"

function multiply(a, b):
    return a * b

function factorial(n):
    if n <= 1:
        return 1
    return n * factorial(n - 1)

let multRes = multiply(7, 8)
say "multiply(7, 8):"
say multRes

let factRes = factorial(5)
say "factorial(5):"
say factRes

say "--- 2. Data Structures (Arrays & Maps) ---"

let numbers = [10, 20, 30, 40]
say "Array element at index 2:"
say numbers[2]

let user = {
    name: "PasLang User",
    role: "Core Developer"
}
say "Map entry for 'name':"
say user["name"]

say "--- 3. Loops (For-In & While) ---"

let fruits = ["Apple", "Banana", "Cherry"]
say "Iterating over array with for-in:"
for item in fruits:
    say item

let counter = 1
say "Counting up with while loop:"
while counter <= 3:
    say counter
    counter = counter + 1

say "--- 4. Object-Oriented Programming (Classes & Methods) ---"

class Person:
    name
    role

    function introduce():
        say "Hello! My name is:"
        say name

let p = Person()
p.name = "Alex"
p.introduce()

say "=================================================="
say "           Execution Completed Successfully        "
say "=================================================="
