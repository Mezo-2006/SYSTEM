#ifndef SEMANTIC_ANALYZER_H
#define SEMANTIC_ANALYZER_H

#include "AST.h"
#include "Lexer.h"
#include <string>
#include <vector>
#include <map>
#include <set>
#include <memory>

// Semantic error
struct SemanticError {
    std::string message;
    int line;
    int column;
    std::string errorType;  // "undeclared", "uninitialized", "type_mismatch", "duplicate_case", "duplicate_declaration", "unreachable"
    
    SemanticError(const std::string& msg, int ln, int col, const std::string& type)
        : message(msg), line(ln), column(col), errorType(type) {}
};

// Enhanced symbol table for semantic analysis
class ScopedSymbolTable {
private:
    struct Symbol {
        std::string name;
        std::string type;
        std::string value;
        int scopeLevel;
        int line;
        int column;
        bool initialized;
        
        // Default constructor for STL containers
        Symbol() : name(""), type(""), value(""), scopeLevel(0), line(0), column(0), initialized(false) {}
        
        Symbol(const std::string& n, const std::string& t, int scope, int ln, int col)
            : name(n), type(t), value(""), scopeLevel(scope), line(ln), column(col), 
              initialized(false) {}
    };
    
    std::vector<std::map<std::string, Symbol>> scopes;
    int currentScope;
    
public:
    ScopedSymbolTable();
    
    void enterScope();
    void exitScope();
    
    bool declare(const std::string& name, const std::string& type, int line, int col);
    bool isDeclared(const std::string& name);
    Symbol* lookup(const std::string& name);
    
    void setInitialized(const std::string& name);
    bool isInitialized(const std::string& name);
    
    int getCurrentScope() const { return currentScope; }
    const std::vector<std::map<std::string, Symbol>>& getAllScopes() const { return scopes; }
};

// Semantic Analyzer using visitor pattern
class SemanticAnalyzer : public ASTVisitor {
private:
    ScopedSymbolTable symbolTable;
    std::vector<SemanticError> errors;
    std::set<int> usedCaseValues;  // Track case values to detect duplicates
    bool afterBreak;  // Track unreachable code
    
    void addError(const std::string& message, int line, int col, const std::string& type);
    std::string inferType(Expression* expr);
    
public:
    SemanticAnalyzer();
    
    bool analyze(Program* program);
    
    const std::vector<SemanticError>& getErrors() const { return errors; }
    const ScopedSymbolTable& getSymbolTable() const { return symbolTable; }
    bool hasErrors() const { return !errors.empty(); }
    
    // Visitor methods
    void visit(Program* node) override;
    void visit(SwitchStatement* node) override;
    void visit(CaseClause* node) override;
    void visit(AssignmentStatement* node) override;
    void visit(DeclarationStatement* node) override;
    void visit(CinStatement* node) override;
    void visit(CoutStatement* node) override;
    void visit(BinaryExpression* node) override;
    void visit(Identifier* node) override;
    void visit(Constant* node) override;
    void visit(StringLiteral* node) override;
};

#endif // SEMANTIC_ANALYZER_H
