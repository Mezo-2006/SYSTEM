#include "Optimizer.h"
#include <algorithm>
#include <cctype>
#include <cmath>

Optimizer::Optimizer() {}

void Optimizer::setInstructions(const std::vector<TACInstruction>& code) {
    instructions = code;
    originalInstructions = code;  // Preserve raw unoptimized TAC for educational display
    optimizationResults.clear();
}

bool Optimizer::isConstant(const std::string& str) {
    if (str.empty()) return false;
    
    size_t start = 0;
    if (str[0] == '-' || str[0] == '+') start = 1;
    if (start >= str.length()) return false;
    
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
            else return "";  // Division by zero — leave untouched
            break;
        case TACOpcode::EQ: result = (val1 == val2) ? 1 : 0; break;
        case TACOpcode::NEQ: result = (val1 != val2) ? 1 : 0; break;
        case TACOpcode::LT: result = (val1 < val2) ? 1 : 0; break;
        case TACOpcode::GT: result = (val1 > val2) ? 1 : 0; break;
        default: return "";
    }
    
    return std::to_string(result);
}

// ── Constant Folding ──────────────────────────────────────────────────────────
// Evaluates compile-time constant expressions and propagates constant values.
std::vector<TACInstruction> Optimizer::constantFolding(const std::vector<TACInstruction>& code) {
    std::vector<TACInstruction> optimized;
    std::map<std::string, std::string> constants;  // var -> constant value
    
    for (const auto& inst : code) {
        TACInstruction newInst = inst;
        
        switch (inst.opcode) {
            case TACOpcode::ASSIGN:
                // Check if assigning a constant
                if (isConstant(inst.arg1)) {
                    constants[inst.result] = inst.arg1;
                } else if (constants.find(inst.arg1) != constants.end()) {
                    // Replace variable with its constant value
                    newInst.arg1 = constants[inst.arg1];
                    constants[inst.result] = constants[inst.arg1];
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
                }
                if (!isConstant(op2) && constants.find(op2) != constants.end()) {
                    op2 = constants[op2];
                }
                
                // Try to evaluate constant expression
                std::string result = computeConstantExpression(inst.opcode, op1, op2);
                if (!result.empty()) {
                    // Replace with assignment
                    newInst = TACInstruction(TACOpcode::ASSIGN, inst.result, result);
                    constants[inst.result] = result;
                } else {
                    newInst.arg1 = op1;
                    newInst.arg2 = op2;
                    constants.erase(inst.result);
                }
                break;
            }
            
            case TACOpcode::LABEL:
            case TACOpcode::GOTO:
                // Control flow boundaries — invalidate all constants (conservative)
                constants.clear();
                break;
                
            case TACOpcode::IF_GOTO:
            case TACOpcode::IF_FALSE_GOTO:
                // Conditional branches don't invalidate, but substitute arg1
                if (!inst.arg1.empty() && constants.find(inst.arg1) != constants.end()) {
                    newInst.arg1 = constants[inst.arg1];
                }
                break;
                
            case TACOpcode::COUT:
                // Substitute in output if constant
                if (!inst.result.empty() && constants.find(inst.result) != constants.end()) {
                    newInst.result = constants[inst.result];
                }
                break;
                
            default:
                break;
        }
        
        optimized.push_back(newInst);
    }
    
    return optimized;
}

// ── Dead Code Elimination ─────────────────────────────────────────────────────
// Removes assignments to variables/temporaries that are never used, and
// unreachable code after unconditional GOTOs.

std::set<std::string> Optimizer::findLiveVariables(const std::vector<TACInstruction>& code) {
    std::set<std::string> liveVars;
    
    // Mark all variables that are USED as operands (not just defined)
    for (const auto& inst : code) {
        switch (inst.opcode) {
            case TACOpcode::ASSIGN:
                if (!inst.arg1.empty() && !isConstant(inst.arg1)) {
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
                if (!inst.arg1.empty() && !isConstant(inst.arg1)) liveVars.insert(inst.arg1);
                if (!inst.arg2.empty() && !isConstant(inst.arg2)) liveVars.insert(inst.arg2);
                break;
                
            case TACOpcode::IF_GOTO:
            case TACOpcode::IF_FALSE_GOTO:
                if (!inst.arg1.empty() && !isConstant(inst.arg1)) liveVars.insert(inst.arg1);
                break;
                
            case TACOpcode::COUT:
                if (!inst.result.empty() && !isConstant(inst.result)) liveVars.insert(inst.result);
                break;
                
            case TACOpcode::RETURN:
                if (!inst.result.empty() && !isConstant(inst.result)) liveVars.insert(inst.result);
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
        
        // Temporary variables (t0, t1, ...) that are never used are dead
        if (!inst.result.empty() && inst.result[0] == 't' && 
            liveVars.find(inst.result) == liveVars.end()) {
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

// ── Common Subexpression Elimination ──────────────────────────────────────────
// Identifies identical expressions and replaces duplicates with the result
// of the first computation. Handles commutative operators (a+b == b+a).

std::string Optimizer::expressionKey(const TACInstruction& inst) {
    // For commutative operations, normalize operand order
    bool isCommutative = (inst.opcode == TACOpcode::ADD || inst.opcode == TACOpcode::MUL);
    
    std::string a1 = inst.arg1;
    std::string a2 = inst.arg2;
    if (isCommutative && a1 > a2) {
        std::swap(a1, a2);
    }
    
    return inst.opcodeToString() + ":" + a1 + ":" + a2;
}

std::vector<TACInstruction> Optimizer::commonSubexpressionElimination(
    const std::vector<TACInstruction>& code) {
    
    std::vector<TACInstruction> optimized;
    std::map<std::string, std::string> expressions;  // expression key -> result variable
    
    for (const auto& inst : code) {
        TACInstruction newInst = inst;
        
        switch (inst.opcode) {
            case TACOpcode::ADD:
            case TACOpcode::SUB:
            case TACOpcode::MUL:
            case TACOpcode::DIV: {
                std::string key = expressionKey(inst);
                
                if (expressions.find(key) != expressions.end()) {
                    // Found common subexpression, replace with assignment
                    newInst = TACInstruction(TACOpcode::ASSIGN, inst.result, expressions[key]);
                } else {
                    // Record this expression
                    expressions[key] = inst.result;
                }
                break;
            }
                
            case TACOpcode::ASSIGN:
                // Assignment invalidates expressions that reference the result variable
                // as an operand (since its value is changing)
                for (auto it = expressions.begin(); it != expressions.end(); ) {
                    const std::string& key = it->first;
                    // Check if the assigned variable appears in the expression key
                    // Key format: "OPCODE:arg1:arg2"
                    if (key.find(":" + inst.result + ":") != std::string::npos ||
                        key.rfind(":" + inst.result) == key.length() - inst.result.length() - 1) {
                        it = expressions.erase(it);
                    } else {
                        ++it;
                    }
                }
                break;
                
            case TACOpcode::LABEL:
            case TACOpcode::GOTO:
                // Clear expression map at control flow boundaries (conservative)
                expressions.clear();
                break;
                
            default:
                break;
        }
        
        optimized.push_back(newInst);
    }
    
    return optimized;
}

// ── Algebraic Simplification ──────────────────────────────────────────────────
// Applies algebraic identities to simplify instructions.

std::vector<TACInstruction> Optimizer::algebraicSimplification(const std::vector<TACInstruction>& code) {
    std::vector<TACInstruction> optimized;
    
    for (const auto& inst : code) {
        TACInstruction newInst = inst;
        
        switch (inst.opcode) {
            case TACOpcode::ADD:
                // x + 0 = x
                if (inst.arg2 == "0") {
                    newInst = TACInstruction(TACOpcode::ASSIGN, inst.result, inst.arg1);
                }
                // 0 + x = x
                else if (inst.arg1 == "0") {
                    newInst = TACInstruction(TACOpcode::ASSIGN, inst.result, inst.arg2);
                }
                break;
                
            case TACOpcode::SUB:
                // x - 0 = x
                if (inst.arg2 == "0") {
                    newInst = TACInstruction(TACOpcode::ASSIGN, inst.result, inst.arg1);
                }
                // x - x = 0
                else if (inst.arg1 == inst.arg2) {
                    newInst = TACInstruction(TACOpcode::ASSIGN, inst.result, "0");
                }
                break;
                
            case TACOpcode::MUL:
                // x * 0 = 0, 0 * x = 0
                if (inst.arg1 == "0" || inst.arg2 == "0") {
                    newInst = TACInstruction(TACOpcode::ASSIGN, inst.result, "0");
                }
                // x * 1 = x
                else if (inst.arg2 == "1") {
                    newInst = TACInstruction(TACOpcode::ASSIGN, inst.result, inst.arg1);
                }
                // 1 * x = x
                else if (inst.arg1 == "1") {
                    newInst = TACInstruction(TACOpcode::ASSIGN, inst.result, inst.arg2);
                }
                break;
                
            case TACOpcode::DIV:
                // x / 1 = x
                if (inst.arg2 == "1") {
                    newInst = TACInstruction(TACOpcode::ASSIGN, inst.result, inst.arg1);
                }
                // x / x = 1 (assuming x != 0)
                else if (inst.arg1 == inst.arg2 && inst.arg1 != "0") {
                    newInst = TACInstruction(TACOpcode::ASSIGN, inst.result, "1");
                }
                // 0 / x = 0 (assuming x != 0)
                else if (inst.arg1 == "0" && inst.arg2 != "0") {
                    newInst = TACInstruction(TACOpcode::ASSIGN, inst.result, "0");
                }
                break;
                
            default:
                break;
        }
        
        optimized.push_back(newInst);
    }
    
    return optimized;
}

// ── Copy Propagation ──────────────────────────────────────────────────────────
// When x = y, replaces subsequent uses of x with y (until x is reassigned).

std::vector<TACInstruction> Optimizer::copyPropagation(const std::vector<TACInstruction>& code) {
    std::vector<TACInstruction> optimized;
    std::map<std::string, std::string> copies; // var -> copy source
    
    // Helper to resolve through copy chains: if a=b and b=c, resolve a to c
    auto resolve = [&copies](const std::string& var) -> std::string {
        std::string current = var;
        int depth = 0;
        while (copies.find(current) != copies.end() && depth < 10) {
            current = copies[current];
            depth++;
        }
        return current;
    };
    
    for (const auto& inst : code) {
        TACInstruction newInst = inst;
        
        // Replace uses with propagated values
        if (!inst.arg1.empty() && !isConstant(inst.arg1) && copies.find(inst.arg1) != copies.end()) {
            newInst.arg1 = resolve(inst.arg1);
        }
        if (!inst.arg2.empty() && !isConstant(inst.arg2) && copies.find(inst.arg2) != copies.end()) {
            newInst.arg2 = resolve(inst.arg2);
        }
        
        // Track copy assignments: x = y (where y is not a constant)
        if (inst.opcode == TACOpcode::ASSIGN && !isConstant(inst.arg1)) {
            copies[inst.result] = inst.arg1;
        } else if (inst.opcode != TACOpcode::LABEL && inst.opcode != TACOpcode::GOTO &&
                   inst.opcode != TACOpcode::IF_GOTO && inst.opcode != TACOpcode::IF_FALSE_GOTO &&
                   inst.opcode != TACOpcode::COUT && inst.opcode != TACOpcode::CIN) {
            // Non-copy assignment to result invalidates any copy mapping for result
            if (!inst.result.empty()) {
                copies.erase(inst.result);
                // Also invalidate any copies that reference this variable as source
                for (auto it = copies.begin(); it != copies.end(); ) {
                    if (it->second == inst.result) {
                        it = copies.erase(it);
                    } else {
                        ++it;
                    }
                }
            }
        }
        
        // Labels/GOTOs clear the copy map (conservative at control flow boundaries)
        if (inst.opcode == TACOpcode::LABEL) {
            copies.clear();
        }
        
        optimized.push_back(newInst);
    }
    
    return optimized;
}

// ── Strength Reduction ────────────────────────────────────────────────────────
// Replaces expensive operations with cheaper equivalents:
//   x * 2 → x + x
//   x * 4 → x << 2  (represented as shift, but we emit ADD chain for simplicity)
//   x / 2 → x >> 1  (not implemented — integer division semantics differ)

std::vector<TACInstruction> Optimizer::strengthReduction(const std::vector<TACInstruction>& code) {
    std::vector<TACInstruction> optimized;
    
    for (const auto& inst : code) {
        TACInstruction newInst = inst;
        
        switch (inst.opcode) {
            case TACOpcode::MUL:
                // x * 2 = x + x
                if (inst.arg2 == "2") {
                    newInst = TACInstruction(TACOpcode::ADD, inst.result, inst.arg1, inst.arg1);
                } else if (inst.arg1 == "2") {
                    newInst = TACInstruction(TACOpcode::ADD, inst.result, inst.arg2, inst.arg2);
                }
                // x * 4 = (x + x) + (x + x) — not worth the extra instruction, skip
                break;
                
            default:
                break;
        }
        
        optimized.push_back(newInst);
    }
    
    return optimized;
}

// ── Pass wrappers (with result tracking) ──────────────────────────────────────

void Optimizer::optimizeConstantFolding(bool enabled) {
    if (!enabled) return;
    
    OptimizationResult result("Constant Folding");
    result.before = originalInstructions;  // Always show raw unoptimized TAC
    instructions = constantFolding(instructions);
    result.after = instructions;
    result.removedInstructions = static_cast<int>(result.before.size() - result.after.size());
    result.modifiedInstructions = 0;
    for (size_t i = 0; i < std::min(result.before.size(), result.after.size()); ++i) {
        if (result.before[i].toString() != result.after[i].toString()) {
            result.modifiedInstructions++;
        }
    }
    optimizationResults.push_back(result);
}

void Optimizer::optimizeDeadCode(bool enabled) {
    if (!enabled) return;
    
    OptimizationResult result("Dead Code Elimination");
    result.before = originalInstructions;  // Always show raw unoptimized TAC
    instructions = deadCodeElimination(instructions);
    result.after = instructions;
    result.removedInstructions = static_cast<int>(result.before.size() - result.after.size());
    result.modifiedInstructions = 0;
    for (size_t i = 0; i < std::min(result.before.size(), result.after.size()); ++i) {
        if (result.before[i].toString() != result.after[i].toString()) {
            result.modifiedInstructions++;
        }
    }
    optimizationResults.push_back(result);
}

void Optimizer::optimizeCSE(bool enabled) {
    if (!enabled) return;
    
    OptimizationResult result("Common Subexpression Elimination");
    result.before = originalInstructions;  // Always show raw unoptimized TAC
    instructions = commonSubexpressionElimination(instructions);
    result.after = instructions;
    result.removedInstructions = static_cast<int>(result.before.size() - result.after.size());
    result.modifiedInstructions = 0;
    for (size_t i = 0; i < std::min(result.before.size(), result.after.size()); ++i) {
        if (result.before[i].toString() != result.after[i].toString()) {
            result.modifiedInstructions++;
        }
    }
    optimizationResults.push_back(result);
}

void Optimizer::optimizeAlgebraicSimplification(bool enabled) {
    if (!enabled) return;
    
    OptimizationResult result("Algebraic Simplification");
    result.before = originalInstructions;  // Always show raw unoptimized TAC
    instructions = algebraicSimplification(instructions);
    result.after = instructions;
    result.removedInstructions = static_cast<int>(result.before.size() - result.after.size());
    result.modifiedInstructions = 0;
    for (size_t i = 0; i < std::min(result.before.size(), result.after.size()); ++i) {
        if (result.before[i].toString() != result.after[i].toString()) {
            result.modifiedInstructions++;
        }
    }
    optimizationResults.push_back(result);
}

void Optimizer::optimizeCopyPropagation(bool enabled) {
    if (!enabled) return;
    
    OptimizationResult result("Copy Propagation");
    result.before = originalInstructions;  // Always show raw unoptimized TAC
    instructions = copyPropagation(instructions);
    result.after = instructions;
    result.removedInstructions = static_cast<int>(result.before.size() - result.after.size());
    result.modifiedInstructions = 0;
    for (size_t i = 0; i < std::min(result.before.size(), result.after.size()); ++i) {
        if (result.before[i].toString() != result.after[i].toString()) {
            result.modifiedInstructions++;
        }
    }
    optimizationResults.push_back(result);
}

void Optimizer::optimizeStrengthReduction(bool enabled) {
    if (!enabled) return;
    
    OptimizationResult result("Strength Reduction");
    result.before = originalInstructions;  // Always show raw unoptimized TAC
    instructions = strengthReduction(instructions);
    result.after = instructions;
    result.removedInstructions = static_cast<int>(result.before.size() - result.after.size());
    result.modifiedInstructions = 0;
    for (size_t i = 0; i < std::min(result.before.size(), result.after.size()); ++i) {
        if (result.before[i].toString() != result.after[i].toString()) {
            result.modifiedInstructions++;
        }
    }
    optimizationResults.push_back(result);
}

// ── Main optimization driver ──────────────────────────────────────────────────

std::vector<TACInstruction> Optimizer::optimize(bool constFold, bool deadCode, bool cse, 
                                               bool algebraic, bool copyProp, bool strengthRed) {
    optimizationResults.clear();
    originalInstructions = instructions;  // Snapshot the raw unoptimized TAC
    
    optimizeConstantFolding(constFold);
    optimizeAlgebraicSimplification(algebraic);
    optimizeCopyPropagation(copyProp);
    optimizeDeadCode(deadCode);
    optimizeCSE(cse);
    optimizeStrengthReduction(strengthRed);
    
    return instructions;
}