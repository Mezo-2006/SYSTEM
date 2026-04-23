#include "AST.h"
#include <sstream>

// BinaryExpression
void BinaryExpression::accept(ASTVisitor* visitor) {
    visitor->visit(this);
}

std::string BinaryExpression::toString() const {
    std::stringstream ss;
    ss << "(" << left->toString() << " " << op << " " << right->toString() << ")";
    return ss.str();
}

// Identifier
void Identifier::accept(ASTVisitor* visitor) {
    visitor->visit(this);
}

std::string Identifier::toString() const {
    return name;
}

// Constant
void Constant::accept(ASTVisitor* visitor) {
    visitor->visit(this);
}

std::string Constant::toString() const {
    return std::to_string(value);
}

// StringLiteral
void StringLiteral::accept(ASTVisitor* visitor) {
    visitor->visit(this);
}

std::string StringLiteral::toString() const {
    return "\"" + value + "\"";
}

// AssignmentStatement
void AssignmentStatement::accept(ASTVisitor* visitor) {
    visitor->visit(this);
}

std::string AssignmentStatement::toString() const {
    std::stringstream ss;
    ss << variableName << " = " << expression->toString() << ";";
    return ss.str();
}

// DeclarationStatement
void DeclarationStatement::accept(ASTVisitor* visitor) {
    visitor->visit(this);
}

std::string DeclarationStatement::toString() const {
    std::stringstream ss;
    ss << variableType << " " << variableName;
    if (initializer) {
        ss << " = " << initializer->toString();
    }
    ss << ";";
    return ss.str();
}

// CaseClause
// CinStatement
void CinStatement::accept(ASTVisitor* visitor) {
    visitor->visit(this);
}

std::string CinStatement::toString() const {
    return "cin >> " + variableName + ";";
}

// CoutStatement
void CoutStatement::accept(ASTVisitor* visitor) {
    visitor->visit(this);
}

std::string CoutStatement::toString() const {
    return "cout << " + expression->toString() + ";";
}

void CaseClause::accept(ASTVisitor* visitor) {
    visitor->visit(this);
}

std::string CaseClause::toString() const {
    std::stringstream ss;
    if (isDefault) {
        ss << "default: ";
    } else {
        ss << "case " << caseValue << ": ";
    }
    for (const auto& stmt : statements) {
        ss << stmt->toString() << " ";
    }
    ss << "break;";
    return ss.str();
}

// SwitchStatement
void SwitchStatement::accept(ASTVisitor* visitor) {
    visitor->visit(this);
}

std::string SwitchStatement::toString() const {
    std::stringstream ss;
    ss << "switch (" << condition->toString() << ") { ";
    for (const auto& caseClause : cases) {
        ss << caseClause->toString() << " ";
    }
    if (defaultCase) {
        ss << defaultCase->toString() << " ";
    }
    ss << "}";
    return ss.str();
}

// Program
void Program::accept(ASTVisitor* visitor) {
    visitor->visit(this);
}

std::string Program::toString() const {
    std::stringstream ss;
    for (const auto& stmt : preSwitchStatements) {
        ss << stmt->toString() << " ";
    }
    if (switchStmt) {
        ss << switchStmt->toString();
    }
    return ss.str();
}
