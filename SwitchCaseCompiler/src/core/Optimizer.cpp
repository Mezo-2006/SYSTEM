#include "Optimizer.h"
#include <algorithm>
#include <cctype>

Optimizer::Optimizer() {}

void Optimizer::setInstructions(const std::vector<TACInstruction>& code) {
    instructions = code;
    optimizationResults.clear();
}

bool Optimizer::isConstant(const std::string& str) {
    if (str.empty()) return false;
    
    size_t start = 0;
    if (str[0] == '-' || str[0] == '+') start = 1;
    
    for (size_t i = start; i < str.length(); i++) {
        if (!std::isdigit(str[i])) return false;
    }
    return true;
}

int Optimizer::evaluateConstant(const std::string& str) {
    return std::stoi(str);
}

std::string Optimizer::computeConstantExpression(TACOpcode op, 
                                                const std::string& arg1, 
                                                const std::string& arg2) {
    if (!isConstant(arg1) || !isConstant(arg2)) {
        return "";  // Cannot compute
    }
    
    int val1 = evaluateConstant(arg1);
    int val2 = evaluateConstant(arg2);
    int result = 0;
    
    switch (op) {
        case TACOpcode::ADD: result = val1 + val2; break;
        case TACOpcode::SUB: result = val1 - val2; break;
        case TACOpcode::MUL: result = val1 * val2; break;
        case TACOpcode::DIV: 
            if (val2 != 0) result = val1 / val2;
            else return "";  // Division by zero
            break;
        case TACOpcode::EQ: result = (val1 == val2) ? 1 : 0; break;
        case TACOpcode::NEQ: result = (val1 != val2) ? 1 : 0; break;
        case TACOpcode::LT: result = (val1 < val2) ? 1 : 0; break;
        case TACOpcode::GT: result = (val1 > val2) ? 1 : 0; break;
        default: return "";
    }
    
    return std::to_string(result);
}

std::vector<TACInstruction> Optimizer::constantFolding(const std::vector<TACInstruction>& code) {
    std::vector<TACInstruction> optimized;
    std::map<std::string, std::string> constants;  // var -> constant value
    
    for (const auto& inst : code) {
        TACInstruction newInst = inst;
        bool modified = false;
        
        switch (inst.opcode) {
            case TACOpcode::ASSIGN:
                // Check if assigning a constant
                if (isConstant(inst.arg1)) {
                    constants[inst.result] = inst.arg1;
                } else if (constants.find(inst.arg1) != constants.end()) {
                    // Replace variable with its constant value
                    newInst.arg1 = constants[inst.arg1];
                    constants[inst.result] = constants[inst.arg1];
                    modified = true;
                } else {
                    // Variable is no longer constant
                    constants.erase(inst.result);
                }
                break;
                
            case TACOpcode::ADD:
            case TACOpcode::SUB:
            case TACOpcode::MUL:
            case TACOpcode::DIV:
            case TACOpcode::EQ:
            case TACOpcode::NEQ:
            case TACOpcode::LT:
            case TACOpcode::GT: {
                // Replace operands with constants if available
                std::string op1 = inst.arg1;
                std::string op2 = inst.arg2;
                
                if (!isConstant(op1) && constants.find(op1) != constants.end()) {
                    op1 = constants[op1];
                    modified = true;
                }
                if (!isConstant(op2) && constants.find(op2) != constants.end()) {
                    op2 = constants[op2];
                    modified = true;
                }
                
                // Try to evaluate constant expression
                std::string result = computeConstantExpression(inst.opcode, op1, op2);
                if (!result.empty()) {
                    // Replace with assignment
                    newInst = TACInstruction(TACOpcode::ASSIGN, inst.result, result);
                    constants[inst.result] = result;
                    modified = true;
                } else {
                    newInst.arg1 = op1;
                    newInst.arg2 = op2;
                    constants.erase(inst.result);
                }
                break;
            }
                
            default:
                // Other instructions might invalidate constants
                break;
        }
        
        optimized.push_back(newInst);
    }
    
    return optimized;
}

std::set<std::string> Optimizer::findLiveVariables(const std::vector<TACInstruction>& code) {
    std::set<std::string> liveVars;
    
    // Simple approach: mark all variables that are used (not just defined)
    for (const auto& inst : code) {
        switch (inst.opcode) {
            case TACOpcode::ASSIGN:
                if (!isConstant(inst.arg1)) {
                    liveVars.insert(inst.arg1);
                }
                break;
                
            case TACOpcode::ADD:
            case TACOpcode::SUB:
            case TACOpcode::MUL:
            case TACOpcode::DIV:
            case TACOpcode::EQ:
            case TACOpcode::NEQ:
            case TACOpcode::LT:
            case TACOpcode::GT:
                if (!isConstant(inst.arg1)) liveVars.insert(inst.arg1);
                if (!isConstant(inst.arg2)) liveVars.insert(inst.arg2);
                break;
                
            case TACOpcode::IF_GOTO:
            case TACOpcode::IF_FALSE_GOTO:
                if (!isConstant(inst.arg1)) liveVars.insert(inst.arg1);
                break;
                
            default:
                break;
        }
    }
    
    return liveVars;
}

bool Optimizer::isDeadCode(const TACInstruction& inst, const std::set<std::string>& liveVars) {
    // Check if instruction defines a variable that is never used
    if (inst.opcode == TACOpcode::ASSIGN || 
        inst.opcode == TACOpcode::ADD ||
        inst.opcode == TACOpcode::SUB ||
        inst.opcode == TACOpcode::MUL ||
        inst.opcode == TACOpcode::DIV ||
        inst.opcode == TACOpcode::EQ ||
        inst.opcode == TACOpcode::NEQ ||
        inst.opcode == TACOpcode::LT ||
        inst.opcode == TACOpcode::GT) {
        
        // Temporary variables that are never used are dead
        if (!inst.result.empty() && inst.result[0] == 't' && liveVars.find(inst.result) == liveVars.end()) {
            return true;
        }
    }
    
    return false;
}

std::vector<TACInstruction> Optimizer::deadCodeElimination(const std::vector<TACInstruction>& code) {
    std::vector<TACInstruction> optimized;
    std::set<std::string> liveVars = findLiveVariables(code);
    
    bool afterGoto = false;
    
    for (const auto& inst : code) {
        // Remove unreachable code after unconditional goto (until next label)
        if (inst.opcode == TACOpcode::GOTO) {
            optimized.push_back(inst);
            afterGoto = true;
            continue;
        }
        
        if (inst.opcode == TACOpcode::LABEL) {
            afterGoto = false;
            optimized.push_back(inst);
            continue;
        }
        
        if (afterGoto) {
            // Skip this instruction (unreachable code)
            continue;
        }
        
        // Check if instruction is dead (unused result)
        if (!isDeadCode(inst, liveVars)) {
            optimized.push_back(inst);
        }
    }
    
    return optimized;
}

std::string Optimizer::expressionKey(const TACInstruction& inst) {
    // Create a unique key for this expression
    return inst.opcodeToString() + ":" + inst.arg1 + ":" + inst.arg2;
}

std::vector<TACInstruction> Optimizer::commonSubexpressionElimination(
    const std::vector<TACInstruction>& code) {
    
    std::vector<TACInstruction> optimized;
    std::map<std::string, std::string> expressions;  // expression key -> result variable
    
    for (const auto& inst : code) {
        TACInstruction newInst = inst;
        bool replaced = false;
        
        switch (inst.opcode) {
            case TACOpcode::ADD:
            case TACOpcode::SUB:
            case TACOpcode::MUL:
            case TACOpcode::DIV: {
                std::string key = expressionKey(inst);
                
                if (expressions.find(key) != expressions.end()) {
                    // Found common subexpression, replace with assignment
                    newInst = TACInstruction(TACOpcode::ASSIGN, inst.result, expressions[key]);
                    replaced = true;
                } else {
                    // Record this expression
                    expressions[key] = inst.result;
                }
                break;
            }
                
            case TACOpcode::ASSIGN:
                // Assignment may invalidate expressions
                for (auto it = expressions.begin(); it != expressions.end(); ) {
                    if (it->second == inst.arg1) {
                        // The variable being assigned is used in an expression
                        // We should be more conservative here, but for simplicity:
                        it = expressions.erase(it);
                    } else {
                        ++it;
                    }
                }
                break;
                
            case TACOpcode::LABEL:
                // Clear expression map at labels (conservative)
                expressions.clear();
                break;
                
            default:
                break;
        }
        
        optimized.push_back(newInst);
    }
    
    return optimized;
}

void Optimizer::optimizeConstantFolding(bool enabled) {
    if (enabled) {
        OptimizationResult result("Constant Folding");
        result.before = instructions;
        instructions = constantFolding(instructions);
        result.after = instructions;
        result.removedInstructions = result.before.size() - result.after.size();
        optimizationResults.push_back(result);
    }
}

void Optimizer::optimizeDeadCode(bool enabled) {
    if (enabled) {
        OptimizationResult result("Dead Code Elimination");
        result.before = instructions;
        instructions = deadCodeElimination(instructions);
        result.after = instructions;
        result.removedInstructions = result.before.size() - result.after.size();
        optimizationResults.push_back(result);
    }
}

void Optimizer::optimizeCSE(bool enabled) {
    if (enabled) {
        OptimizationResult result("Common Subexpression Elimination");
        result.before = instructions;
        instructions = commonSubexpressionElimination(instructions);
        result.after = instructions;
        result.removedInstructions = result.before.size() - result.after.size();
        optimizationResults.push_back(result);
    }
}

std::vector<TACInstruction> Optimizer::optimize(bool constFold, bool deadCode, bool cse) {
    optimizationResults.clear();
    
    optimizeConstantFolding(constFold);
    optimizeDeadCode(deadCode);
    optimizeCSE(cse);
    
    return instructions;
}
