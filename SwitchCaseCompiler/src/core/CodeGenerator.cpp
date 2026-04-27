#include "CodeGenerator.h"
#include <sstream>
#include <algorithm>

// AssemblyInstruction implementation
std::string AssemblyInstruction::toString() const {
    std::stringstream ss;
    
    if (opcode == "LABEL") {
        ss << operand1 << ":";
    } else {
        ss << "    " << opcode;
        if (!operand1.empty()) {
            ss << " " << operand1;
            if (!operand2.empty()) {
                ss << ", " << operand2;
            }
        }
    }
    
    if (!comment.empty()) {
        ss << "  ; " << comment;
    }
    
    return ss.str();
}

// CodeGenerator implementation
CodeGenerator::CodeGenerator() {
    reset();
}

void CodeGenerator::reset() {
    assembly.clear();
    registerAllocation.clear();
    // Initialize standard x86 general purpose register pool
    registerPool = {"EAX", "EBX", "ECX", "EDX", "ESI", "EDI"};
}

std::string CodeGenerator::allocateRegister(const std::string& var) {
    if (registerAllocation.find(var) != registerAllocation.end()) {
        return registerAllocation[var];
    }
    
    std::string reg;
    if (!registerPool.empty()) {
        // Pop from the pool
        reg = registerPool.back();
        registerPool.pop_back();
    } else {
        // Fallback or "spill" scenario if we run out of registers.
        // In a real compiler, we would spill to stack. Here we just reuse a generic one 
        // to avoid crashing, but add a comment to indicate it.
        reg = "EAX"; 
        emitAsm("; WARNING", "", "", "Register spill fallback for " + var);
    }
    
    registerAllocation[var] = reg;
    return reg;
}

std::string CodeGenerator::getRegister(const std::string& var) {
    // Check if it's a constant
    if (!var.empty() && (std::isdigit(var[0]) || var[0] == '-')) {
        return var;  // Return constant as-is
    }
    
    if (registerAllocation.find(var) != registerAllocation.end()) {
        return registerAllocation[var];
    }
    
    return allocateRegister(var);
}

void CodeGenerator::freeRegister(const std::string& var) {
    auto it = registerAllocation.find(var);
    if (it != registerAllocation.end()) {
        std::string reg = it->second;
        // Don't put "spill" fallback back into the pool to avoid duplicates
        if (std::find(registerPool.begin(), registerPool.end(), reg) == registerPool.end()) {
            registerPool.push_back(reg);
        }
        registerAllocation.erase(it);
    }
}

void CodeGenerator::emitAsm(const std::string& opcode, const std::string& op1, 
                        const std::string& op2, const std::string& comment) {
    assembly.push_back(AssemblyInstruction(opcode, op1, op2, comment));
}

void CodeGenerator::generateInstruction(const TACInstruction& inst) {
    switch (inst.opcode) {
        case TACOpcode::LABEL:
            emitAsm("LABEL", inst.result);
            break;
            
        case TACOpcode::GOTO:
            emitAsm("JMP", inst.result);
            break;
            
        case TACOpcode::IF_GOTO: {
            std::string condReg = getRegister(inst.arg1);
            emitAsm("CMP", condReg, "0");
            emitAsm("JNE", inst.result, "", "if " + inst.arg1);
            break;
        }
            
        case TACOpcode::IF_FALSE_GOTO: {
            std::string condReg = getRegister(inst.arg1);
            emitAsm("CMP", condReg, "0");
            emitAsm("JE", inst.result, "", "ifFalse " + inst.arg1);
            break;
        }
            
        case TACOpcode::ASSIGN: {
            std::string destReg = getRegister(inst.result);
            std::string srcReg = getRegister(inst.arg1);
            emitAsm("MOV", destReg, srcReg, inst.result + " = " + inst.arg1);
            break;
        }

        case TACOpcode::CIN: {
            std::string reg = allocateRegister(inst.result);
            emitAsm("IN", reg, "", "Read input into " + inst.result);
            break;
        }

        case TACOpcode::COUT: {
            // inst.result holds the value/variable to print
            std::string reg = getRegister(inst.result);
            emitAsm("OUT", reg, "", "Print: " + inst.result);
            break;
        }
            
        case TACOpcode::ADD: {
            std::string destReg = getRegister(inst.result);
            std::string arg1Reg = getRegister(inst.arg1);
            std::string arg2Reg = getRegister(inst.arg2);
            
            emitAsm("MOV", destReg, arg1Reg);
            emitAsm("ADD", destReg, arg2Reg, inst.result + " = " + inst.arg1 + " + " + inst.arg2);
            break;
        }
            
        case TACOpcode::SUB: {
            std::string destReg = getRegister(inst.result);
            std::string arg1Reg = getRegister(inst.arg1);
            std::string arg2Reg = getRegister(inst.arg2);
            
            emitAsm("MOV", destReg, arg1Reg);
            emitAsm("SUB", destReg, arg2Reg, inst.result + " = " + inst.arg1 + " - " + inst.arg2);
            break;
        }
            
        case TACOpcode::MUL: {
            std::string destReg = getRegister(inst.result);
            std::string arg1Reg = getRegister(inst.arg1);
            std::string arg2Reg = getRegister(inst.arg2);
            
            emitAsm("MOV", destReg, arg1Reg);
            emitAsm("MUL", destReg, arg2Reg, inst.result + " = " + inst.arg1 + " * " + inst.arg2);
            break;
        }
            
        case TACOpcode::DIV: {
            std::string destReg = getRegister(inst.result);
            std::string arg1Reg = getRegister(inst.arg1);
            std::string arg2Reg = getRegister(inst.arg2);
            
            emitAsm("MOV", destReg, arg1Reg);
            emitAsm("DIV", destReg, arg2Reg, inst.result + " = " + inst.arg1 + " / " + inst.arg2);
            break;
        }
            
        case TACOpcode::EQ: {
            std::string destReg = getRegister(inst.result);
            std::string arg1Reg = getRegister(inst.arg1);
            std::string arg2Reg = getRegister(inst.arg2);
            
            emitAsm("MOV", destReg, arg1Reg);
            emitAsm("CMP", destReg, arg2Reg);
            emitAsm("SETE", destReg, "", inst.result + " = " + inst.arg1 + " == " + inst.arg2);
            break;
        }
            
        case TACOpcode::NEQ: {
            std::string destReg = getRegister(inst.result);
            std::string arg1Reg = getRegister(inst.arg1);
            std::string arg2Reg = getRegister(inst.arg2);
            emitAsm("MOV", destReg, arg1Reg);
            emitAsm("CMP", destReg, arg2Reg);
            emitAsm("SETNE", destReg, "", inst.result + " = " + inst.arg1 + " != " + inst.arg2);
            break;
        }

        case TACOpcode::LT: {
            std::string destReg = getRegister(inst.result);
            std::string arg1Reg = getRegister(inst.arg1);
            std::string arg2Reg = getRegister(inst.arg2);
            emitAsm("MOV", destReg, arg1Reg);
            emitAsm("CMP", destReg, arg2Reg);
            emitAsm("SETL", destReg, "", inst.result + " = " + inst.arg1 + " < " + inst.arg2);
            break;
        }

        case TACOpcode::GT: {
            std::string destReg = getRegister(inst.result);
            std::string arg1Reg = getRegister(inst.arg1);
            std::string arg2Reg = getRegister(inst.arg2);
            emitAsm("MOV", destReg, arg1Reg);
            emitAsm("CMP", destReg, arg2Reg);
            emitAsm("SETG", destReg, "", inst.result + " = " + inst.arg1 + " > " + inst.arg2);
            break;
        }
            
        default:
            emitAsm("; UNKNOWN INSTRUCTION: " + inst.toString());
            break;
    }
}

std::vector<AssemblyInstruction> CodeGenerator::generate(const std::vector<TACInstruction>& tacCode) {
    reset();
    
    for (const auto& inst : tacCode) {
        generateInstruction(inst);
    }
    
    return assembly;
}

// TargetCodeOptimizer implementation
TargetCodeOptimizer::TargetCodeOptimizer() {}

void TargetCodeOptimizer::setAssembly(const std::vector<AssemblyInstruction>& code) {
    assembly = code;
}

std::vector<AssemblyInstruction> TargetCodeOptimizer::removeRedundantMoves(
    const std::vector<AssemblyInstruction>& code) {
    
    std::vector<AssemblyInstruction> optimized;
    
    for (const auto& inst : code) {
        // Remove MOV R1, R1 (redundant move to same register)
        if (inst.opcode == "MOV" && inst.operand1 == inst.operand2) {
            continue;  // Skip redundant move
        }
        optimized.push_back(inst);
    }
    
    return optimized;
}

std::vector<AssemblyInstruction> TargetCodeOptimizer::instructionReordering(
    const std::vector<AssemblyInstruction>& code) {
    
    std::vector<AssemblyInstruction> optimized;
    
    // Simple reordering: try to avoid pipeline stalls
    // This is a simplified version - real optimization would need dependency analysis
    
    for (size_t i = 0; i < code.size(); i++) {
        const auto& inst = code[i];
        
        // Look ahead to see if we can reorder
        if (i + 1 < code.size()) {
            const auto& nextInst = code[i + 1];
            
            // If current instruction doesn't affect next instruction, 
            // and there's a better instruction to execute, swap them
            // (This is a very simplified heuristic)
            
            // For now, just emit as-is (full implementation would be complex)
            optimized.push_back(inst);
        } else {
            optimized.push_back(inst);
        }
    }
    
    return optimized;
}

std::vector<AssemblyInstruction> TargetCodeOptimizer::optimize() {
    assembly = removeRedundantMoves(assembly);
    assembly = instructionReordering(assembly);
    return assembly;
}
