#include "src/core/Lexer.h"
#include "src/core/Parser.h"
#include <iostream>
#include <fstream>

int main() {
    std::string code = R"(
int x;
switch (x) {
    case 1:
        x = 5;
        break;
    default:
        x = 0;
        break;
}
)";

    Lexer lexer(code);
    auto tokens = lexer.tokenize();
    
    Parser parser(tokens);
    parser.parse();
    
    std::cout << "=== RIGHTMOST DERIVATION SEQUENCE ===\n\n";
    
    const auto& steps = parser.getDerivationSteps();
    for (size_t i = 0; i < steps.size(); ++i) {
        std::cout << "Step " << (i + 1) << ":\n";
        std::cout << "  Previous: " << steps[i].previousSententialForm << "\n";
        std::cout << "  Current:  " << steps[i].sententialForm << "\n";
        std::cout << "  Rule:     " << steps[i].productionRule << "\n";
        std::cout << "\n";
    }
    
    return 0;
}
