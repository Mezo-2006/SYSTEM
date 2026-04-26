# Fixes Applied to SwitchCaseCompiler

## Issue 1: iostream added to symbol table as identifier
**Fixed in:** `SwitchCaseCompiler/src/core/Lexer.cpp`

**Change:** Modified `scanIdentifierOrKeyword()` to detect when an identifier appears after `#include <` and skip adding it to the symbol table.

```cpp
// Don't add header file names (like iostream) to symbol table
// Check if this identifier appears after #include <
bool isHeaderFile = false;
if (tokens.size() >= 2) {
    size_t lastIdx = tokens.size() - 1;
    if (tokens[lastIdx].type == TokenType::LT && 
        tokens[lastIdx - 1].type == TokenType::INCLUDE) {
        isHeaderFile = true;
    }
}

// Only add to symbol table if it's not a header file
if (!isHeaderFile) {
    addToSymbolTable(identifier, "identifier");
}
```

## Issue 2: 3AC table format incorrect
**Fixed in:** `SwitchCaseCompiler/src/core/TACGenerator.cpp`

**Change:** Modified `toString()` method for `if_goto` instruction to match expected format:

**Before:**
```cpp
case TACOpcode::IF_GOTO:
    ss << "if " << arg1 << " goto " << result;
    break;
```

**After:**
```cpp
case TACOpcode::IF_GOTO:
    ss << "if_goto " << result << ", " << arg1;
    break;
```

This changes the display from:
- `if t0 goto L1` 

To:
- `if_goto L1, t0`

Which properly separates into table columns as:
- OPCODE: `if_goto`
- RESULT: `L1`
- ARG1: `t0`

## To Apply These Fixes:

The source files have been modified. To rebuild:

1. Clean and reconfigure with correct Qt path:
```bash
cd SwitchCaseCompiler
rm -rf build
mkdir build
cd build
cmake -DCMAKE_PREFIX_PATH="D:\Qt\6.8.3\msvc2022_64" ..
```

2. Build with license bypass:
```bash
$env:QTFRAMEWORK_BYPASS_LICENSE_CHECK = "1"
cmake --build . --config Release
```

3. Run the GUI:
```bash
.\Release\SwitchCaseCompiler.exe
```

## Expected Results:

1. **iostream** will no longer appear in the symbol table
2. **3AC table** will show:
   - Line 1: `=` | `x` | `5` | 
   - Line 2: `==` | `t0` | `x` | `5`
   - Line 3: `if_goto` | `L1` | `t0` | 
   - Line 4: `goto` | `L0` | | 
   - Line 5: `LABEL` | `L1` | | 
   - Line 6: `goto` | `L0` | | 
   - Line 7: `LABEL` | `L0` | | 
