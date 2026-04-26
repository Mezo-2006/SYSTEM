#include "SemanticAnalyzer.h"

// ScopedSymbolTable implementation
ScopedSymbolTable::ScopedSymbolTable() : currentScope(0) {
    scopes.push_back(std::map<std::string, Symbol>());  // Global scope
    declarationHistory.clear();
}

void ScopedSymbolTable::enterScope() {
    currentScope++;
    scopes.push_back(std::map<std::string, Symbol>());
}

void ScopedSymbolTable::exitScope() {
    if (currentScope > 0) {
        scopes.pop_back();
        currentScope--;
    }
}

bool ScopedSymbolTable::declare(const std::string& name, const std::string& type, 
                               int line, int col) {
    // Check if already declared in current scope
    if (scopes[currentScope].find(name) != scopes[currentScope].end()) {
        return false;  // Already declared
    }
    
    scopes[currentScope][name] = Symbol(name, type, currentScope, line, col);
    declarationHistory.emplace_back(name, type, "uninitialized", currentScope, line, col);
    return true;
}

bool ScopedSymbolTable::isDeclared(const std::string& name) {
    return lookup(name) != nullptr;
}

ScopedSymbolTable::Symbol* ScopedSymbolTable::lookup(const std::string& name) {
    // Search from current scope up to global scope
    for (int i = currentScope; i >= 0; i--) {
        auto it = scopes[i].find(name);
        if (it != scopes[i].end()) {
            return &(it->second);
        }
    }
    return nullptr;
}

void ScopedSymbolTable::setInitialized(const std::string& name) {
    Symbol* sym = lookup(name);
    if (sym) {
        sym->initialized = true;
        for (auto it = declarationHistory.rbegin(); it != declarationHistory.rend(); ++it) {
            if (it->name == sym->name && it->scope == sym->scopeLevel) {
                it->value = "initialized";
                break;
            }
        }
    }
}

bool ScopedSymbolTable::isInitialized(const std::string& name) {
    Symbol* sym = lookup(name);
    return sym && sym->initialized;
}

// SemanticAnalyzer implementation
SemanticAnalyzer::SemanticAnalyzer() : afterBreak(false) {}

bool SemanticAnalyzer::analyze(Program* program) {
    // Each compile must start from a clean semantic environment.
    symbolTable = ScopedSymbolTable();
    errors.clear();
    usedCaseValues.clear();
    afterBreak = false;
    
    if (program) {
        program->accept(this);
    }
    
    return errors.empty();
}

bool SemanticAnalyzer::analyze(TranslationUnit* tu) {
    // Each compile must start from a clean semantic environment.
    symbolTable = ScopedSymbolTable();
    errors.clear();
    usedCaseValues.clear();
    afterBreak = false;
    
    if (tu) {
        tu->accept(this);
    }
    
    return errors.empty();
}

// New visitor implementations
void SemanticAnalyzer::visit(TranslationUnit* node) {
    if (node->mainFunction) {
        node->mainFunction->accept(this);
    }
}

void SemanticAnalyzer::visit(FunctionDecl* node) {
    symbolTable.enterScope();
    if (node->body) {
        node->body->accept(this);
    }
    symbolTable.exitScope();
}

void SemanticAnalyzer::visit(CompoundStmt* node) {
    for (auto& stmt : node->statements) {
        stmt->accept(this);
    }
}

void SemanticAnalyzer::visit(DeclStmt* node) {
    if (node->declaration) {
        node->declaration->accept(this);
    }
}

void SemanticAnalyzer::visit(VarDecl* node) {
    if (!symbolTable.declare(node->name, node->type, node->line, node->column)) {
        addError("Variable '" + node->name + "' is already declared in this scope",
                node->line, node->column, "duplicate_declaration");
    }
    
    if (node->initializer) {
        node->initializer->accept(this);
        symbolTable.setInitialized(node->name);
    }
}

void SemanticAnalyzer::visit(DeclRefExpr* node) {
    if (!symbolTable.isDeclared(node->name)) {
        addError("Undeclared variable '" + node->name + "'",
                node->line, node->column, "undeclared");
    } else if (!symbolTable.isInitialized(node->name)) {
        addError("Variable '" + node->name + "' used before initialization",
                node->line, node->column, "uninitialized");
    }
}

void SemanticAnalyzer::visit(IntegerLiteral* node) {
    // Literals are always valid
}

void SemanticAnalyzer::visit(BreakStmt* node) {
    afterBreak = true;
}

void SemanticAnalyzer::visit(ReturnStmt* node) {
    if (node->returnValue) {
        node->returnValue->accept(this);
    }
    afterBreak = true;
}

void SemanticAnalyzer::visit(CaseStmt* node) {
    if (!node->isDefault) {
        if (usedCaseValues.find(node->caseValue) != usedCaseValues.end()) {
            addError("Duplicate case value: " + std::to_string(node->caseValue),
                    node->line, node->column, "duplicate_case");
        }
        usedCaseValues.insert(node->caseValue);
    }
    
    afterBreak = false;
    for (auto& stmt : node->statements) {
        if (afterBreak) {
            addError("Unreachable code after break/return",
                    stmt->line, stmt->column, "unreachable");
        }
        stmt->accept(this);
    }
}

void SemanticAnalyzer::addError(const std::string& message, int line, int col, 
                               const std::string& type) {
    errors.push_back(SemanticError(message, line, col, type));
}

std::string SemanticAnalyzer::inferType(Expression* expr) {
    if (dynamic_cast<Constant*>(expr)) {
        return "int";
    }
    else if (dynamic_cast<StringLiteral*>(expr)) {
        return "string";
    }
    else if (auto* id = dynamic_cast<Identifier*>(expr)) {
        auto* sym = symbolTable.lookup(id->name);
        if (sym) {
            return sym->type;
        }
        return "unknown";
    }
    else if (auto* binExpr = dynamic_cast<BinaryExpression*>(expr)) {
        // Stream operator << is always valid (used in cout chains)
        if (binExpr->op == "<<") {
            return "void";
        }
        std::string leftType = inferType(binExpr->left.get());
        std::string rightType = inferType(binExpr->right.get());

        // Simple type checking: both operands should be int
        if (leftType == "int" && rightType == "int") {
            return "int";
        }
        return "error";
    }
    
    return "unknown";
}

void SemanticAnalyzer::visit(Program* node) {
    for (auto& stmt : node->preSwitchStatements) {
        stmt->accept(this);
    }

    if (node->switchStmt) {
        node->switchStmt->accept(this);
    }
}

void SemanticAnalyzer::visit(SwitchStatement* node) {
    symbolTable.enterScope();
    
    // Check switch condition
    if (node->condition) {
        node->condition->accept(this);
        std::string condType = inferType(node->condition.get());
        node->annotatedType = condType;
        
        if (condType != "int" && condType != "unknown") {
            addError("Switch condition must be of integer type", 
                    node->line, node->column, "type_mismatch");
        }
    }
    
    // Clear used case values for this switch
    usedCaseValues.clear();
    
    // Check each case
    for (auto& caseClause : node->cases) {
        caseClause->accept(this);
    }
    
    // Check default clause
    if (node->defaultCase) {
        node->defaultCase->accept(this);
    }
    
    symbolTable.exitScope();
}

void SemanticAnalyzer::visit(CaseClause* node) {
    // Check for duplicate case values
    if (!node->isDefault) {
        if (usedCaseValues.find(node->caseValue) != usedCaseValues.end()) {
            addError("Duplicate case value: " + std::to_string(node->caseValue),
                    node->line, node->column, "duplicate_case");
        }
        usedCaseValues.insert(node->caseValue);
    }
    
    // Check statements
    afterBreak = false;
    for (size_t i = 0; i < node->statements.size(); i++) {
        if (afterBreak) {
            addError("Unreachable code after break statement",
                    node->statements[i]->line, node->statements[i]->column, "unreachable");
        }
        node->statements[i]->accept(this);
    }
    
    // After processing all statements in a case, we hit the break
    afterBreak = true;
}

void SemanticAnalyzer::visit(AssignmentStatement* node) {
    // Assignment requires prior declaration.
    if (!symbolTable.isDeclared(node->variableName)) {
        addError("Variable '" + node->variableName + "' must be declared before assignment",
                node->line, node->column, "undeclared");
        symbolTable.declare(node->variableName, "int", node->line, node->column);
    }
    
    // Check expression
    if (node->expression) {
        node->expression->accept(this);
        std::string exprType = inferType(node->expression.get());
        node->annotatedType = exprType;
        
        // Type checking
        auto* sym = symbolTable.lookup(node->variableName);
        if (sym && sym->type != exprType && exprType != "unknown" && exprType != "error") {
            addError("Type mismatch in assignment to '" + node->variableName + "'",
                    node->line, node->column, "type_mismatch");
        }
    }
    
    // Mark variable as initialized
    symbolTable.setInitialized(node->variableName);
}

void SemanticAnalyzer::visit(DeclarationStatement* node) {
    if (node->variableType != "int" && node->variableType != "string" && node->variableType != "void") {
        addError("Only 'int' and 'string' declarations are supported in this mini-language",
                node->line, node->column, "type_mismatch");
        return;
    }

    // Skip semantic checks for void type (used for cout statements)
    if (node->variableType == "void") {
        return;
    }

    if (!symbolTable.declare(node->variableName, node->variableType, node->line, node->column)) {
        addError("Duplicate declaration of variable '" + node->variableName + "'",
                node->line, node->column, "duplicate_declaration");
    }

    if (node->initializer) {
        node->initializer->accept(this);
        std::string exprType = inferType(node->initializer.get());
        if (exprType != node->variableType && exprType != "unknown" && exprType != "error") {
            addError("Type mismatch in declaration of '" + node->variableName + "'",
                    node->line, node->column, "type_mismatch");
        }
        symbolTable.setInitialized(node->variableName);
    }
}

void SemanticAnalyzer::visit(BinaryExpression* node) {
    if (node->left)  node->left->accept(this);
    if (node->right) node->right->accept(this);

    // Guard against null children (can happen after parse error recovery)
    if (!node->left || !node->right) {
        node->annotatedType = "error";
        return;
    }

    std::string leftType  = inferType(node->left.get());
    std::string rightType = inferType(node->right.get());

    // Allow << operator (stream output) with strings — it's not arithmetic
    if (node->op != "<<" && (leftType == "string" || rightType == "string")) {
        addError("String operands with arithmetic operators are not supported; use direct string assignment only",
                node->line, node->column, "type_mismatch");
        node->annotatedType = "error";
        return;
    }

    // Infer and annotate type
    node->annotatedType = inferType(node);
}

void SemanticAnalyzer::visit(Identifier* node) {
    // Special identifiers that are valid but not declared in the symbol table
    static const std::set<std::string> builtins = {
        "endl", "std", "cin", "cout", "true", "false", "NULL", "nullptr"
    };
    if (builtins.count(node->name)) {
        node->annotatedType = "unknown";  // valid, just not tracked
        return;
    }

    // Check if identifier is declared
    if (!symbolTable.isDeclared(node->name)) {
        addError("Undeclared variable '" + node->name + "'",
                node->line, node->column, "undeclared");
        node->annotatedType = "error";
    } else {
        auto* sym = symbolTable.lookup(node->name);
        if (sym) {
            node->annotatedType = sym->type;
            if (!sym->initialized) {
                addError("Variable '" + node->name + "' may be used before initialization",
                        node->line, node->column, "uninitialized");
            }
        }
    }
}

void SemanticAnalyzer::visit(Constant* node) {
    node->annotatedType = "int";
}

void SemanticAnalyzer::visit(StringLiteral* node) {
    node->annotatedType = "string";
}

void SemanticAnalyzer::visit(CinStatement* node) {
    if (!symbolTable.isDeclared(node->variableName)) {
        addError("Variable '" + node->variableName + "' is used before it is declared",
                node->line, node->column, "undeclared");
    } else {
        symbolTable.setInitialized(node->variableName);
    }
}

void SemanticAnalyzer::visit(CoutStatement* node) {
    if (node->expression) {
        node->expression->accept(this);
    }
}
