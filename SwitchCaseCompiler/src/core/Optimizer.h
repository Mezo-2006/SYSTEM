#ifndef OPTIMIZER_H
#define OPTIMIZER_H

#include "TACGenerator.h"
#include <vector>
#include <map>
#include <set>
#include <string>

// Optimization result for before/after comparison
struct OptimizationResult {
    std::string optimizationName;
    std::vector<TACInstruction> before;
    std::vector<TACInstruction> after;
    int removedInstructions;
    int modifiedInstructions;
    
    OptimizationResult(const std::string& name)
        : optimizationName(name), removedInstructions(0), modifiedInstructions(0) {}
};

class Optimizer {
private:
    std::vector<TACInstruction> instructions;
    std::vector<TACInstruction> originalInstructions;  // Preserved raw TAC before any optimization
    std::vector<OptimizationResult> optimizationResults;
    
    // Individual optimization passes
    std::vector<TACInstruction> constantFolding(const std::vector<TACInstruction>& code);
    std::vector<TACInstruction> deadCodeElimination(const std::vector<TACInstruction>& code);
    std::vector<TACInstruction> commonSubexpressionElimination(const std::vector<TACInstruction>& code);
    std::vector<TACInstruction> algebraicSimplification(const std::vector<TACInstruction>& code);
    std::vector<TACInstruction> copyPropagation(const std::vector<TACInstruction>& code);
    std::vector<TACInstruction> strengthReduction(const std::vector<TACInstruction>& code);
    
    // Helper methods
    bool isConstant(const std::string& str);
    int evaluateConstant(const std::string& str);
    std::string computeConstantExpression(TACOpcode op, const std::string& arg1, const std::string& arg2);
    
    std::set<std::string> findLiveVariables(const std::vector<TACInstruction>& code);
    bool isDeadCode(const TACInstruction& inst, const std::set<std::string>& liveVars);
    
    std::string expressionKey(const TACInstruction& inst);
    
public:
    Optimizer();
    
    void setInstructions(const std::vector<TACInstruction>& code);
    
    // Optimization methods (can be toggled)
    void optimizeConstantFolding(bool enabled);
    void optimizeDeadCode(bool enabled);
    void optimizeCSE(bool enabled);
    void optimizeAlgebraicSimplification(bool enabled);
    void optimizeCopyPropagation(bool enabled);
    void optimizeStrengthReduction(bool enabled);
    
    // Apply all enabled optimizations
    std::vector<TACInstruction> optimize(bool constFold, bool deadCode, bool cse, 
                                       bool algebraic = true, bool copyProp = true, bool strengthRed = true);
    
    const std::vector<TACInstruction>& getInstructions() const { return instructions; }
    const std::vector<TACInstruction>& getOriginalInstructions() const { return originalInstructions; }
    const std::vector<OptimizationResult>& getOptimizationResults() const { 
        return optimizationResults; 
    }
};

#endif // OPTIMIZER_H
