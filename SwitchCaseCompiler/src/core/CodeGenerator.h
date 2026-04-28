#ifndef CODE_GENERATOR_H
#define CODE_GENERATOR_H

#include "TACGenerator.h"
#include <string>
#include <vector>
#include <map>

// Assembly instruction
struct AssemblyInstruction {
    std::string opcode;
    std::string operand1;
    std::string operand2;
    std::string comment;
    
    AssemblyInstruction(const std::string& op, const std::string& op1 = "", 
                       const std::string& op2 = "", const std::string& cmt = "")
        : opcode(op), operand1(op1), operand2(op2), comment(cmt) {}
    
    std::string toString() const;
};

// Code Generator (TAC to Assembly)
class CodeGenerator {
private:
    std::vector<AssemblyInstruction> assembly;
    std::map<std::string, std::string> registerAllocation;  // variable/temp -> register
    std::vector<std::string> registerPool;

    
    std::string allocateRegister(const std::string& var);
    std::string getRegister(const std::string& var);
    void freeRegister(const std::string& var);
    
    void emitAsm(const std::string& op, const std::string& op1 = "", 
                 const std::string& op2 = "", const std::string& comment = "");
    
    void generateInstruction(const TACInstruction& inst);
    
public:
    CodeGenerator();
    
    std::vector<AssemblyInstruction> generate(const std::vector<TACInstruction>& tacCode);
    const std::vector<AssemblyInstruction>& getAssembly() const { return assembly; }
    
    void reset();
};

#endif // CODE_GENERATOR_H
