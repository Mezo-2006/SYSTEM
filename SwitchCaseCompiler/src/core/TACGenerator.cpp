#include "TACGenerator.h"
#include <sstream>
#include <functional>

// TACInstruction implementation
std::string TACInstruction::opcodeToString() const {
    switch (opcode) {
        case TACOpcode::ASSIGN: return "=";
        case TACOpcode::ADD: return "+";
        case TACOpcode::SUB: return "-";
        case TACOpcode::MUL: return "*";
        case TACOpcode::DIV: return "/";
        case TACOpcode::EQ: return "==";
        case TACOpcode::NEQ: return "!=";
        case TACOpcode::LT: return "<";
        case TACOpcode::GT: return ">";
        case TACOpcode::LABEL: return "LABEL";
        case TACOpcode::GOTO: return "goto";
        case TACOpcode::IF_GOTO: return "if_goto";
        case TACOpcode::IF_FALSE_GOTO: return "if_false_goto";
        case TACOpcode::PARAM: return "param";
        case TACOpcode::CALL: return "call";
        case TACOpcode::RETURN: return "return";
        default: return "UNKNOWN";
    }
}

std::string TACInstruction::toString() const {
    std::stringstream ss;
    
    switch (opcode) {
        case TACOpcode::LABEL:
            ss << result << ":";
            break;
            
        case TACOpcode::GOTO:
            ss << "goto " << result;
            break;
            
        case TACOpcode::IF_GOTO:
            ss << "if " << arg1 << " goto " << result;
            break;
            
        case TACOpcode::IF_FALSE_GOTO:
            ss << "ifFalse " << arg1 << " goto " << result;
            break;
            
        case TACOpcode::ASSIGN:
            ss << result << " = " << arg1;
            break;
            
        case TACOpcode::ADD:
        case TACOpcode::SUB:
        case TACOpcode::MUL:
        case TACOpcode::DIV:
        case TACOpcode::EQ:
        case TACOpcode::NEQ:
        case TACOpcode::LT:
        case TACOpcode::GT:
            ss << result << " = " << arg1 << " " << opcodeToString() << " " << arg2;
            break;
            
        case TACOpcode::RETURN:
            ss << "return " << result;
            break;
            
        default:
            ss << "UNKNOWN INSTRUCTION";
    }
    
    return ss.str();
}

// TACGenerator implementation
TACGenerator::TACGenerator() : tempCounter(0), labelCounter(0), lastResult("") {}

void TACGenerator::reset() {
    instructions.clear();
    tempCounter = 0;
    labelCounter = 0;
    lastResult = "";
}

std::string TACGenerator::newTemp() {
    return "t" + std::to_string(tempCounter++);
}

std::string TACGenerator::newLabel() {
    return "L" + std::to_string(labelCounter++);
}

void TACGenerator::emitInstruction(TACOpcode op, const std::string& result, 
                                   const std::string& arg1, const std::string& arg2) {
    instructions.push_back(TACInstruction(op, result, arg1, arg2));
}

void TACGenerator::emitLabel(const std::string& label) {
    emitInstruction(TACOpcode::LABEL, label);
}

void TACGenerator::emitGoto(const std::string& label) {
    emitInstruction(TACOpcode::GOTO, label);
}

void TACGenerator::emitIfGoto(const std::string& condition, const std::string& label) {
    emitInstruction(TACOpcode::IF_GOTO, label, condition);
}

std::vector<TACInstruction> TACGenerator::generate(Program* program) {
    reset();
    
    if (program) {
        program->accept(this);
    }
    
    return instructions;
}

void TACGenerator::visit(Program* node) {
    for (auto& stmt : node->preSwitchStatements) {
        stmt->accept(this);
    }

    if (node->switchStmt) {
        node->switchStmt->accept(this);
    }
}

void TACGenerator::visit(SwitchStatement* node) {
    // Generate code for switch condition
    if (node->condition) {
        node->condition->accept(this);
    } else {
        lastResult = "0";  // fallback — no condition
    }
    std::string switchVar = lastResult.empty() ? "0" : lastResult;
    
    std::string endLabel = newLabel();
    std::vector<std::string> caseLabels;
    std::string defaultLabel;
    
    // Generate labels for each case
    for (size_t i = 0; i < node->cases.size(); i++) {
        caseLabels.push_back(newLabel());
    }
    
    if (node->defaultCase) {
        defaultLabel = newLabel();
    }
    
    // Generate comparison and jumps for each case
    for (size_t i = 0; i < node->cases.size(); i++) {
        std::string caseValue = std::to_string(node->cases[i]->caseValue);
        std::string tempCond = newTemp();
        
        // t1 = switchVar == caseValue
        emitInstruction(TACOpcode::EQ, tempCond, switchVar, caseValue);
        
        // if t1 goto L_case_i
        emitIfGoto(tempCond, caseLabels[i]);
    }
    
    // If no case matches, goto default (or end if no default)
    if (node->defaultCase) {
        emitGoto(defaultLabel);
    } else {
        emitGoto(endLabel);
    }
    
    // Generate code for each case
    for (size_t i = 0; i < node->cases.size(); i++) {
        emitLabel(caseLabels[i]);
        node->cases[i]->accept(this);
        // Case statements include break, which generates goto end
        emitGoto(endLabel);
    }
    
    // Generate code for default case
    if (node->defaultCase) {
        emitLabel(defaultLabel);
        node->defaultCase->accept(this);
        emitGoto(endLabel);
    }
    
    // End label
    emitLabel(endLabel);
}

void TACGenerator::visit(CaseClause* node) {
    // Generate code for statements in the case
    for (auto& stmt : node->statements) {
        stmt->accept(this);
    }
    // Break is implicit (handled by switch statement with goto end_label)
}

void TACGenerator::visit(AssignmentStatement* node) {
    // Generate code for the expression
    if (node->expression) {
        node->expression->accept(this);
    }
    
    // Assign the result to the variable
    emitInstruction(TACOpcode::ASSIGN, node->variableName, lastResult);
}

void TACGenerator::visit(DeclarationStatement* node) {
    if (node->initializer) {
        node->initializer->accept(this);
        emitInstruction(TACOpcode::ASSIGN, node->variableName, lastResult);
    }
}

void TACGenerator::visit(BinaryExpression* node) {
    // Stream operator << is handled specially — just emit COUT for each operand
    if (node->op == "<<") {
        // For cout chains, we just visit both sides and emit COUT for the right side
        // The left side is either another << chain or the first operand
        if (node->left) node->left->accept(this);
        if (node->right) node->right->accept(this);
        // lastResult is now the rightmost operand — that's what gets printed
        return;
    }

    // Generate code for left operand
    if (node->left) {
        node->left->accept(this);
    }
    std::string leftResult = lastResult;

    // Generate code for right operand
    if (node->right) {
        node->right->accept(this);
    }
    std::string rightResult = lastResult;

    // Generate operation
    std::string temp = newTemp();

    TACOpcode opcode;
    if (node->op == "+") opcode = TACOpcode::ADD;
    else if (node->op == "-") opcode = TACOpcode::SUB;
    else if (node->op == "*") opcode = TACOpcode::MUL;
    else if (node->op == "/") opcode = TACOpcode::DIV;
    else opcode = TACOpcode::ADD;  // Default

    emitInstruction(opcode, temp, leftResult, rightResult);
    lastResult = temp;
}

void TACGenerator::visit(Identifier* node) {
    lastResult = node->name;
}

void TACGenerator::visit(Constant* node) {
    lastResult = std::to_string(node->value);
}

void TACGenerator::visit(StringLiteral* node) {
    lastResult = "\"" + node->value + "\"";
}

void TACGenerator::visit(CinStatement* node) {
    emitInstruction(TACOpcode::CIN, node->variableName);
}

void TACGenerator::visit(CoutStatement* node) {
    if (!node->expression) return;

    // For chained << expressions, emit a COUT for each leaf operand
    // We do a recursive flatten: collect all leaf expressions from the << tree
    std::function<void(Expression*)> emitCout = [&](Expression* expr) {
        if (!expr) return;
        auto* bin = dynamic_cast<BinaryExpression*>(expr);
        if (bin && bin->op == "<<") {
            emitCout(bin->left.get());
            emitCout(bin->right.get());
        } else {
            expr->accept(this);
            emitInstruction(TACOpcode::COUT, lastResult);
        }
    };

    emitCout(node->expression.get());
}
