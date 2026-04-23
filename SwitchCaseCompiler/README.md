# Switch-Case Compiler - Educational Tool

An educational compiler for a simplified programming language supporting ONLY switch-case statements. This compiler demonstrates all standard compilation phases with a rich Qt-based GUI.

## Features

### Compiler Phases
1. **Lexical Analysis** - Hand-written scanner with symbol table
2. **Syntax Analysis** - Explicit rightmost derivation parser
3. **Semantic Analysis** - Type checking, undeclared variable detection
4. **Intermediate Code Generation** - Three-Address Code (TAC)
5. **Code Optimization** - Constant folding, dead code elimination, CSE
6. **Target Code Generation** - x86-like pseudo-assembly
7. **Target Code Optimization** - Instruction optimization

### GUI Features
- **Source Editor** - Syntax-highlighted editor with line numbers
- **Derivation Step Viewer** ⭐ - Animated rightmost derivation with step controls
- **Parse Tree Viewer** - Graphical tree rendering
- **Token Table** - Complete token stream
- **Symbol Table** - Variable tracking with scopes
- **Three-Column Trace** - Source/TAC/Assembly correlation
- **Optimization Panel** - Before/after comparison with toggles
- **Error Panel** - Comprehensive error reporting with navigation
- **Output Console** - Phase-by-phase compilation progress

## Language Grammar

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

## Example Program

```
switch (x) {
  case 1: y = x + 2; break;
  case 2: y = x * 3; break;
  case 3: z = y + x; break;
  default: y = 0; break;
}
```

## Requirements

- C++17 compatible compiler (MSVC, GCC, or Clang)
- Qt 5.12+ or Qt 6.x
- CMake 3.16+
- Visual Studio 2019+ (on Windows) or GCC 7+ (on Linux)

## Building the Project

### Windows (Visual Studio)

#### Option 1: Using CMake GUI
1. Open CMake GUI
2. Set source directory to: `SwitchCaseCompiler/`
3. Set build directory to: `SwitchCaseCompiler/build/`
4. Click "Configure" and select your Visual Studio version
5. Click "Generate"
6. Open the generated `.sln` file in Visual Studio
7. Build the solution (F7)
8. Run the executable from `build/Debug/SwitchCaseCompiler.exe`

#### Option 2: Using Command Line
```batch
cd SwitchCaseCompiler
mkdir build
cd build
cmake -G "Visual Studio 16 2019" ..
cmake --build . --config Release
.\Release\SwitchCaseCompiler.exe
```

#### Option 3: Using Qt Creator
1. Open Qt Creator
2. File → Open File or Project
3. Select `CMakeLists.txt` from the SwitchCaseCompiler directory
4. Configure project with your Qt kit
5. Build (Ctrl+B)
6. Run (Ctrl+R)

### Linux

```bash
cd SwitchCaseCompiler
mkdir build
cd build
cmake ..
make -j4
./SwitchCaseCompiler
```

### macOS

```bash
cd SwitchCaseCompiler
mkdir build
cd build
cmake ..
make -j4
./SwitchCaseCompiler
```

## Project Structure

```
SwitchCaseCompiler/
├── CMakeLists.txt
├── README.md
├── src/
│   ├── main.cpp
│   ├── core/
│   │   ├── Lexer.h / Lexer.cpp
│   │   ├── Parser.h / Parser.cpp
│   │   ├── AST.h / AST.cpp
│   │   ├── SemanticAnalyzer.h / SemanticAnalyzer.cpp
│   │   ├── TACGenerator.h / TACGenerator.cpp
│   │   ├── Optimizer.h / Optimizer.cpp
│   │   └── CodeGenerator.h / CodeGenerator.cpp
│   └── gui/
│       ├── MainWindow.h / MainWindow.cpp
│       ├── DerivationViewer.h / DerivationViewer.cpp
│       ├── ParseTreeView.h / ParseTreeView.cpp
│       ├── ThreeColumnView.h / ThreeColumnView.cpp
│       └── OptimizationPanel.h / OptimizationPanel.cpp
└── build/  (generated)
```

## Usage

1. Launch the application
2. Enter or paste switch-case code in the Source Editor tab
3. Click the "Compile" button
4. Explore the results in different tabs:
   - **Derivation Steps**: Watch the rightmost derivation unfold step-by-step
   - **Parse Tree**: See the hierarchical structure
   - **Token Table**: View all lexical tokens
   - **Symbol Table**: Check variable declarations
   - **Trace Panel**: Correlate source with TAC and assembly
   - **Optimizations**: See before/after optimization results
   - **Errors**: Review any compilation errors
   - **Console**: Read compilation phase messages

## Command-Line Mode (For Testing)

The project also builds a CLI executable for automated regression testing:

```batch
build\Release\SwitchCaseCompilerCLI.exe --input tests\cases\valid_basic_switch.txt
```

CLI exit codes:
- `0` success
- `1` lexical error
- `2` syntax error
- `3` semantic error

Run the regression suite:

```powershell
powershell -ExecutionPolicy Bypass -File tests/run_regression_tests.ps1
```

## Key Features Explained

### Rightmost Derivation Viewer
- Shows each step of the derivation process
- Highlights the rightmost non-terminal in red/bold
- Displays the production rule applied
- Step-by-step navigation with auto-play mode
- Adjustable animation speed

### Parser Implementation
- Uses explicit rightmost derivation (not hidden recursive descent)
- Switch-case based state machine (meta-requirement)
- Tracks every derivation step for visualization
- Error recovery to show multiple errors

### Optimizations
- **Constant Folding**: `y = 3 + 4` → `y = 7`
- **Dead Code Elimination**: Removes unreachable code
- **Common Subexpression Elimination**: Reuses computed expressions
- Toggle each optimization independently
- View before/after side-by-side

## ⚠️ Important: This is NOT a C++ Compiler!

**This compiler only accepts simplified switch-case statements, NOT full C++ code.**

If you're trying to compile code with `#include`, `cout`, `cin`, or `int main()`, see **[COMMON_ERRORS.md](COMMON_ERRORS.md)** for:
- Why your C++ code won't compile
- What syntax this compiler expects
- Helpful examples of correct programs
- Improved error messages that guide you

## Troubleshooting

### Qt Not Found
If CMake can't find Qt, set the Qt installation path:
```batch
cmake -DCMAKE_PREFIX_PATH="C:/Qt/6.5.0/msvc2019_64" ..
```

### Build Errors
- Ensure C++17 is enabled
- Check that all Qt modules are installed (Core, Widgets, Gui)
- Verify CMake version is 3.16+

### Runtime Issues
- Make sure Qt DLLs are in your PATH or next to the executable
- On Windows, use `windeployqt.exe` to copy required DLLs:
  ```batch
  windeployqt.exe Release\SwitchCaseCompiler.exe
  ```

## Contributing

This is an educational project. Feel free to:
- Add more optimization passes
- Enhance the GUI with syntax highlighting
- Extend the language grammar
- Improve error messages
- Add more comprehensive examples

## License

Educational use only. Developed as a compiler construction learning tool.

## Authors

Systems Programming Course Project

## Acknowledgments

- Compiler design principles from Dragon Book
- Qt framework for GUI development
- Educational compiler examples and resources
