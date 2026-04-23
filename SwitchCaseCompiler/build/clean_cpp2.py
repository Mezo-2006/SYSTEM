import os
import re

def fix_sema():
    path = r"c:\Users\Mizo\OneDrive - Arab Academy for Science and Technology\Desktop\systems 2\SwitchCaseCompiler\src\core\SemanticAnalyzer.cpp"
    with open(path, "r", encoding="utf-8") as f:
        content = f.read()
    
    # Remove bad insertions
    to_remove = """
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
    content = content.replace(to_remove, "")
    
    # insert correctly at END of file
    content += """
void SemanticAnalyzer::visit(CinStatement* node) {
    if (!symbolTable.isDeclared(node->variableName)) {
        addError("Variable '" + node->variableName + "' is used before it is declared", node->line, node->column);
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

fix_sema()
print("Fixed")
