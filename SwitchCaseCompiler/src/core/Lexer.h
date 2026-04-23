#ifndef LEXER_H
#define LEXER_H

#include <string>
#include <vector>
#include <map>
#include <memory>

// Token types
enum class TokenType {
    // Keywords - Control flow
    SWITCH,
    CASE,
    DEFAULT,
    BREAK,
    RETURN,
    
    // Keywords - Types
    INT,
    FLOAT,
    STRING,
    VOID,
    
    // Keywords - I/O
    COUT,
    CIN,
    
    // Keywords - Standard
    USING,
    NAMESPACE,
    STD,
    INCLUDE,
    MAIN,
    
    // Identifiers and constants
    ID,
    CONSTANT,
    STRING_LITERAL,
    
    // Operators - Arithmetic
    PLUS,       // +
    MINUS,      // -
    MUL,        // *
    DIV,        // /
    ASSIGN,     // =
    
    // Operators - Stream
    LSHIFT,     // <<
    RSHIFT,     // >>
    
    // Delimiters
    LPAREN,     // (
    RPAREN,     // )
    LBRACE,     // {
    RBRACE,     // }
    COLON,      // :
    SEMICOLON,  // ;
    COMMA,      // ,
    HASH,       // #
    LT,         // <
    GT,         // >
    DOT,        // .
    
    // Special
    END_OF_FILE,
    INVALID
};

// Token structure
struct Token {
    TokenType type;
    std::string lexeme;
    int line;
    int column;
    
    Token(TokenType t = TokenType::INVALID, const std::string& lex = "", int ln = 0, int col = 0)
        : type(t), lexeme(lex), line(ln), column(col) {}
    
    std::string toString() const;
    std::string typeToString() const;
};

// Symbol table entry
struct SymbolTableEntry {
    std::string name;
    std::string type;
    std::string value;
    int scope;
    int line;
    int column;
    
    SymbolTableEntry(const std::string& n = "", const std::string& t = "", 
                    const std::string& v = "", int s = 0, int ln = 0, int col = 0)
        : name(n), type(t), value(v), scope(s), line(ln), column(col) {}
};

// Lexical error structure
struct LexicalError {
    std::string message;
    int line;
    int column;
    char illegalChar;
    
    LexicalError(const std::string& msg, int ln, int col, char ch = '\0')
        : message(msg), line(ln), column(col), illegalChar(ch) {}
};

// Lexer class
class Lexer {
private:
    std::string source;
    size_t position;
    int line;
    int column;
    char currentChar;
    
    std::vector<Token> tokens;
    std::map<std::string, SymbolTableEntry> symbolTable;
    std::vector<LexicalError> errors;
    std::map<std::string, TokenType> keywords;
    
    void advance();
    void skipWhitespace();
    void skipComment();
    char peek(int offset = 1);
    
    Token scanNumber();
    Token scanIdentifierOrKeyword();
    Token scanOperatorOrDelimiter();
    Token scanStringLiteral();
    
    void addError(const std::string& message, char illegalChar = '\0');
    void addToSymbolTable(const std::string& name, const std::string& type);

public:
    Lexer();
    explicit Lexer(const std::string& sourceCode);
    
    void setSource(const std::string& sourceCode);
    std::vector<Token> tokenize();
    
    const std::vector<Token>& getTokens() const { return tokens; }
    const std::map<std::string, SymbolTableEntry>& getSymbolTable() const { return symbolTable; }
    const std::vector<LexicalError>& getErrors() const { return errors; }
    
    void reset();
};

#endif // LEXER_H
