#ifndef TAC_GENERATOR_H
#define TAC_GENERATOR_H

#include "AST.h"
#include <string>
#include <vector>
#include <memory>

// Three-Address Code Instruction Types
enum class TACOpcode {
    ASSIGN,      // x = y
    ADD,         // x = y + z
    SUB,         // x = y - z
    MUL,         // x = y * z
    DIV,         // x = y / z
    EQ,          // x = y == z
    NEQ,         // x = y != z
    LT,          // x = y < z
    GT,          // x = y > z
    CIN,         // cin >> x
    COUT,        // cout << x
    LABEL,       // L1:
    GOTO,        // goto L1
    IF_GOTO,     // if x goto L1
    IF_FALSE_GOTO, // ifFalse x goto L1
    PARAM,       // param x
    CALL,        // x = call f, n
    RETURN       // return x
};

// TAC Instruction
struct TACInstruction {
    TACOpcode opcode;
    std::string result;
    std::string arg1;
    std::string arg2;
    
    TACInstruction(TACOpcode op, const std::string& res = "", 
                  const std::string& a1 = "", const std::string& a2 = "")
        : opcode(op), result(res), arg1(a1), arg2(a2) {}
    
    std::string toString() const;
    std::string opcodeToString() const;
};

// TAC Generator using visitor pattern
class TACGenerator : public ASTVisitor {
private:
    std::vector<TACInstruction> instructions;
    int tempCounter;
    int labelCounter;
    std::string lastResult;  // Holds the result of the last expression
    
    std::string newTemp();
    std::string newLabel();
    
    void emitInstruction(TACOpcode op, const std::string& result = "", 
                        const std::string& arg1 = "", const std::string& arg2 = "");
    void emitLabel(const std::string& label);
    void emitGoto(const std::string& label);
    void emitIfGoto(const std::string& condition, const std::string& label);
    
public:
    TACGenerator();
    
    std::vector<TACInstruction> generate(Program* program);
    std::vector<TACInstruction> generate(TranslationUnit* tu);
    const std::vector<TACInstruction>& getInstructions() const { return instructions; }
    
    void reset();
    
    // New visitor methods
    void visit(TranslationUnit* node) override;
    void visit(FunctionDecl* node) override;
    void visit(CompoundStmt* node) override;
    void visit(DeclStmt* node) override;
    void visit(VarDecl* node) override;
    void visit(DeclRefExpr* node) override;
    void visit(IntegerLiteral* node) override;
    void visit(BreakStmt* node) override;
    void visit(ReturnStmt* node) override;
    void visit(CaseStmt* node) override;
    
    // Legacy visitor methods
    void visit(Program* node) override;
    void visit(SwitchStatement* node) override;
    void visit(CaseClause* node) override;
    void visit(AssignmentStatement* node) override;
    void visit(DeclarationStatement* node) override;
    void visit(CinStatement* node) override;
    void visit(CoutStatement* node) override;
    void visit(BinaryExpression* node) override;
    void visit(Identifier* node) override;
    void visit(Constant* node) override;
    void visit(StringLiteral* node) override;
};

#endif // TAC_GENERATOR_H
