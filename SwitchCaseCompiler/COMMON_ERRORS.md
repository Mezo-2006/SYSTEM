# Common Errors and Solutions

## Problem: C++ Code Doesn't Compile in SwitchCaseCompiler

This is a **specialized compiler for educational purposes**. It only accepts a **simplified switch-case language**, NOT full C++ code.

### ❌ WRONG - Full C++ Code (WILL NOT WORK)

```cpp
#include <iostream>
using namespace std;

int main() {
    int day;
    cout << "Enter a number (1-7): ";
    cin >> day;

    switch(day) {
        case 1:
            cout << "Sunday";
            break;
        case 2:
            cout << "Monday";
            break;
        default:
            cout << "Invalid!";
    }
    return 0;
}
```

**Error Messages You'll See:**
- "ERROR: C++ preprocessor directives (#include, #define) are not supported."
- "ERROR: C++ 'using namespace' is not supported."
- "ERROR: C++ function definitions (int main) are not supported."
- "ERROR: C++ I/O statements (cout/cin) are not supported."

### ✅ CORRECT - Switch-Case Only Syntax

```
switch (day) {
    case 1:
        result = 1;
        break;
    case 2:
        result = 2;
        break;
    case 3:
        result = 3;
        break;
    case 4:
        result = 4;
        break;
    case 5:
        result = 5;
        break;
    case 6:
        result = 6;
        break;
    case 7:
        result = 7;
        break;
    default:
        result = 0;
        break;
}
```

## What's Allowed in This Compiler?

### ✅ Supported:
- `switch` statements
- `case` labels with integer constants
- `default` clause
- Variable assignments (`variable = expression;`)
- Arithmetic expressions (`+`, `-`, `*`, `/`)
- `break` statements

### ❌ NOT Supported:
- `#include` directives
- `using namespace`
- `int main()` or any function definitions
- `cout` / `cin` (I/O operations)
- `return` statements
- String literals in case statements
- Variable declarations (`int x;`)
- Anything outside a switch statement

## How to Use This Compiler

1. **Input**: Write ONLY a switch-case statement (no includes, no main, no cout/cin)
2. **Variables**: Assume all variables are already declared and initialized
3. **Statements**: Use simple assignments in case bodies
4. **Purpose**: This compiler demonstrates compilation phases (lexical analysis, parsing, code generation)

## Example Programs

### Example 1: Simple Calculator
```
switch (operation) {
    case 1:
        result = x + y;
        break;
    case 2:
        result = x - y;
        break;
    case 3:
        result = x * y;
        break;
    case 4:
        result = x / y;
        break;
    default:
        result = 0;
        break;
}
```

### Example 2: Grade Calculator
```
switch (grade) {
    case 1:
        score = 90;
        break;
    case 2:
        score = 80;
        break;
    case 3:
        score = 70;
        break;
    default:
        score = 50;
        break;
}
```

## Why These Restrictions?

This is an **educational compiler** designed to teach:
- Lexical analysis
- Syntax parsing (rightmost derivation)
- Abstract syntax trees (AST)
- Semantic analysis
- Three-address code (TAC) generation
- Code optimization
- Assembly code generation

The simplified grammar makes it easier to understand compiler construction phases without the complexity of a full C++ compiler.

## Need to Run Actual C++ Code?

Use a standard C++ compiler instead:
- **Visual Studio** (Windows)
- **GCC** (g++ command)
- **Clang** (clang++ command)
- **Online compilers**: OnlineGDB, Compiler Explorer, etc.

Example with g++:
```bash
g++ -o program day_of_week.cpp
./program
```

## Getting Better Error Messages

**The improved compiler now provides descriptive errors when you accidentally paste C++ code:**

- ✅ Detects `#include` and explains it's not supported
- ✅ Detects `using namespace` and suggests correct syntax
- ✅ Detects `int main()` and explains the limitation
- ✅ Detects `cout`/`cin` and suggests using assignments
- ✅ Provides example of correct syntax in error messages

These improvements help you quickly understand what went wrong and how to fix it!
