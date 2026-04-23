#include "Parser.h"
#include <sstream>
#include <algorithm>
#include <cstdlib>
#include <cerrno>
#include <climits>
#include <cctype>
#include <unordered_set>
#include <functional>

namespace {
bool tryParseInt(const std::string& text, int& out) {
    if (text.empty()) {
        return false;
    }

    errno = 0;
    char* endPtr = nullptr;
    const long value = std::strtol(text.c_str(), &endPtr, 10);
    if (errno != 0 || endPtr == text.c_str() || *endPtr != '\0') {
        return false;
    }
    if (value < INT_MIN || value > INT_MAX) {
        return false;
    }

    out = static_cast<int>(value);
    return true;
}
}

Parser::Parser() : currentTokenIndex(0) {}

Parser::Parser(const std::vector<Token>& tokenStream) : currentTokenIndex(0) {
    setTokens(tokenStream);
}

void Parser::setTokens(const std::vector<Token>& tokenStream) {
    tokens = tokenStream;
    currentTokenIndex = 0;
    derivationSteps.clear();
    errors.clear();
    parseTree.reset();
    astRoot.reset();
    includeDirectives.clear();
    hasUsingNamespaceStd = false;
    usesMainWrapper = false;
    parsedReturnValue = 0;
}

Token& Parser::currentToken() {
    if (currentTokenIndex < tokens.size()) {
        return tokens[currentTokenIndex];
    }
    return eofToken;
}

Token& Parser::peekToken(int offset) {
    size_t index = currentTokenIndex + offset;
    if (index < tokens.size()) {
        return tokens[index];
    }
    return eofToken;
}

bool Parser::match(TokenType type) {
    return currentToken().type == type;
}

bool Parser::isTypeToken(TokenType type) const {
    return type == TokenType::INT || type == TokenType::STRING;
}

void Parser::consume(TokenType type, const std::string& errorMsg) {
    if (match(type)) {
        advance();
    } else {
        // Report the error at the END of the previous token (where the missing
        // token should appear), not at the current (unexpected) token.
        addErrorAtPrev(errorMsg, Token(type, "", 0, 0).typeToString());
        // Try to recover
        synchronize();
    }
}

void Parser::advance() {
    if (currentTokenIndex < tokens.size()) {
        currentTokenIndex++;
    }
}

void Parser::addDerivationStep(const std::string& sententialForm, 
                              const std::string& rule,
                              const std::string& rightmostNonTerm) {
    // Find position of rightmost non-terminal
    int pos = -1;
    if (!rightmostNonTerm.empty()) {
        size_t foundPos = sententialForm.rfind(rightmostNonTerm);
        if (foundPos != std::string::npos) {
            pos = static_cast<int>(foundPos);
