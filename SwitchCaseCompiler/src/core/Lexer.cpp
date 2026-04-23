#include "Lexer.h"
#include <cctype>
#include <sstream>
#include <limits>

// Token methods
std::string Token::typeToString() const {
    switch (type) {
        // Control flow
        case TokenType::SWITCH: return "SWITCH";
        case TokenType::CASE: return "CASE";
        case TokenType::DEFAULT: return "DEFAULT";
        case TokenType::BREAK: return "BREAK";
        case TokenType::RETURN: return "RETURN";
        // Types
        case TokenType::INT: return "INT";
        case TokenType::FLOAT: return "FLOAT";
        case TokenType::STRING: return "STRING";
        case TokenType::VOID: return "VOID";
        // I/O
        case TokenType::COUT: return "COUT";
        case TokenType::CIN: return "CIN";
        // Standard
        case TokenType::USING: return "USING";
        case TokenType::NAMESPACE: return "NAMESPACE";
        case TokenType::STD: return "STD";
        case TokenType::INCLUDE: return "INCLUDE";
        case TokenType::MAIN: return "MAIN";
        // Identifiers
        case TokenType::ID: return "ID";
        case TokenType::CONSTANT: return "CONSTANT";
        case TokenType::STRING_LITERAL: return "STRING_LITERAL";
        // Arithmetic operators
        case TokenType::PLUS: return "PLUS";
        case TokenType::MINUS: return "MINUS";
        case TokenType::MUL: return "MUL";
        case TokenType::DIV: return "DIV";
        case TokenType::ASSIGN: return "ASSIGN";
        // Stream operators
        case TokenType::LSHIFT: return "LSHIFT";
        case TokenType::RSHIFT: return "RSHIFT";
        // Delimiters
        case TokenType::LPAREN: return "LPAREN";
        case TokenType::RPAREN: return "RPAREN";
        case TokenType::LBRACE: return "LBRACE";
        case TokenType::RBRACE: return "RBRACE";
        case TokenType::COLON: return "COLON";
        case TokenType::SEMICOLON: return "SEMICOLON";
        case TokenType::COMMA: return "COMMA";
        case TokenType::HASH: return "HASH";
        case TokenType::LT: return "LT";
        case TokenType::GT: return "GT";
        case TokenType::DOT: return "DOT";
        // Special
        case TokenType::END_OF_FILE: return "EOF";
        case TokenType::INVALID: return "INVALID";
        default: return "UNKNOWN";
    }
}

std::string Token::toString() const {
    std::stringstream ss;
    ss << "<" << typeToString() << ", \"" << lexeme << "\", " << line << ":" << column << ">";
    return ss.str();
}

// Lexer implementation
Lexer::Lexer() : position(0), line(1), column(1), currentChar('\0') {
    // Initialize keywords map - Control flow
    keywords["switch"] = TokenType::SWITCH;
    keywords["case"] = TokenType::CASE;
    keywords["default"] = TokenType::DEFAULT;
    keywords["break"] = TokenType::BREAK;
    keywords["return"] = TokenType::RETURN;
    
    // Types
    keywords["int"] = TokenType::INT;
    keywords["float"] = TokenType::FLOAT;
    keywords["string"] = TokenType::STRING;
    keywords["void"] = TokenType::VOID;
    
    // I/O
    keywords["cout"] = TokenType::COUT;
    keywords["cin"] = TokenType::CIN;
    
    // Standard
    keywords["using"] = TokenType::USING;
    keywords["namespace"] = TokenType::NAMESPACE;
    keywords["std"] = TokenType::STD;
    keywords["include"] = TokenType::INCLUDE;
    keywords["main"] = TokenType::MAIN;
}

Lexer::Lexer(const std::string& sourceCode) : Lexer() {
    setSource(sourceCode);
}

void Lexer::setSource(const std::string& sourceCode) {
    source = sourceCode;
    reset();
}

void Lexer::reset() {
    position = 0;
    line = 1;
    column = 1;
    tokens.clear();
    symbolTable.clear();
    errors.clear();
    currentChar = source.empty() ? '\0' : source[0];
}

void Lexer::advance() {
    if (position < source.length()) {
        if (currentChar == '\n') {
            line++;
            column = 1;
        } else {
            column++;
        }
        position++;
        currentChar = (position < source.length()) ? source[position] : '\0';
    }
}

char Lexer::peek(int offset) {
    size_t peekPos = position + offset;
    return (peekPos < source.length()) ? source[peekPos] : '\0';
}

void Lexer::skipWhitespace() {
    while (currentChar != '\0' && std::isspace(currentChar)) {
        advance();
    }
}

void Lexer::skipComment() {
    // Single line comment: //
    if (currentChar == '/' && peek() == '/') {
        while (currentChar != '\0' && currentChar != '\n') {
            advance();
        }
    }
    // Multi-line comment: /* ... */
    else if (currentChar == '/' && peek() == '*') {
        advance(); // skip /
        advance(); // skip *
        while (currentChar != '\0') {
            if (currentChar == '*' && peek() == '/') {
                advance(); // skip *
                advance(); // skip /
                break;
            }
            advance();
        }
    }
}

Token Lexer::scanNumber() {
    int startLine = line;
    int startCol = column;
    std::string number;
    
    while (currentChar != '\0' && std::isdigit(currentChar)) {
        number += currentChar;
        advance();
    }

    constexpr size_t maxIntDigits = 10; // 2147483647
    if (number.length() > maxIntDigits) {
        addError("Integer literal is too large", '\0');
    } else if (number.length() == maxIntDigits && number > "2147483647") {
        addError("Integer literal is too large", '\0');
    }
    
    Token token(TokenType::CONSTANT, number, startLine, startCol);
    return token;
}

Token Lexer::scanIdentifierOrKeyword() {
    int startLine = line;
    int startCol = column;
    std::string identifier;
    
    // First character must be letter or underscore
    while (currentChar != '\0' && (std::isalnum(currentChar) || currentChar == '_')) {
        identifier += currentChar;
        advance();
    }
    
    // Check if it's a keyword
    TokenType type = TokenType::ID;
    auto it = keywords.find(identifier);
    if (it != keywords.end()) {
        type = it->second;
    } else {
        // Add to symbol table
        addToSymbolTable(identifier, "identifier");
    }
    
    Token token(type, identifier, startLine, startCol);
    return token;
}

Token Lexer::scanOperatorOrDelimiter() {
    int startLine = line;
    int startCol = column;
    char ch = currentChar;
    std::string lexeme(1, ch);
    
    TokenType type = TokenType::INVALID;
    
    switch (ch) {
        case '+': type = TokenType::PLUS; break;
        case '-': type = TokenType::MINUS; break;
        case '*': type = TokenType::MUL; break;
        case '/': type = TokenType::DIV; break;
        case '=': type = TokenType::ASSIGN; break;
        case '(': type = TokenType::LPAREN; break;
        case ')': type = TokenType::RPAREN; break;
        case '{': type = TokenType::LBRACE; break;
        case '}': type = TokenType::RBRACE; break;
        case ':': type = TokenType::COLON; break;
        case ';': type = TokenType::SEMICOLON; break;
        case ',': type = TokenType::COMMA; break;
        case '#': type = TokenType::HASH; break;
        case '<':
            // Check for <<
            if (peek() == '<') {
                lexeme += '<';
                advance();
                type = TokenType::LSHIFT;
            } else {
                type = TokenType::LT;
            }
            break;
        case '>':
            // Check for >>
            if (peek() == '>') {
                lexeme += '>';
                advance();
                type = TokenType::RSHIFT;
            } else {
                type = TokenType::GT;
            }
            break;
        case '.':
            type = TokenType::DOT;
            break;
        default:
            addError("Illegal character", ch);
            type = TokenType::INVALID;
    }
    
    advance();
    return Token(type, lexeme, startLine, startCol);
}

Token Lexer::scanStringLiteral() {
    int startLine = line;
    int startCol = column;
    std::string str;
    
    // Skip opening quote
    advance();
    
    while (currentChar != '\0' && currentChar != '"') {
        if (currentChar == '\\' && peek() != '\0') {
            // Handle escape sequences
            advance();
            switch (currentChar) {
                case 'n': str += '\n'; break;
                case 't': str += '\t'; break;
                case 'r': str += '\r'; break;
                case '\\': str += '\\'; break;
                case '"': str += '"'; break;
                default: str += currentChar; break;
            }
            advance();
        } else {
            str += currentChar;
            advance();
        }
    }
    
    if (currentChar == '"') {
        advance(); // Skip closing quote
    } else {
        addError("Unterminated string literal", '\0');
    }
    
    return Token(TokenType::STRING_LITERAL, str, startLine, startCol);
}

void Lexer::addError(const std::string& message, char illegalChar) {
    errors.push_back(LexicalError(message, line, column, illegalChar));
}

void Lexer::addToSymbolTable(const std::string& name, const std::string& type) {
    if (symbolTable.find(name) == symbolTable.end()) {
        symbolTable[name] = SymbolTableEntry(name, type, "", 0, line, column);
    }
}

std::vector<Token> Lexer::tokenize() {
    tokens.clear();
    errors.clear();
    symbolTable.clear();

    constexpr size_t maxSourceLength = 1024 * 1024; // 1 MiB hard limit
    if (source.length() > maxSourceLength) {
        addError("Source code exceeds maximum supported size (1 MiB)", '\0');
        tokens.push_back(Token(TokenType::END_OF_FILE, "", line, column));
        return tokens;
    }
    
    while (currentChar != '\0') {
        // Skip whitespace
        if (std::isspace(currentChar)) {
            skipWhitespace();
            continue;
        }

        // Skip comments
        if (currentChar == '/' && (peek() == '/' || peek() == '*')) {
            skipComment();
            continue;
        }
        
        // Scan string literals
        if (currentChar == '"') {
            tokens.push_back(scanStringLiteral());
            continue;
        }
        
        // Scan numbers
        if (std::isdigit(currentChar)) {
            tokens.push_back(scanNumber());
            continue;
        }
        
        // Scan identifiers and keywords
        if (std::isalpha(currentChar) || currentChar == '_') {
            tokens.push_back(scanIdentifierOrKeyword());
            continue;
        }
        
        // Scan operators and delimiters (updated for new tokens)
        if (std::string("+-*/=(){}:;,#<>.").find(currentChar) != std::string::npos) {
            tokens.push_back(scanOperatorOrDelimiter());
            continue;
        }
        
        // Invalid character
        addError("Illegal character", currentChar);
        advance();
    }
    
    // Add EOF token
    tokens.push_back(Token(TokenType::END_OF_FILE, "", line, column));
    
    return tokens;
}
