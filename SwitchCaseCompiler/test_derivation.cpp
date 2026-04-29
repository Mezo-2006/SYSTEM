#include <iostream>
#include <fstream>
#include "src/core/Lexer.h"
#include "src/core/Parser.h"

int main() {
    std::string source = R"(
        switch (x) {
            case 1: y = 2; break;
            default: z = 3; break;
        }
    )";
    Lexer lexer(source);
    auto tokens = lexer.tokenize();
    Parser parser(tokens);
    parser.parse();
    
    std::ofstream out("derivation_output.txt");
    int stepNum = 1;
    for (const auto& step : parser.getDerivationSteps()) {
        out << stepNum++ << ". " << step.productionRule << "\n";
        out << "   Form: " << step.sententialForm << "\n";
        out << "   Rightmost: " << step.rightmostNonTerminal << "\n\n";
    }
    return 0;
}
