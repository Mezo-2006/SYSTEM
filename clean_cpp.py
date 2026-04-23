import os
import re

def fix_sema():
    path = r"c:\Users\Mizo\OneDrive - Arab Academy for Science and Technology\Desktop\systems 2\SwitchCaseCompiler\src\core\SemanticAnalyzer.cpp"
    with open(path, "r", encoding="utf-8") as f:
        content = f.read()
    
    # Remove the bad insertions
    to_remove = """
void SemanticAnalyzer::visit(CinStatement* node) {
    if (!symbolTable.isDeclared(node->variableName)) {
        addError("Variable '" + node->variableName + "' is used before it is declared", node->line, node->column);
    } else if (!symbolTable.isInitialized(node->variableName)) {
        symbolTable.markInitialized(node->variableName);
    }
}

void SemanticAnalyzer::visit(CoutStatement* node) {
    if (node->expression) {
        node->expression->accept(this);
    }
}
"""
    content = content.replace(to_remove, "")
    
    # insert correctly at END of file
    content += """
void SemanticAnalyzer::visit(CinStatement* node) {
    if (!symbolTable.isDeclared(node->variableName)) {
        addError("Variable '" + node->variableName + "' is used before it is declared", node->annotatedType);
    }
}

void SemanticAnalyzer::visit(CoutStatement* node) {
    if (node->expression) {
        node->expression->accept(this);
    }
}
"""
    with open(path, "w", encoding="utf-8") as f:
        f.write(content)

def fix_tac():
    path = r"c:\Users\Mizo\OneDrive - Arab Academy for Science and Technology\Desktop\systems 2\SwitchCaseCompiler\src\core\TACGenerator.cpp"
    with open(path, "r", encoding="utf-8") as f:
        content = f.read()

    to_remove = """
void TACGenerator::visit(CinStatement* node) {
    emit(TACOpcode::CIN, node->variableName);
}

void TACGenerator::visit(CoutStatement* node) {
    node->expression->accept(this);
    emit(TACOpcode::COUT, currentTemp);
}
"""
    content = content.replace(to_remove, "")

    content += """
void TACGenerator::visit(CinStatement* node) {
    emitInstruction(TACOpcode::CIN, node->variableName);
}

void TACGenerator::visit(CoutStatement* node) {
    node->expression->accept(this);
    emitInstruction(TACOpcode::COUT, "%" + std::to_string(tempCounter - 1));
}
"""
    with open(path, "w", encoding="utf-8") as f:
        f.write(content)

fix_sema()
fix_tac()
print("Fixed")
