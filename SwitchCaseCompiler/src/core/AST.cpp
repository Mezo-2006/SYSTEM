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

// DeclRefExpr
void DeclRefExpr::accept(ASTVisitor* visitor) {
    visitor->visit(this);
}

std::string DeclRefExpr::toString() const {
    return name + (type.empty() ? "" : " : " + type);
}

// Identifier (legacy)
void Identifier::accept(ASTVisitor* visitor) {
    visitor->visit(this);
}

std::string Identifier::toString() const {
    return name;
}

// IntegerLiteral
void IntegerLiteral::accept(ASTVisitor* visitor) {
    visitor->visit(this);
}

std::string IntegerLiteral::toString() const {
    return std::to_string(value);
}

// Constant (legacy)
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

// CompoundStmt
void CompoundStmt::accept(ASTVisitor* visitor) {
    visitor->visit(this);
}

std::string CompoundStmt::toString() const {
    std::stringstream ss;
    ss << "{ ";
    for (const auto& stmt : statements) {
        ss << stmt->toString() << " ";
    }
    ss << "}";
    return ss.str();
}

// VarDecl
void VarDecl::accept(ASTVisitor* visitor) {
    visitor->visit(this);
}

std::string VarDecl::toString() const {
    std::stringstream ss;
    ss << type << " " << name;
    if (initializer) {
        ss << " = " << initializer->toString();
    }
    return ss.str();
}

// DeclStmt
void DeclStmt::accept(ASTVisitor* visitor) {
    visitor->visit(this);
}

std::string DeclStmt::toString() const {
    return declaration->toString() + ";";
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

// DeclarationStatement (legacy)
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

// BreakStmt
void BreakStmt::accept(ASTVisitor* visitor) {
    visitor->visit(this);
}

std::string BreakStmt::toString() const {
    return "break;";
}

// ReturnStmt
void ReturnStmt::accept(ASTVisitor* visitor) {
    visitor->visit(this);
}

std::string ReturnStmt::toString() const {
    if (returnValue) {
        return "return " + returnValue->toString() + ";";
    }
    return "return;";
}

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

// CaseStmt
void CaseStmt::accept(ASTVisitor* visitor) {
    visitor->visit(this);
}

std::string CaseStmt::toString() const {
    std::stringstream ss;
    if (isDefault) {
        ss << "default: ";
    } else {
        ss << "case " << caseValue << ": ";
    }
    for (const auto& stmt : statements) {
        ss << stmt->toString() << " ";
    }
    if (hasBreak) ss << "break;";
    return ss.str();
}

// CaseClause (legacy)
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
    ss << "switch (" << condition->toString() << ") ";
    if (body) {
        ss << body->toString();
    } else {
        // Legacy format
        ss << "{ ";
        for (const auto& caseClause : cases) {
            ss << caseClause->toString() << " ";
        }
        if (defaultCase) {
            ss << defaultCase->toString() << " ";
        }
        ss << "}";
    }
    return ss.str();
}

// FunctionDecl
void FunctionDecl::accept(ASTVisitor* visitor) {
    visitor->visit(this);
}

std::string FunctionDecl::toString() const {
    std::stringstream ss;
    ss << returnType << " " << name << "() ";
    if (body) {
        ss << body->toString();
    }
    return ss.str();
}

// TranslationUnit
void TranslationUnit::accept(ASTVisitor* visitor) {
    visitor->visit(this);
}

std::string TranslationUnit::toString() const {
    if (mainFunction) {
        return mainFunction->toString();
    }
    return "TranslationUnit";
}

// Program (legacy)
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
