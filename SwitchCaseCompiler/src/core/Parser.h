#ifndef PARSER_H
#define PARSER_H

#include "Lexer.h"
#include "AST.h"
#include <vector>
#include <string>
#include <memory>

// Parse tree node for visualization
#include <memory>
#include <string>
#include <vector>
#include "Lexer.h"

struct ParseTreeNode {
    std::string label;
    bool isTerminal;
    std::vector<std::unique_ptr<ParseTreeNode>> children;
    int derivationStep;  // Which derivation step created this node
    const Token* token;  // Pointer back to the generating token for editor highlighting
    
    ParseTreeNode(const std::string& lbl, bool term = false, int step = -1, const Token* tok = nullptr)
        : label(lbl), isTerminal(term), derivationStep(step), token(tok) {}
};

// Derivation step for visualization
struct DerivationStep {
    std::string sententialForm;
    std::string productionRule;
    int rightmostNonTerminalPos;  // Position of rightmost non-terminal
    std::string rightmostNonTerminal;
    
    DerivationStep(const std::string& form, const std::string& rule, 
                  int pos = -1, const std::string& nonTerm = "")
        : sententialForm(form), productionRule(rule), 
          rightmostNonTerminalPos(pos), rightmostNonTerminal(nonTerm) {}
};

// Parser error
struct ParseError {
    std::string message;
    int line;
    int column;
    std::string expected;
    std::string found;
    
    ParseError(const std::string& msg, int ln, int col, 
              const std::string& exp = "", const std::string& fnd = "")
        : message(msg), line(ln), column(col), expected(exp), found(fnd) {}
};

// Rightmost Derivation Parser
class Parser {
private:
    struct IncludeDirective {
        bool angled;
        std::string headerLeft;
        std::string headerRight;
    };

    std::vector<Token> tokens;
    size_t currentTokenIndex;
    Token eofToken{TokenType::END_OF_FILE, "", 0, 0};
    std::vector<DerivationStep> derivationSteps;
    std::vector<ParseError> errors;
    std::unique_ptr<ParseTreeNode> parseTree;
    std::unique_ptr<Program> astRoot;  // Legacy
    std::unique_ptr<TranslationUnit> translationUnit;  // New
    std::vector<IncludeDirective> includeDirectives;
    bool hasUsingNamespaceStd = false;
    bool usesMainWrapper = false;
    int parsedReturnValue = 0;
    bool useNewAST = true;  // Flag to control which AST to build
    
    // Current token access
    Token& currentToken();
    Token& peekToken(int offset = 1);
    bool match(TokenType type);
    void consume(TokenType type, const std::string& errorMsg);
    void advance();
    bool isTypeToken(TokenType type) const;
    
    // Derivation tracking
    void addDerivationStep(const std::string& sententialForm, 
                          const std::string& rule,
                          const std::string& rightmostNonTerm);
    
    // Recursive descent parsing methods (building AST)
    std::unique_ptr<Program> parseProgram();
    std::unique_ptr<SwitchStatement> parseSwitchStatement();
    std::vector<std::unique_ptr<CaseClause>> parseCaseList();
    std::unique_ptr<CaseClause> parseCaseClause();
    std::unique_ptr<CaseClause> parseDefaultClause();
    std::vector<std::unique_ptr<Statement>> parseStatementList();
    std::unique_ptr<Statement> parseStatement();
    std::unique_ptr<Statement> parseDeclarationStatement();
    std::unique_ptr<Statement> parseAssignmentStatement();
    std::unique_ptr<Statement> parseCinStatement();
    std::unique_ptr<Statement> parseCoutStatement();
    std::unique_ptr<Expression> parseExpression();
    std::unique_ptr<Expression> parseTerm();
    std::unique_ptr<Expression> parseFactor();
    
    // AST conversion
    std::unique_ptr<TranslationUnit> convertToTranslationUnit(Program* program);
    std::unique_ptr<Expression> convertExpression(Expression* expr);
    std::unique_ptr<Statement> convertStatement(Statement* stmt);
    
    // Parse tree construction (for visualization)
    std::unique_ptr<ParseTreeNode> buildParseTree();
    
    // Rightmost derivation (for visualization)
    void performRightmostDerivation();
    
    // Validation helpers (ensure code is pure switch-case)
    void validateNoLoops();           // Reject for, while, do
    void validateOnlyOneSwitch();     // Require exactly one switch
    void validateOnlyValidStatements(); // Allow only: declarations, assignments, cin/cout, switch
    
    // Derivation reconstruction
    std::string reconstructSourceFromParseTree() const;
    
    // Error handling
    void addError(const std::string& message, const std::string& expected = "");
    void addErrorAtPrev(const std::string& message, const std::string& expected = "");
    void synchronize();  // Error recovery
    
public:
    Parser();
    explicit Parser(const std::vector<Token>& tokenStream);
    
    void setTokens(const std::vector<Token>& tokenStream);
    bool parse();
    
    // Phase 2 Validation (enforce C++ compliance)
    void validateMainWrapper();       // Require int main() {...}
    void validateIncludeStatements(); // Enforce #include <iostream> or <string>
    
    const std::vector<DerivationStep>& getDerivationSteps() const { return derivationSteps; }
    const std::vector<ParseError>& getErrors() const { return errors; }
    ParseTreeNode* getParseTree() const { return parseTree.get(); }
    Program* getAST() const { return astRoot.get(); }
    TranslationUnit* getTranslationUnit() const { return translationUnit.get(); }
    
    bool hasErrors() const { return !errors.empty(); }
    void setUseNewAST(bool use) { useNewAST = use; }
};

#endif // PARSER_H
