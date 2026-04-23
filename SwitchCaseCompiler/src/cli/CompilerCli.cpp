#include "../core/Lexer.h"
#include "../core/Parser.h"
#include "../core/SemanticAnalyzer.h"
#include "../core/TACGenerator.h"
#include "../core/Optimizer.h"
#include "../core/CodeGenerator.h"

#include <fstream>
#include <iostream>
#include <iterator>
#include <string>

namespace {

std::string readTextFile(const std::string& filePath) {
    std::ifstream in(filePath, std::ios::in | std::ios::binary);
    if (!in) {
        return "";
    }
    return std::string((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
}

void printUsage() {
    std::cout << "Usage: SwitchCaseCompilerCLI --input <path> [--no-cf] [--no-dce] [--no-cse]\\n";
}

} // namespace

int main(int argc, char* argv[]) {
    std::string inputPath;
    bool enableConstantFolding = true;
    bool enableDeadCodeElimination = true;
    bool enableCse = true;

    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];

        if (arg == "--input" && i + 1 < argc) {
            inputPath = argv[++i];
        } else if (arg == "--no-cf") {
            enableConstantFolding = false;
        } else if (arg == "--no-dce") {
            enableDeadCodeElimination = false;
        } else if (arg == "--no-cse") {
            enableCse = false;
        } else if (arg == "--help" || arg == "-h") {
            printUsage();
            return 0;
        } else {
            std::cerr << "Unknown argument: " << arg << "\\n";
            printUsage();
            return 64;
        }
    }

    if (inputPath.empty()) {
        std::cerr << "Missing required --input argument.\\n";
        printUsage();
        return 64;
    }

    const std::string sourceCode = readTextFile(inputPath);
    if (sourceCode.empty()) {
        std::cerr << "Failed to read input file or file is empty: " << inputPath << "\\n";
        return 66;
    }

    Lexer lexer;
    lexer.setSource(sourceCode);
    const auto tokens = lexer.tokenize();

    std::cout << "[phase] lexer\\n";
    std::cout << "[tokens] " << tokens.size() << "\\n";    for (size_t i = 0; i < tokens.size(); i++) {
        std::cout << "Token " << i << ": " << tokens[i].lexeme << "\n";
    }
    if (!lexer.getErrors().empty()) {
        for (const auto& err : lexer.getErrors()) {
            std::cout << "[error][lexer] " << err.line << ":" << err.column << " " << err.message;
            if (err.illegalChar != '\0') {
                std::cout << " ('" << err.illegalChar << "')";
            }
            std::cout << "\\n";
        }
        return 1;
    }

    Parser parser;
    parser.setTokens(tokens);
    const bool parseSuccess = parser.parse();

    std::cout << "[phase] parser\\n";
    std::cout << "[derivation_steps] " << parser.getDerivationSteps().size() << "\\n";

    if (!parseSuccess) {
        for (const auto& err : parser.getErrors()) {
            std::cout << "[error][parser] " << err.line << ":" << err.column << " " << err.message << "\\n";
        }
        return 2;
    }

    SemanticAnalyzer semanticAnalyzer;
    const bool semanticSuccess = semanticAnalyzer.analyze(parser.getAST());

    std::cout << "[phase] semantic\\n";

    if (!semanticSuccess) {
        for (const auto& err : semanticAnalyzer.getErrors()) {
            std::cout << "[error][semantic] " << err.line << ":" << err.column << " [" << err.errorType << "] "
                      << err.message << "\\n";
        }
        return 3;
    }

    TACGenerator tacGenerator;
    const auto tac = tacGenerator.generate(parser.getAST());

    Optimizer optimizer;
    optimizer.setInstructions(tac);
    const auto optimizedTac = optimizer.optimize(
        enableConstantFolding,
        enableDeadCodeElimination,
        enableCse
    );

    CodeGenerator codeGenerator;
    const auto assembly = codeGenerator.generate(optimizedTac);

    TargetCodeOptimizer targetOptimizer;
    targetOptimizer.setAssembly(assembly);
    const auto optimizedAssembly = targetOptimizer.optimize();

    std::cout << "[phase] codegen\\n";
    std::cout << "[tac] " << tac.size() << "\\n";
    std::cout << "[tac_optimized] " << optimizedTac.size() << "\\n";
    std::cout << "[asm] " << assembly.size() << "\\n";
    std::cout << "[asm_optimized] " << optimizedAssembly.size() << "\\n";
    std::cout << "[status] success\\n";

    return 0;
}
