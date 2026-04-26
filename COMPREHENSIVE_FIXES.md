# Comprehensive Fixes for SwitchCaseCompiler

## Issues Identified

### 1. iostream Should NOT Be in Symbol Table
**Status:** ✅ FIXED in Lexer.cpp

**Problem:** `iostream` after `#include <iostream>` was being added to symbol table as an identifier.

**Solution:** Modified `scanIdentifierOrKeyword()` to detect header file names after `#include <` and skip adding them to the symbol table.

### 2. Token Count Mismatch
**Status:** ⚠️ NEEDS INVESTIGATION

**Problem:** Number of tokens shown in compile pipeline differs from token chart.

**Likely Cause:** 
- Console log shows: `tokens.size()` which includes EOF token
- Token table might filter out EOF or other special tokens

**Solution Needed:**
Check `updateTokenTable()` in MainWindow.cpp to see if it filters tokens. The count should match exactly.

### 3. AST Structure Not Clear
**Status:** ⚠️ NEEDS IMPROVEMENT

**Problem:** Current AST shows:
```
Program
└── switch
    ├── condition
    │   └── x
    ├── case 5
    │   └── assign
    └── ...
```

**Expected Structure (like Clang AST):**
```
TranslationUnit (Program)
└── FunctionDecl: main (returns int)
    └── CompoundStmt (body)
        ├── DeclStmt
        │   └── VarDecl: x (type: int)
        │       └── IntegerLiteral: 5
        ├── SwitchStmt
        │   ├── Condition
        │   │   └── ImplicitCastExpr (LValueToRValue)
        │   │       └── DeclRefExpr: x (type: int)
        │   └── CompoundStmt (body)
        │       └── CaseStmt
        │           ├── LHS
        │           │   └── IntegerLiteral: 5
        │           └── BreakStmt
        └── ReturnStmt
            └── IntegerLiteral: 0
```

**Changes Needed:**

1. **Add new AST node types:**
   - `TranslationUnit` (rename Program)
   - `FunctionDecl` (for main function)
   - `CompoundStmt` (for statement blocks)
   - `DeclStmt` (declaration statement wrapper)
   - `VarDecl` (variable declaration)
   - `DeclRefExpr` (variable reference)
   - `IntegerLiteral` (rename Constant)
   - `BreakStmt` (break statement)
   - `ReturnStmt` (return statement)
   - `ImplicitCastExpr` (type casting)

2. **Update AST.h structure:**
```cpp
class TranslationUnit : public ASTNode {
    std::unique_ptr<FunctionDecl> mainFunction;
};

class FunctionDecl : public ASTNode {
    std::string name;
    std::string returnType;
    std::unique_ptr<CompoundStmt> body;
};

class CompoundStmt : public ASTNode {
    std::vector<std::unique_ptr<Statement>> statements;
};

class DeclStmt : public Statement {
    std::unique_ptr<VarDecl> declaration;
};

class VarDecl : public ASTNode {
    std::string type;
    std::string name;
    std::unique_ptr<Expression> initializer;
};

class DeclRefExpr : public Expression {
    std::string name;
    std::string type;
};

class IntegerLiteral : public Expression {
    int value;
};

class BreakStmt : public Statement {};

class ReturnStmt : public Statement {
    std::unique_ptr<Expression> returnValue;
};
```

3. **Update Parser to build proper hierarchy:**
   - Wrap everything in TranslationUnit
   - Create FunctionDecl for main()
   - Use CompoundStmt for statement blocks
   - Wrap declarations in DeclStmt
   - Add BreakStmt and ReturnStmt nodes

4. **Update ASTView.cpp to display new nodes:**
   - Add visit methods for new node types
   - Use appropriate colors and labels
   - Show type information in sublabels

## Implementation Priority

1. **HIGH:** Fix iostream symbol table issue (DONE)
2. **HIGH:** Fix 3AC table format (DONE in previous fix)
3. **MEDIUM:** Investigate token count mismatch
4. **LOW:** Improve AST structure (requires significant refactoring)

## Notes

- The AST restructuring is a major change that affects Parser, AST, and all visitors
- Consider if the current simplified AST is sufficient for educational purposes
- Full Clang-style AST might be overkill for a switch-case-only compiler
- Could add a "detailed view" toggle instead of completely replacing current AST

