#ifndef AST_H
#define AST_H

#include <string>
#include <vector>
#include <memory>

// Forward declarations
class ASTVisitor;

// Base AST Node
class ASTNode {
public:
    int line;
    int column;
    std::string annotatedType;  // For semantic analysis
    
    ASTNode(int ln = 0, int col = 0) : line(ln), column(col), annotatedType("") {}
    virtual ~ASTNode() = default;
    
    virtual void accept(ASTVisitor* visitor) = 0;
    virtual std::string toString() const = 0;
};

// Expression nodes
class Expression : public ASTNode {
public:
    Expression(int ln = 0, int col = 0) : ASTNode(ln, col) {}
};

class BinaryExpression : public Expression {
public:
    std::string op;  // +, -, *, /
    std::unique_ptr<Expression> left;
    std::unique_ptr<Expression> right;
    
    BinaryExpression(const std::string& operation, 
                    std::unique_ptr<Expression> l, 
                    std::unique_ptr<Expression> r,
                    int ln = 0, int col = 0)
        : Expression(ln, col), op(operation), left(std::move(l)), right(std::move(r)) {}
    
    void accept(ASTVisitor* visitor) override;
    std::string toString() const override;
};

class Identifier : public Expression {
public:
    std::string name;
    
    Identifier(const std::string& n, int ln = 0, int col = 0)
        : Expression(ln, col), name(n) {}
    
    void accept(ASTVisitor* visitor) override;
    std::string toString() const override;
};

class Constant : public Expression {
public:
    int value;
    
    Constant(int val, int ln = 0, int col = 0)
        : Expression(ln, col), value(val) {}
    
    void accept(ASTVisitor* visitor) override;
    std::string toString() const override;
};

class StringLiteral : public Expression {
public:
    std::string value;

    StringLiteral(const std::string& val, int ln = 0, int col = 0)
        : Expression(ln, col), value(val) {}

    void accept(ASTVisitor* visitor) override;
    std::string toString() const override;
};

// Statement nodes
class Statement : public ASTNode {
public:
    Statement(int ln = 0, int col = 0) : ASTNode(ln, col) {}
};

class AssignmentStatement : public Statement {
public:
    std::string variableName;
    std::unique_ptr<Expression> expression;
    
    AssignmentStatement(const std::string& varName, 
                       std::unique_ptr<Expression> expr,
                       int ln = 0, int col = 0)
        : Statement(ln, col), variableName(varName), expression(std::move(expr)) {}
    
    void accept(ASTVisitor* visitor) override;
    std::string toString() const override;
};

class DeclarationStatement : public Statement {
public:
    std::string variableType;
    std::string variableName;
    std::unique_ptr<Expression> initializer;

    DeclarationStatement(const std::string& varType,
                        const std::string& varName,
                        std::unique_ptr<Expression> init,
                        int ln = 0, int col = 0)
        : Statement(ln, col), variableType(varType), variableName(varName), initializer(std::move(init)) {}

    void accept(ASTVisitor* visitor) override;
    std::string toString() const override;
};

// Case clause

// I/O Statements
class CinStatement : public Statement {
public:
    std::string variableName;

    CinStatement(const std::string& varName, int ln = 0, int col = 0)
        : Statement(ln, col), variableName(varName) {}

    void accept(ASTVisitor* visitor) override;
    std::string toString() const override;
};

class CoutStatement : public Statement {
public:
    std::unique_ptr<Expression> expression;

    CoutStatement(std::unique_ptr<Expression> expr, int ln = 0, int col = 0)
        : Statement(ln, col), expression(std::move(expr)) {}

    void accept(ASTVisitor* visitor) override;
    std::string toString() const override;
};

class CaseClause : public ASTNode {
public:
    int caseValue;
    std::vector<std::unique_ptr<Statement>> statements;
    bool isDefault;
    bool hasBreak; // Tracks whether this clause explicitly declared a break

    // Constructor for regular case with value
    explicit CaseClause(int val, int ln, int col)
        : ASTNode(ln, col), caseValue(val), isDefault(false), hasBreak(false) {}

    // Constructor for default case (no value needed)
    CaseClause(bool isDefaultCase, int ln, int col)
        : ASTNode(ln, col), caseValue(-1), isDefault(isDefaultCase), hasBreak(false) {}

    void accept(ASTVisitor* visitor) override;
    std::string toString() const override;
};

// Switch statement
class SwitchStatement : public Statement {
public:
    std::unique_ptr<Expression> condition;
    std::vector<std::unique_ptr<CaseClause>> cases;
    std::unique_ptr<CaseClause> defaultCase;
    
    SwitchStatement(std::unique_ptr<Expression> cond, int ln = 0, int col = 0)
        : Statement(ln, col), condition(std::move(cond)), defaultCase(nullptr) {}
    
    void accept(ASTVisitor* visitor) override;
    std::string toString() const override;
};

// Program (root)
class Program : public ASTNode {
public:
    std::vector<std::unique_ptr<Statement>> preSwitchStatements;
    std::unique_ptr<SwitchStatement> switchStmt;

    Program(std::vector<std::unique_ptr<Statement>> prelude, std::unique_ptr<SwitchStatement> stmt)
        : preSwitchStatements(std::move(prelude)), switchStmt(std::move(stmt)) {}
    
    void accept(ASTVisitor* visitor) override;
    std::string toString() const override;
};

// Visitor pattern for AST traversal
class ASTVisitor {
public:
    virtual ~ASTVisitor() = default;
    
    virtual void visit(Program* node) = 0;
    virtual void visit(SwitchStatement* node) = 0;
    virtual void visit(CaseClause* node) = 0;
    virtual void visit(AssignmentStatement* node) = 0;
    virtual void visit(DeclarationStatement* node) = 0;
    virtual void visit(CinStatement* node) = 0;
    virtual void visit(CoutStatement* node) = 0;
    virtual void visit(BinaryExpression* node) = 0;
    virtual void visit(Identifier* node) = 0;
    virtual void visit(Constant* node) = 0;
    virtual void visit(StringLiteral* node) = 0;
};

#endif // AST_H
