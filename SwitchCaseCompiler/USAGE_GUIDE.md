# Switch-Case Compiler - Complete Usage Guide

## ✅ PROJECT STATUS: **FULLY COMPLETE AND WORKING**

The executable has been successfully built and is ready to use!

**Executable Location:** `bin\SwitchCaseCompiler.exe`

---

## 🚀 Quick Start

### Method 1: Run Pre-built Executable
```cmd
cd "c:\Users\Mizo\OneDrive - Arab Academy for Science and Technology\Desktop\systems 2\SwitchCaseCompiler\bin"
SwitchCaseCompiler.exe
```

### Method 2: Build from Source
```cmd
cd "c:\Users\Mizo\OneDrive - Arab Academy for Science and Technology\Desktop\systems 2\SwitchCaseCompiler"
build.bat
```

---

## 📚 Compilation Errors Fixed

The following errors were encountered and **successfully resolved**:

### 1. **Function Name Conflicts**
- **Problem:** MSVC compiler had issues with function named `emit` 
- **Solution:** Renamed to `emitInstruction` in TACGenerator and `emitAsm` in CodeGenerator

### 2. **Constructor Ambiguity in AST.h**
- **Problem:** `CaseClause` had two constructors with overlapping signatures
- **Solution:** Changed constructors to:
  ```cpp
  CaseClause(int val, int ln, int col)           // Regular case
  CaseClause(bool isDefaultCase, int ln, int col) // Default case
  ```

### 3. **Missing Default Constructor**
- **Problem:** `ScopedSymbolTable::Symbol` needed default constructor for STL containers
- **Solution:** Added default constructor:
  ```cpp
  Symbol() : name(""), type(""), value(""), scopeLevel(0), line(0), column(0), initialized(false) {}
  ```

---

## 📋 How to Use the Compiler

### Step 1: Launch the Application
Double-click `bin\SwitchCaseCompiler.exe` or run from command line.

### Step 2: Enter Source Code
In the **Source Editor** tab, enter switch-case code:

```c
switch (x) {
  case 1: y = x + 2; break;
  case 2: y = x * 3; break;
  case 3: z = (y + x) * 2; break;
  default: y = 0; break;
}
```

### Step 3: Click "Compile"
The compiler will execute all 7 phases automatically.

### Step 4: Explore the Results

Navigate through the tabs to see:

#### **Tab 1: Source Editor**
- Your input code with line numbers
- Syntax highlighting (basic)
- Compile button
- Status indicator (green=success, red=error)

#### **Tab 2: Derivation Steps** ⭐ (Most Important)
- Shows **explicit rightmost derivation**
- Each step displays:
  - Current sentential form
  - Rightmost non-terminal highlighted in **RED/BOLD**
  - Production rule applied
- Controls:
  - ◀ Step Back
  - Step Forward ▶
  - ▶ Auto Play (with speed slider)

#### **Tab 3: Parse Tree**
- Graphical tree visualization
- Blue circles = non-terminals
- Green circles = terminals
- Click nodes to see derivation step

#### **Tab 4: Token Table**
- Complete lexical token stream
- Columns: Token | Type | Line | Column

#### **Tab 5: Symbol Table**
- All identified variables
- Columns: Name | Type | Value | Scope | Line | Column

#### **Tab 6: Source/TAC/Assembly Trace**
- Three-column synchronized view:
  - **Left:** Original source code
  - **Center:** Three-Address Code (TAC)
  - **Right:** Generated assembly

#### **Tab 7: Optimizations**
- Checkboxes to toggle optimizations:
  - ✅ Constant Folding
  - ✅ Dead Code Elimination
  - ✅ Common Subexpression Elimination
- **Before/After** side-by-side comparison
- Shows number of instructions removed

#### **Tab 8: Errors**
- Lists all compilation errors
- Columns: Phase | Type | Line | Column | Message
- **Double-click** an error to jump to source location

#### **Tab 9: Console**
- Phase-by-phase compilation progress
- Messages from each compiler phase

---

## 🎯 Supported Language Features

### Valid Syntax
```c
// Simple case
switch (x) {
  case 1: y = 5; break;
}

// Multiple cases
switch (value) {
  case 1: a = b + c; break;
  case 2: a = b * c; break;
  case 3: a = b - c; break;
  default: a = 0; break;
}

// Nested expressions
switch (x) {
  case 1: y = (a + b) * (c - d); break;
  case 2: z = x / 2 + 3; break;
}

// Multiple statements per case
switch (x) {
  case 1: 
    y = x + 2;
    z = y * 3;
    break;
  case 2:
    a = b;
    c = d;
    break;
}
```

### Keywords
- `switch`, `case`, `default`, `break`

### Operators
- Arithmetic: `+`, `-`, `*`, `/`
- Assignment: `=`

### Delimiters
- Parentheses: `(`, `)`
- Braces: `{`, `}`
- Colon: `:`
- Semicolon: `;`

### Comments
```c
// Single-line comment

/* Multi-line
   comment */
```

---

## 🔧 Compilation Phases Demonstrated

### Phase 1: Lexical Analysis
- Hand-written character-by-character scanner
- Token recognition
- Symbol table construction
- Comment removal
- Error reporting with line/column

### Phase 2: Syntax Analysis
- **Explicit rightmost derivation parser**
- NOT hidden recursive descent
- Uses **switch-case** logic internally (meta-requirement)
- Stores every derivation step
- Error recovery for multiple errors

### Phase 3: Semantic Analysis
- Scoped symbol table
- Undeclared variable detection
- Type checking
- Duplicate case value detection
- Unreachable code detection

### Phase 4: Intermediate Code (TAC)
- Three-address code generation
- Switch converted to conditional jumps
- Label-based control flow

### Phase 5: Code Optimization
1. **Constant Folding**
   - `y = 3 + 4` → `y = 7`
   
2. **Dead Code Elimination**
   - Removes unreachable code after `goto`
   
3. **Common Subexpression Elimination**
   - Reuses previously computed expressions

### Phase 6: Target Code Generation
- x86-like pseudo-assembly
- Instructions: MOV, ADD, SUB, MUL, DIV, CMP, JE, JNE, JMP
- Register allocation

### Phase 7: Target Code Optimization
- Redundant instruction removal
- Instruction reordering
- Peephole optimization

---

## 📂 Project Structure

```
SwitchCaseCompiler/
├── bin/
│   └── SwitchCaseCompiler.exe  ← YOUR EXECUTABLE
├── src/
│   ├── core/
│   │   ├── Lexer.h / Lexer.cpp           ✅ Complete
│   │   ├── Parser.h / Parser.cpp         ✅ Complete (Rightmost derivation)
│   │   ├── AST.h / AST.cpp               ✅ Complete
│   │   ├── SemanticAnalyzer.h / .cpp     ✅ Complete
│   │   ├── TACGenerator.h / .cpp         ✅ Complete
│   │   ├── Optimizer.h / .cpp            ✅ Complete
│   │   └── CodeGenerator.h / .cpp        ✅ Complete
│   ├── gui/
│   │   ├── MainWindow.h / .cpp           ✅ Complete
│   │   ├── DerivationViewer.h / .cpp     ✅ Complete (Step viewer)
│   │   ├── ParseTreeView.h / .cpp        ✅ Complete (Tree renderer)
│   │   ├── ThreeColumnView.h / .cpp      ✅ Complete
│   │   └── OptimizationPanel.h / .cpp    ✅ Complete
│   └── main.cpp                          ✅ Complete
├── CMakeLists.txt                        ✅ Qt6 compatible
├── build.bat                             ✅ Windows build script
├── SwitchCaseCompiler.sln                ✅ Visual Studio solution
├── SwitchCaseCompiler.vcxproj            ✅ VS project file
├── README.md                             ✅ Documentation
└── USAGE_GUIDE.md                        ✅ This file
```

---

## 🎓 Educational Value

This compiler demonstrates:

1. **Complete Compilation Pipeline**
   - All standard phases from source to assembly

2. **Rightmost Derivation Visualization**
   - Explicit step-by-step derivation
   - Not hidden in recursive descent

3. **Error Handling**
   - Multiple error reporting
   - Error recovery
   - Precise error locations

4. **Optimization Techniques**
   - Three major optimization algorithms
   - Before/after visualization

5. **Modern C++ Practices**
   - C++17 features
   - Smart pointers (`unique_ptr`)
   - Visitor pattern
   - STL containers

6. **GUI Development**
   - Qt6 framework
   - Multiple synchronized views
   - Interactive visualization

---

## 🐛 Troubleshooting

### Application Won't Start
**Problem:** Missing Qt DLLs
**Solution:**
```cmd
cd "C:\Qt\6.8.3\msvc2022_64\bin"
windeployqt "c:\Users\Mizo\OneDrive - Arab Academy for Science and Technology\Desktop\systems 2\SwitchCaseCompiler\bin\SwitchCaseCompiler.exe"
```

### Build Errors
**Problem:** Can't find Qt
**Solution:** Edit `build.bat` line 29:
```batch
set QT_PATH=C:\Qt\6.8.3\msvc2022_64
```

### Parse Tree Not Showing
**Problem:** No input compiled yet
**Solution:** Enter code and click "Compile" first

### Derivation Steps Empty
**Problem:** Syntax errors in code
**Solution:** Check Errors tab for issues

---

## 📝 Example Test Cases

### Test 1: Simple Switch
```c
switch (x) {
  case 1: y = 10; break;
  default: y = 0; break;
}
```
**Expected:** 2 cases, basic TAC, simple assembly

### Test 2: Arithmetic Expressions
```c
switch (x) {
  case 1: y = x + 2 * 3; break;
  case 2: z = (a + b) / 2; break;
}
```
**Expected:** Constant folding `2 * 3 = 6`, proper operator precedence

### Test 3: Multiple Statements
```c
switch (value) {
  case 1: 
    a = x + 1;
    b = y + 1;
    c = a + b;
    break;
  case 2:
    a = 0;
    b = 0;
    break;
}
```
**Expected:** Multiple TAC instructions per case, proper scope

### Test 4: Common Subexpression
```c
switch (x) {
  case 1: 
    y = a + b;
    z = a + b;
    break;
}
```
**Expected:** CSE optimization reuses `t1 = a + b`

---

## 🎨 GUI Keyboard Shortcuts

- **F5** - Compile
- **Ctrl+Tab** - Next tab
- **Ctrl+Shift+Tab** - Previous tab
- **Ctrl+Q** - Quit

---

## 📖 Grammar Reference

```
program     → switch_stmt
switch_stmt → switch ( expr ) { case_list default_clause }
case_list   → case_clause case_list | ε
case_clause → case constant : stmt_list break ;
default_clause → default : stmt_list break ; | ε
stmt_list   → stmt stmt_list | ε
stmt        → id = expr ;
expr        → expr + term | expr - term | term
term        → term * factor | term / factor | factor
factor      → ( expr ) | id | constant
```

---

## 🏆 Project Completion Checklist

- ✅ Lexical Analyzer (hand-written)
- ✅ Syntax Analyzer (rightmost derivation)
- ✅ Semantic Analyzer (type checking, scopes)
- ✅ TAC Generator (three-address code)
- ✅ Code Optimizer (constant folding, dead code, CSE)
- ✅ Code Generator (x86-like assembly)
- ✅ Target Optimizer (instruction optimization)
- ✅ Source Editor GUI
- ✅ Derivation Step Viewer (with animation)
- ✅ Parse Tree Viewer (graphical)
- ✅ Token Table
- ✅ Symbol Table Explorer
- ✅ Three-Column Trace Panel
- ✅ Optimization Control Panel
- ✅ Error Panel
- ✅ Output Console
- ✅ Full Qt6 GUI integration
- ✅ Windows executable built
- ✅ All compilation errors fixed
- ✅ Documentation complete

---

## 🎓 For Grading/Demonstration

### What to Show:

1. **Launch the application** - Shows it works
2. **Enter sample code** - Demonstrates language support
3. **Click Compile** - Runs all phases
4. **Derivation Steps tab** ⭐ - The KEY requirement
   - Show step-by-step derivation
   - Highlight rightmost non-terminal
   - Use auto-play feature
5. **Parse Tree** - Visual representation
6. **Token/Symbol Tables** - Lexical phase output
7. **TAC/Assembly** - Code generation phases
8. **Optimizations** - Before/after comparison
9. **Error handling** - Enter invalid code, show error detection

### Key Features to Highlight:

✅ **Rightmost Derivation is EXPLICIT** - Not hidden
✅ **Switch-case parser logic** - Meta-requirement
✅ **All 7 phases working** - Complete pipeline
✅ **Rich GUI** - Professional Qt interface
✅ **Error recovery** - Shows multiple errors
✅ **Educational value** - Clear visualization

---

## 📞 Support

For issues or questions:
1. Check this guide
2. Review README.md
3. Examine console output
4. Check Errors tab in application

---

**Congratulations! You have a fully functional educational compiler!** 🎉

Last Updated: April 7, 2026
Version: 1.0 (Complete)
