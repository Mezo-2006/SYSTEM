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

    if (tokens.empty()) {
        eofToken = Token(TokenType::END_OF_FILE, "", 1, 1);
        return;
    }

    const Token& last = tokens.back();
    if (last.type == TokenType::END_OF_FILE) {
        eofToken = last;
        if (eofToken.line <= 0) {
            eofToken.line = 1;
        }
        if (eofToken.column <= 0) {
            eofToken.column = 1;
        }
        return;
    }

    const int eofLine = std::max(1, last.line);
    const int eofColumn = std::max(1, last.column + static_cast<int>(last.lexeme.size()));
    eofToken = Token(TokenType::END_OF_FILE, "", eofLine, eofColumn);
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
        }
    }
    
    derivationSteps.push_back(DerivationStep(sententialForm, rule, pos, rightmostNonTerm));
}

void Parser::addError(const std::string& message, const std::string& expected) {
    Token& tok = currentToken();
    errors.push_back(ParseError(message, tok.line, tok.column, expected, tok.lexeme));
}

void Parser::addErrorAtPrev(const std::string& message, const std::string& expected) {
    // Point to the end of the previous token — that is where the missing
    // punctuation (e.g. ';') should have appeared.
    if (currentTokenIndex > 0) {
        const Token& prev = tokens[currentTokenIndex - 1];
        // column of the character just after the previous token
        int col = prev.column + static_cast<int>(prev.lexeme.size());
        errors.push_back(ParseError(message, prev.line, col, expected, prev.lexeme));
    } else {
        // Fallback: no previous token, use current
        Token& tok = currentToken();
        errors.push_back(ParseError(message, tok.line, tok.column, expected, tok.lexeme));
    }
}

void Parser::synchronize() {
    // Skip tokens until we find a synchronization point
    while (!match(TokenType::END_OF_FILE)) {
        if (match(TokenType::SEMICOLON) || match(TokenType::RBRACE) || 
            match(TokenType::BREAK) || match(TokenType::CASE) || 
            match(TokenType::DEFAULT)) {
            advance();
            return;
        }
        advance();
    }
}

bool Parser::parse() {
    errors.clear();
    derivationSteps.clear();
    currentTokenIndex = 0;
    includeDirectives.clear();
    hasUsingNamespaceStd = false;
    usesMainWrapper = false;
    parsedReturnValue = 0;

    // Build AST using recursive descent
    currentTokenIndex = 0;
    astRoot = parseProgram();

    if (!match(TokenType::END_OF_FILE)) {
        addError("Unexpected tokens after end of program");
    }
    
    // Build new TranslationUnit AST from legacy Program AST
    if (useNewAST && astRoot) {
        translationUnit = convertToTranslationUnit(astRoot.get());
    }
    
    // Run validation checks
    validateNoLoops();
    validateOnlyOneSwitch();
    validateOnlyValidStatements();
    // Keep legacy subset compatibility: main/include wrappers are optional.
    
    // Build parse tree for visualization
    currentTokenIndex = 0;
    parseTree = buildParseTree();

    // Build rightmost derivation from parse-tree expansions
    performRightmostDerivation();
    
    return !hasErrors();
}

void Parser::performRightmostDerivation() {
    derivationSteps.clear();
    const std::unordered_set<std::string> nonTerminals = {
        "program", "include_list", "include_line", "header_name", "preamble_opt", "using_opt",
        "main_func", "return_stmt",
        "pre_stmt_list", "pre_stmt", "declaration", "type_spec", "decl_init_opt", "assignment",
        "switch_stmt", "case_list", "case_clause", "default_clause", "stmt_list",
        "cin_stmt", "cout_stmt",
        "stmt", "expr", "expr_tail", "term", "term_tail", "factor"
    };

    const auto isNonTerminal = [&nonTerminals](const std::string& symbol) {
        return nonTerminals.find(symbol) != nonTerminals.end();
    };

    const auto joinSymbols = [](const std::vector<std::string>& symbols) {
        if (symbols.empty()) {
            return std::string("ε");
        }
        std::ostringstream out;
        for (size_t i = 0; i < symbols.size(); ++i) {
            if (i > 0) {
                out << " ";
            }
            out << symbols[i];
        }
        return out.str();
    };

    if (!parseTree) {
        addDerivationStep("program", "Start Symbol", "program");
        return;
    }

    struct Expansion {
        ParseTreeNode* node;
        std::string lhs;
        std::vector<std::string> rhs;
    };

    std::vector<Expansion> expansions;

    std::function<void(ParseTreeNode*)> collectRightmostExpansions = [&](ParseTreeNode* node) {
        if (!node || node->isTerminal || !isNonTerminal(node->label)) {
            return;
        }

        std::vector<std::string> rhs;
        if (node->children.empty()) {
            rhs.push_back("ε");
        } else {
            for (const auto& child : node->children) {
                rhs.push_back(child->label);
            }
        }
        expansions.push_back({node, node->label, rhs});

        for (auto it = node->children.rbegin(); it != node->children.rend(); ++it) {
            if (!(*it)->isTerminal && isNonTerminal((*it)->label)) {
                collectRightmostExpansions(it->get());
            }
        }
    };

    collectRightmostExpansions(parseTree.get());

    std::vector<std::string> sententialForm = {parseTree->label.empty() ? "program" : parseTree->label};
    parseTree->derivationStep = 0;
    addDerivationStep(joinSymbols(sententialForm), "Start Symbol", sententialForm.front());

    for (const auto& exp : expansions) {
        int replaceIndex = -1;
        for (int i = static_cast<int>(sententialForm.size()) - 1; i >= 0; --i) {
            if (sententialForm[static_cast<size_t>(i)] == exp.lhs) {
                replaceIndex = i;
                break;
            }
        }
        if (replaceIndex < 0) {
            continue;
        }

        sententialForm.erase(sententialForm.begin() + replaceIndex);
        if (!(exp.rhs.size() == 1 && exp.rhs[0] == "ε")) {
            sententialForm.insert(sententialForm.begin() + replaceIndex, exp.rhs.begin(), exp.rhs.end());
        }

        std::string rightmostNonTerminal;
        for (int i = static_cast<int>(sententialForm.size()) - 1; i >= 0; --i) {
            if (isNonTerminal(sententialForm[static_cast<size_t>(i)])) {
                rightmostNonTerminal = sententialForm[static_cast<size_t>(i)];
                break;
            }
        }

        if (exp.node) {
            exp.node->derivationStep = static_cast<int>(derivationSteps.size());
        }

        addDerivationStep(
            joinSymbols(sententialForm),
            exp.lhs + " → " + joinSymbols(exp.rhs),
            rightmostNonTerminal
        );
    }

    addDerivationStep(joinSymbols(sententialForm), "Derivation Complete", "");
}

std::unique_ptr<Program> Parser::parseProgram() {
    // program -> include_list preamble_opt main_func
    // program -> preamble_opt pre_stmt_list switch_stmt (legacy subset)

    for (const auto& tok : tokens) {
        if (tok.type == TokenType::ID) {
            if (tok.lexeme == "for" || tok.lexeme == "while" || tok.lexeme == "do") {
                errors.push_back(ParseError(
                    "ERROR: Loops are not supported. Use declarations, assignments, expressions, and switch-case only.",
                    tok.line,
                    tok.column
                ));
                return std::make_unique<Program>(std::vector<std::unique_ptr<Statement>>{}, nullptr);
            }
        }
    }
    
    // Optional include list
    int includeCount = 0;
    while (match(TokenType::HASH)) {
        IncludeDirective include;
        include.angled = true;
        include.headerLeft.clear();
        include.headerRight.clear();

        consume(TokenType::HASH, "Expected '#'");
        consume(TokenType::INCLUDE, "Expected 'include' after '#'");

        if (match(TokenType::LT)) {
            advance();
            if (match(TokenType::ID) || match(TokenType::STRING)) {
                include.headerLeft = currentToken().lexeme;
                advance();
                if (match(TokenType::DOT)) {
                    advance();
                    if (match(TokenType::ID) || match(TokenType::STRING)) {
                        include.headerRight = currentToken().lexeme;
                        advance();
                    } else {
                        addError("Expected identifier after '.' in header name");
                    }
                }
            } else {
                addError("Expected header name identifier after '<'");
            }
            consume(TokenType::GT, "Expected '>' after header name");
        } else if (match(TokenType::STRING_LITERAL)) {
            include.angled = false;
            const std::string full = currentToken().lexeme;
            const size_t dotPos = full.find('.');
            if (dotPos == std::string::npos) {
                include.headerLeft = full;
            } else {
                include.headerLeft = full.substr(0, dotPos);
                include.headerRight = full.substr(dotPos + 1);
            }
            advance();
        } else {
            addError("Expected <header> or \"header\" after #include");
            synchronize();
        }

        includeDirectives.push_back(include);
        includeCount++;
    }
    
    // Optional C++-style preamble support: using namespace std;
    while (match(TokenType::USING)) {
        consume(TokenType::USING, "Expected 'using'");
        consume(TokenType::NAMESPACE, "Expected 'namespace' after 'using'");
        if (match(TokenType::STD)) {
            advance();
            hasUsingNamespaceStd = true;
        } else {
            addError("Expected 'std' after 'using namespace'");
        }
        consume(TokenType::SEMICOLON, "Expected ';' after 'using namespace std'");
    }



    std::vector<std::unique_ptr<Statement>> preSwitchStatements;

    // Optional C++-style main wrapper: int main() { ... switch ... return 0; }
    if (!match(TokenType::INT) || peekToken().type != TokenType::MAIN) {
        std::vector<std::unique_ptr<Statement>> legacyPreSwitchStatements;

        while (!match(TokenType::END_OF_FILE) && !match(TokenType::SWITCH)) {
            if (isTypeToken(currentToken().type) || match(TokenType::ID) ||
                match(TokenType::CIN) || match(TokenType::COUT)) {
                legacyPreSwitchStatements.push_back(parseStatement());
            } else {
                addError("Expected declaration, assignment, cin/cout, or 'switch' statement");
                synchronize();
            }
        }

        if (!match(TokenType::SWITCH)) {
            addError("Program must contain a 'switch' statement");
            return std::make_unique<Program>(std::move(legacyPreSwitchStatements), nullptr);
        }

        auto switchStmt = parseSwitchStatement();
        return std::make_unique<Program>(std::move(legacyPreSwitchStatements), std::move(switchStmt));
    }

    usesMainWrapper = true;
    consume(TokenType::INT, "Expected 'int' before main");
    consume(TokenType::MAIN, "Expected 'main'");
    consume(TokenType::LPAREN, "Expected '(' after main");
    consume(TokenType::RPAREN, "Expected ')' after main");
    consume(TokenType::LBRACE, "Expected '{' after main()");

    while (!match(TokenType::END_OF_FILE) && !match(TokenType::SWITCH)) {
        if (isTypeToken(currentToken().type) || match(TokenType::ID)) {
            preSwitchStatements.push_back(parseStatement());
        } else if (match(TokenType::CIN) || match(TokenType::COUT)) {
            // Allow cin >> and cout << statements
            preSwitchStatements.push_back(parseStatement());
        } else {
            addError("Expected declaration, assignment, cin/cout, or 'switch' statement inside main");
            synchronize();
        }
    }

    if (!match(TokenType::SWITCH)) {
        addError("Program must contain a 'switch' statement inside main");
        return std::make_unique<Program>(std::move(preSwitchStatements), nullptr);
    }

    auto switchStmt = parseSwitchStatement();

    consume(TokenType::RETURN, "Expected 'return' after switch statement");
    if (match(TokenType::CONSTANT)) {
        if (!tryParseInt(currentToken().lexeme, parsedReturnValue)) {
            addError("Return value is out of integer range");
            parsedReturnValue = 0;
        }
        advance();
    } else {
        addError("Expected integer constant after 'return'");
    }
    consume(TokenType::SEMICOLON, "Expected ';' after return statement");
    consume(TokenType::RBRACE, "Expected '}' to close main function");

    return std::make_unique<Program>(std::move(preSwitchStatements), std::move(switchStmt));
}

std::unique_ptr<SwitchStatement> Parser::parseSwitchStatement() {
    // switch_stmt → switch ( expr ) { case_list default_clause }
    int startLine = currentToken().line;
    int startCol = currentToken().column;
    
    consume(TokenType::SWITCH, "Expected 'switch'");
    consume(TokenType::LPAREN, "Expected '(' after 'switch'");
    
    auto condition = parseExpression();
    
    consume(TokenType::RPAREN, "Expected ')' after switch condition");
    consume(TokenType::LBRACE, "Expected '{' after ')'");
    
    auto switchStmt = std::make_unique<SwitchStatement>(std::move(condition), startLine, startCol);
    
    // Parse case list
    switchStmt->cases = parseCaseList();
    
    // Parse default clause (optional)
    if (match(TokenType::DEFAULT)) {
        switchStmt->defaultCase = parseDefaultClause();
    }
    
    consume(TokenType::RBRACE, "Expected '}' to close switch statement");
    
    return switchStmt;
}

std::vector<std::unique_ptr<CaseClause>> Parser::parseCaseList() {
    // case_list → case_clause case_list | ε
    std::vector<std::unique_ptr<CaseClause>> cases;
    
    while (match(TokenType::CASE)) {
        cases.push_back(parseCaseClause());
    }
    
    return cases;
}

std::unique_ptr<CaseClause> Parser::parseCaseClause() {
    // case_clause → case constant : stmt_list break ;
    int startLine = currentToken().line;
    int startCol = currentToken().column;
    
    consume(TokenType::CASE, "Expected 'case'");
    
    int caseValue = 0;
    bool isNegative = false;
    
    // Handle negative constants: case -5:
    if (match(TokenType::MINUS)) {
        isNegative = true;
        advance();
    }
    
    if (match(TokenType::CONSTANT)) {
        if (!tryParseInt(currentToken().lexeme, caseValue)) {
            addError("Case value is out of integer range");
        }
        if (isNegative) {
            caseValue = -caseValue;
        }
        advance();
    } else {
        addError("Expected constant after 'case'");
    }
    
    auto caseClause = std::make_unique<CaseClause>(caseValue, startLine, startCol);
    
    consume(TokenType::COLON, "Expected ':' after case value");
    
    caseClause->statements = parseStatementList();
    
    consume(TokenType::BREAK, "Expected 'break' in case clause");
    consume(TokenType::SEMICOLON, "Expected ';' after 'break'");
    
    return caseClause;
}

std::unique_ptr<CaseClause> Parser::parseDefaultClause() {
    // default_clause → default : stmt_list break ;
    int startLine = currentToken().line;
    int startCol = currentToken().column;
    
    consume(TokenType::DEFAULT, "Expected 'default'");
    
    auto defaultClause = std::make_unique<CaseClause>(true, startLine, startCol);
    
    consume(TokenType::COLON, "Expected ':' after 'default'");
    
    defaultClause->statements = parseStatementList();

    if (match(TokenType::BREAK)) {
        defaultClause->hasBreak = true;
        consume(TokenType::BREAK, "Expected 'break' in default clause");
        consume(TokenType::SEMICOLON, "Expected ';' after 'break'");
    }

    return defaultClause;
}

std::vector<std::unique_ptr<Statement>> Parser::parseStatementList() {
    // stmt_list → stmt stmt_list | ε
    std::vector<std::unique_ptr<Statement>> statements;

    while (match(TokenType::ID) || isTypeToken(currentToken().type) || match(TokenType::CIN) || match(TokenType::COUT) || match(TokenType::SWITCH)) {
        statements.push_back(parseStatement());
    }
    
    return statements;
}

std::unique_ptr<Statement> Parser::parseStatement() {
    // stmt → declaration | assignment | cin_stmt | cout_stmt
    if (isTypeToken(currentToken().type)) {
        return parseDeclarationStatement();
    } else if (match(TokenType::CIN)) {
        return parseCinStatement();
    } else if (match(TokenType::COUT)) {
        return parseCoutStatement();
    } else if (match(TokenType::SWITCH)) {
        return parseSwitchStatement();
    }
    return parseAssignmentStatement();
}

std::unique_ptr<Statement> Parser::parseDeclarationStatement() {
    // declaration → (int|string) id [= expr] ;
    int startLine = currentToken().line;
    int startCol = currentToken().column;

    std::string varType;
    if (match(TokenType::INT)) {
        varType = "int";
        advance();
    } else if (match(TokenType::STRING)) {
        varType = "string";
        advance();
    } else {
        addError("Expected supported type declaration (int or string)");
        return std::make_unique<DeclarationStatement>("int", "", nullptr, startLine, startCol);
    }

    std::string varName;
    if (match(TokenType::ID)) {
        varName = currentToken().lexeme;
        advance();
    } else {
        addError("Expected identifier in declaration");
    }

    std::unique_ptr<Expression> initializer = nullptr;
    if (match(TokenType::ASSIGN)) {
        advance();
        initializer = parseExpression();
    }

    consume(TokenType::SEMICOLON, "Expected ';' after declaration");

    return std::make_unique<DeclarationStatement>(varType, varName, std::move(initializer), startLine, startCol);
}

std::unique_ptr<Statement> Parser::parseAssignmentStatement() {
    // assignment → id = expr ;
    int startLine = currentToken().line;
    int startCol = currentToken().column;

    std::string varName;
    if (match(TokenType::ID)) {
        varName = currentToken().lexeme;
        advance();
    }

    consume(TokenType::ASSIGN, "Expected '=' in assignment");

    auto expr = parseExpression();

    consume(TokenType::SEMICOLON, "Expected ';' after statement");

    return std::make_unique<AssignmentStatement>(varName, std::move(expr), startLine, startCol);
}

std::unique_ptr<Statement> Parser::parseCinStatement() {
    // cin_stmt → cin >> id ;
    int startLine = currentToken().line;
    int startCol = currentToken().column;
    
    consume(TokenType::CIN, "Expected 'cin'");
    consume(TokenType::RSHIFT, "Expected '>>' after 'cin'");
    
    std::string varName;
    if (match(TokenType::ID)) {
        varName = currentToken().lexeme;
        advance();
    } else {
        addError("Expected identifier after 'cin >>'");
    }
    
    consume(TokenType::SEMICOLON, "Expected ';' after cin statement");
    
    return std::make_unique<CinStatement>(varName, startLine, startCol);
}

std::unique_ptr<Statement> Parser::parseCoutStatement() {
    // cout_stmt → cout { << expr_or_manip } ;
    // Supports chained << and endl/"\n" manipulators
    int startLine = currentToken().line;
    int startCol  = currentToken().column;

    consume(TokenType::COUT, "Expected 'cout'");

    // Collect all << operands, then fold into nested BinaryExpressions
    std::vector<std::unique_ptr<Expression>> parts;

    while (match(TokenType::LSHIFT)) {
        advance();  // consume <<

        // endl → treat as "\n" string literal
        if (match(TokenType::ID) && currentToken().lexeme == "endl") {
            parts.push_back(std::make_unique<StringLiteral>("\\n", startLine, startCol));
            advance();
        }
        else if (match(TokenType::STRING_LITERAL) || match(TokenType::CONSTANT) ||
                 match(TokenType::ID)             || match(TokenType::LPAREN)) {
            parts.push_back(parseExpression());
        }
        else {
            // Unknown token after <<, stop without error
            break;
        }
    }

    consume(TokenType::SEMICOLON, "Expected ';' after cout statement");

    // No operands — emit empty string
    if (parts.empty()) {
        return std::make_unique<CoutStatement>(
            std::make_unique<StringLiteral>("", startLine, startCol),
            startLine, startCol);
    }

    // Fold left-to-right: (((p0 << p1) << p2) << p3)
    auto expr = std::move(parts[0]);
    for (size_t i = 1; i < parts.size(); ++i) {
        expr = std::make_unique<BinaryExpression>(
            "<<", std::move(expr), std::move(parts[i]), startLine, startCol);
    }

    return std::make_unique<CoutStatement>(std::move(expr), startLine, startCol);
}

std::unique_ptr<Expression> Parser::parseExpression() {
    // expr → expr + term | expr - term | term
    // Left-factored: expr → term { (+|-) term }
    auto left = parseTerm();
    
    while (match(TokenType::PLUS) || match(TokenType::MINUS)) {
        std::string op = currentToken().lexeme;
        int line = currentToken().line;
        int col = currentToken().column;
        advance();
        
        auto right = parseTerm();
        left = std::make_unique<BinaryExpression>(op, std::move(left), std::move(right), line, col);
    }
    
    return left;
}

std::unique_ptr<Expression> Parser::parseTerm() {
    // term → term * factor | term / factor | factor
    // Left-factored: term → factor { (*|/) factor }
    auto left = parseFactor();
    
    while (match(TokenType::MUL) || match(TokenType::DIV)) {
        std::string op = currentToken().lexeme;
        int line = currentToken().line;
        int col = currentToken().column;
        advance();
        
        auto right = parseFactor();
        left = std::make_unique<BinaryExpression>(op, std::move(left), std::move(right), line, col);
    }
    
    return left;
}

std::unique_ptr<Expression> Parser::parseFactor() {
    // factor → ( expr ) | id | constant | string_literal
    int line = currentToken().line;
    int col = currentToken().column;
    
    if (match(TokenType::LPAREN)) {
        advance();
        auto expr = parseExpression();
        consume(TokenType::RPAREN, "Expected ')' after expression");
        return expr;
    }
    else if (match(TokenType::ID)) {
        std::string name = currentToken().lexeme;
        advance();
        return std::make_unique<Identifier>(name, line, col);
    }
    else if (match(TokenType::CONSTANT)) {
        int value = 0;
        if (!tryParseInt(currentToken().lexeme, value)) {
            addError("Integer literal is out of range");
            value = 0;
        }
        advance();
        return std::make_unique<Constant>(value, line, col);
    }
    else if (match(TokenType::STRING_LITERAL)) {
        std::string value = currentToken().lexeme;
        advance();
        return std::make_unique<StringLiteral>(value, line, col);
    }
    else {
        addError("Expected expression");
        if (!match(TokenType::END_OF_FILE)) {
            advance();
        }
        return std::make_unique<Constant>(0, line, col);  // Error recovery
    }
}

std::unique_ptr<ParseTreeNode> Parser::buildParseTree() {
    int stepCounter = 0;
    auto makeNonTerminal = [&stepCounter](const std::string& label) {
        return std::make_unique<ParseTreeNode>(label, false, stepCounter++);
    };
    auto makeTerminal = [](const std::string& label) {
        return std::make_unique<ParseTreeNode>(label, true, -1);
    };

    auto root = makeNonTerminal("program");
    if (!astRoot) {
        root->children.push_back(makeTerminal("ε"));
        return root;
    }

    std::function<void(const Expression*, std::vector<const Expression*>&, std::vector<std::string>&)> flattenExpr;
    std::function<void(const Expression*, std::vector<const Expression*>&, std::vector<std::string>&)> flattenTerm;

    flattenExpr = [&](const Expression* expr, std::vector<const Expression*>& terms, std::vector<std::string>& ops) {
        if (auto bin = dynamic_cast<const BinaryExpression*>(expr)) {
            if (bin->op == "+" || bin->op == "-" || bin->op == "<<" || bin->op == ">>") {
                flattenExpr(bin->left.get(), terms, ops);
                ops.push_back(bin->op);
                terms.push_back(bin->right.get());
                return;
            }
        }
        terms.push_back(expr);
    };

    flattenTerm = [&](const Expression* expr, std::vector<const Expression*>& factors, std::vector<std::string>& ops) {
        if (auto bin = dynamic_cast<const BinaryExpression*>(expr)) {
            if (bin->op == "*" || bin->op == "/") {
                flattenTerm(bin->left.get(), factors, ops);
                ops.push_back(bin->op);
                factors.push_back(bin->right.get());
                return;
            }
        }
        factors.push_back(expr);
    };

    std::function<std::unique_ptr<ParseTreeNode>(const Expression*)> buildExpr;
    std::function<std::unique_ptr<ParseTreeNode>(const std::vector<const Expression*>&, const std::vector<std::string>&, size_t)> buildExprTail;
    std::function<std::unique_ptr<ParseTreeNode>(const Expression*)> buildTerm;
    std::function<std::unique_ptr<ParseTreeNode>(const std::vector<const Expression*>&, const std::vector<std::string>&, size_t)> buildTermTail;
    std::function<std::unique_ptr<ParseTreeNode>(const Expression*)> buildFactor;

    buildFactor = [&](const Expression* expr) {
        auto node = makeNonTerminal("factor");
        if (!expr) {
            node->children.push_back(makeTerminal("ε"));
            return node;
        }
        if (auto id = dynamic_cast<const Identifier*>(expr)) {
            auto identNode = makeNonTerminal("identifier");
            identNode->children.push_back(makeTerminal(id->name));
            node->children.push_back(std::move(identNode));
        } else if (auto cnst = dynamic_cast<const Constant*>(expr)) {
            auto constNode = makeNonTerminal("int_constant");
            constNode->children.push_back(makeTerminal(std::to_string(cnst->value)));
            node->children.push_back(std::move(constNode));
        } else if (auto str = dynamic_cast<const StringLiteral*>(expr)) {
            auto strNode = makeNonTerminal("string_literal");
            strNode->children.push_back(makeTerminal("\"" + str->value + "\""));
            node->children.push_back(std::move(strNode));
        } else {
            node->children.push_back(makeTerminal("("));
            node->children.push_back(buildExpr(expr));
            node->children.push_back(makeTerminal(")"));
        }
        return node;
    };

    buildTermTail = [&](const std::vector<const Expression*>& factors, const std::vector<std::string>& ops, size_t index) {
        auto node = makeNonTerminal("term_tail");
        if (index >= ops.size()) {
            node->children.push_back(makeTerminal("ε"));
            return node;
        }
        node->children.push_back(makeTerminal(ops[index]));
        node->children.push_back(buildFactor(factors[index + 1]));
        node->children.push_back(buildTermTail(factors, ops, index + 1));
        return node;
    };

    buildTerm = [&](const Expression* expr) {
        auto node = makeNonTerminal("term");
        std::vector<const Expression*> factors;
        std::vector<std::string> ops;
        flattenTerm(expr, factors, ops);
        node->children.push_back(buildFactor(factors[0]));
        node->children.push_back(buildTermTail(factors, ops, 0));
        return node;
    };

    buildExprTail = [&](const std::vector<const Expression*>& terms, const std::vector<std::string>& ops, size_t index) {
        auto node = makeNonTerminal("expr_tail");
        if (index >= ops.size()) {
            node->children.push_back(makeTerminal("ε"));
            return node;
        }
        node->children.push_back(makeTerminal(ops[index]));
        node->children.push_back(buildTerm(terms[index + 1]));
        node->children.push_back(buildExprTail(terms, ops, index + 1));
        return node;
    };

    buildExpr = [&](const Expression* expr) {
        auto node = makeNonTerminal("expr");
        std::vector<const Expression*> terms;
        std::vector<std::string> ops;
        flattenExpr(expr, terms, ops);
        node->children.push_back(buildTerm(terms[0]));
        node->children.push_back(buildExprTail(terms, ops, 0));
        return node;
    };

    std::function<std::unique_ptr<ParseTreeNode>(const SwitchStatement*)> buildSwitchStmt;

    auto buildCin = [&](const CinStatement* cinStmt) {
        auto cinNode = makeNonTerminal("cin_stmt");
        cinNode->children.push_back(makeTerminal("cin"));
        cinNode->children.push_back(makeTerminal(">>"));
        auto identNode = makeNonTerminal("identifier");
        identNode->children.push_back(makeTerminal(cinStmt->variableName));
        cinNode->children.push_back(std::move(identNode));
        cinNode->children.push_back(makeTerminal(";"));
        return cinNode;
    };

    auto buildCout = [&](const CoutStatement* coutStmt) {
        auto coutNode = makeNonTerminal("cout_stmt");
        coutNode->children.push_back(makeTerminal("cout"));
        coutNode->children.push_back(makeTerminal("<<"));
        coutNode->children.push_back(buildExpr(coutStmt->expression.get()));
        coutNode->children.push_back(makeTerminal(";"));
        return coutNode;
    };

    std::function<std::unique_ptr<ParseTreeNode>(const Statement*)> buildPreStmt;
    std::function<std::unique_ptr<ParseTreeNode>(const Statement*)> buildStmtCore;

    auto buildDecl = [&](const DeclarationStatement* decl) {
        auto declNode = makeNonTerminal("declaration");
        auto typeNode = makeNonTerminal("type_spec");
        typeNode->children.push_back(makeTerminal(decl->variableType));
        declNode->children.push_back(std::move(typeNode));

        auto identNode = makeNonTerminal("identifier");
        identNode->children.push_back(makeTerminal(decl->variableName));
        declNode->children.push_back(std::move(identNode));

        auto initNode = makeNonTerminal("decl_init_opt");
        if (decl->initializer) {
            initNode->children.push_back(makeTerminal("="));
            initNode->children.push_back(buildExpr(decl->initializer.get()));
        } else {
            initNode->children.push_back(makeTerminal("ε"));
        }
        declNode->children.push_back(std::move(initNode));
        declNode->children.push_back(makeTerminal(";"));
        return declNode;
    };

    auto buildAssign = [&](const AssignmentStatement* assign) {
        auto assignNode = makeNonTerminal("assignment");
        auto identNode = makeNonTerminal("identifier");
        identNode->children.push_back(makeTerminal(assign->variableName));
        assignNode->children.push_back(std::move(identNode));
        assignNode->children.push_back(makeTerminal("="));
        assignNode->children.push_back(buildExpr(assign->expression.get()));
        assignNode->children.push_back(makeTerminal(";"));
        return assignNode;
    };

    buildPreStmt = [&](const Statement* stmt) {
        auto node = makeNonTerminal("pre_stmt");
        if (auto decl = dynamic_cast<const DeclarationStatement*>(stmt)) {
            node->children.push_back(buildDecl(decl));
        } else if (auto assign = dynamic_cast<const AssignmentStatement*>(stmt)) {
            node->children.push_back(buildAssign(assign));
        } else if (auto cinStmt = dynamic_cast<const CinStatement*>(stmt)) {
            node->children.push_back(buildCin(cinStmt));
        } else if (auto coutStmt = dynamic_cast<const CoutStatement*>(stmt)) {
            node->children.push_back(buildCout(coutStmt));
        } else {
            // fallback for anything else
            auto dummy = makeNonTerminal("assignment");
            dummy->children.push_back(makeTerminal("error"));
            dummy->children.push_back(makeTerminal(";"));
            node->children.push_back(std::move(dummy));
        }
        return node;
    };

    std::function<std::unique_ptr<ParseTreeNode>(size_t)> buildPreStmtList = [&](size_t idx) {
        auto node = makeNonTerminal("pre_stmt_list");
        if (idx >= astRoot->preSwitchStatements.size()) {
            node->children.push_back(makeTerminal("ε"));
            return node;
        }
        node->children.push_back(buildPreStmt(astRoot->preSwitchStatements[idx].get()));
        node->children.push_back(buildPreStmtList(idx + 1));
        return node;
    };

    auto buildStmt = [&](const Statement* stmt) {
        auto node = makeNonTerminal("stmt");
        if (auto decl = dynamic_cast<const DeclarationStatement*>(stmt)) {
            node->children.push_back(buildDecl(decl));
        } else if (auto assign = dynamic_cast<const AssignmentStatement*>(stmt)) {
            node->children.push_back(buildAssign(assign));
        } else if (auto cinStmt = dynamic_cast<const CinStatement*>(stmt)) {
            node->children.push_back(buildCin(cinStmt));
        } else if (auto coutStmt = dynamic_cast<const CoutStatement*>(stmt)) {
            node->children.push_back(buildCout(coutStmt));
        } else if (auto sw = dynamic_cast<const SwitchStatement*>(stmt)) {
            node->children.push_back(buildSwitchStmt(sw));
        } else {
            auto dummy = makeNonTerminal("assignment");
            dummy->children.push_back(makeTerminal("error"));
            dummy->children.push_back(makeTerminal(";"));
            node->children.push_back(std::move(dummy));
        }
        return node;
    };

    std::function<std::unique_ptr<ParseTreeNode>(const std::vector<std::unique_ptr<Statement>>&, size_t)> buildStmtList;
    buildStmtList = [&](const std::vector<std::unique_ptr<Statement>>& statements, size_t idx) {
        auto node = makeNonTerminal("stmt_list");
        if (idx >= statements.size()) {
            node->children.push_back(makeTerminal("ε"));
            return node;
        }
        node->children.push_back(buildStmt(statements[idx].get()));
        node->children.push_back(buildStmtList(statements, idx + 1));
        return node;
    };

    auto buildCaseClause = [&](const CaseClause* clause) {
        auto caseNode = makeNonTerminal("case_clause");
        caseNode->children.push_back(makeTerminal("case"));
        auto constNode = makeNonTerminal("int_constant");
        constNode->children.push_back(makeTerminal(std::to_string(clause->caseValue)));
        caseNode->children.push_back(std::move(constNode));
        caseNode->children.push_back(makeTerminal(":"));
        caseNode->children.push_back(buildStmtList(clause->statements, 0));
        caseNode->children.push_back(makeTerminal("break"));
        caseNode->children.push_back(makeTerminal(";"));
        return caseNode;
    };

    std::function<std::unique_ptr<ParseTreeNode>(const std::vector<std::unique_ptr<CaseClause>>&, size_t)> buildCaseList;
    buildCaseList = [&](const std::vector<std::unique_ptr<CaseClause>>& cases, size_t idx) {
        auto listNode = makeNonTerminal("case_list");
        if (idx >= cases.size()) {
            listNode->children.push_back(makeTerminal("ε"));
            return listNode;
        }
        listNode->children.push_back(buildCaseClause(cases[idx].get()));
        listNode->children.push_back(buildCaseList(cases, idx + 1));
        return listNode;
    };

    auto buildDefaultClause = [&](const CaseClause* defaultCase) {
        auto defaultNode = makeNonTerminal("default_clause");
        if (!defaultCase) {
            defaultNode->children.push_back(makeTerminal("ε"));
            return defaultNode;
        }
        defaultNode->children.push_back(makeTerminal("default"));
        defaultNode->children.push_back(makeTerminal(":"));
        defaultNode->children.push_back(buildStmtList(defaultCase->statements, 0));
        defaultNode->children.push_back(makeTerminal("break"));
        defaultNode->children.push_back(makeTerminal(";"));
        return defaultNode;
    };

    buildSwitchStmt = [&](const SwitchStatement* switchStmt) {
        auto switchNode = makeNonTerminal("switch_stmt");
        if (!switchStmt) {
            switchNode->children.push_back(makeTerminal("ε"));
            return switchNode;
        }
        switchNode->children.push_back(makeTerminal("switch"));
        switchNode->children.push_back(makeTerminal("("));
        switchNode->children.push_back(buildExpr(switchStmt->condition.get()));
        switchNode->children.push_back(makeTerminal(")"));
        switchNode->children.push_back(makeTerminal("{"));
        switchNode->children.push_back(buildCaseList(switchStmt->cases, 0));
        switchNode->children.push_back(buildDefaultClause(switchStmt->defaultCase.get()));
        switchNode->children.push_back(makeTerminal("}"));
        return switchNode;
    };

    // STRICT CFG MATCHING FOR R1: program -> preamble_opt pre_stmt_list switch_stmt
    auto buildUsingOpt = [&]() {
        auto usingNode = makeNonTerminal("using_opt");
        if (hasUsingNamespaceStd) {
            usingNode->children.push_back(makeTerminal("using"));
            usingNode->children.push_back(makeTerminal("namespace"));
            usingNode->children.push_back(makeTerminal("std"));
            usingNode->children.push_back(makeTerminal(";"));
        } else {
            usingNode->children.push_back(makeTerminal("ε"));
        }
        return usingNode;
    };

    auto buildPreambleOpt = [&]() {
        auto preambleNode = makeNonTerminal("preamble_opt");
        preambleNode->children.push_back(buildUsingOpt());
        return preambleNode;
    };

    root->children.push_back(buildPreambleOpt());
    root->children.push_back(buildPreStmtList(0));
    root->children.push_back(buildSwitchStmt(astRoot->switchStmt.get()));

    return root;
}

// VALIDATION: Ensure code contains no loops
void Parser::validateNoLoops() {
    if (!astRoot) return;
    
    std::function<bool(const Statement*)> hasLoops = [&](const Statement* stmt) -> bool {
        // Since we don't have explicit loop AST nodes (parser rejects them at tokenization),
        // this is defensive check. Loop keywords would appear as ID tokens if somehow passed.
        return false;
    };
    
    for (const auto& stmt : astRoot->preSwitchStatements) {
        if (hasLoops(stmt.get())) {
            addError("ERROR: Loops (for, while, do) are not supported in switch-case compiler");
            return;
        }
    }
    
    if (astRoot->switchStmt) {
        for (const auto& caseClause : astRoot->switchStmt->cases) {
            for (const auto& stmt : caseClause->statements) {
                if (hasLoops(stmt.get())) {
                    addError("ERROR: Loops (for, while, do) are not supported inside switch cases");
                    return;
                }
            }
        }
    }
}

// VALIDATION: Ensure code contains exactly one switch statement
void Parser::validateOnlyOneSwitch() {
    if (!astRoot) {
        addError("ERROR: Code must contain a switch-case statement");
        return;
    }
    
    if (!astRoot->switchStmt) {
        addError("ERROR: Code must contain exactly one switch-case statement in main()");
    }
}

// VALIDATION: Ensure only declarations, assignments, cin/cout, and switch are used
void Parser::validateOnlyValidStatements() {
    if (!astRoot) return;
    
    auto isValidStatement = [](const Statement* stmt) -> bool {
        return dynamic_cast<const DeclarationStatement*>(stmt) ||
               dynamic_cast<const AssignmentStatement*>(stmt) ||
               dynamic_cast<const CinStatement*>(stmt) ||
               dynamic_cast<const CoutStatement*>(stmt) ||
               dynamic_cast<const SwitchStatement*>(stmt);
    };
    
    std::function<bool(const Statement*)> checkValidRecursive = [&](const Statement* stmt) -> bool {
        if (!isValidStatement(stmt)) return false;
        
        if (auto sw = dynamic_cast<const SwitchStatement*>(stmt)) {
            for (const auto& caseClause : sw->cases) {
                for (const auto& innerStmt : caseClause->statements) {
                    if (!checkValidRecursive(innerStmt.get())) return false;
                }
            }
            if (sw->defaultCase) {
                for (const auto& innerStmt : sw->defaultCase->statements) {
                    if (!checkValidRecursive(innerStmt.get())) return false;
                }
            }
        }
        return true;
    };
    
    for (const auto& stmt : astRoot->preSwitchStatements) {
        if (!checkValidRecursive(stmt.get())) {
            addError("ERROR: Inside main(), only declarations, assignments, cin/cout, and switch are allowed");
            return;
        }
    }
    
    if (astRoot->switchStmt) {
        if (!checkValidRecursive(astRoot->switchStmt.get())) {
            addError("ERROR: Inside case blocks, only declarations, assignments, cin/cout, and switch are allowed");
            return;
        }
    }
}

// PHASE 2: VALIDATION - Ensure int main() {...} wrapper is present
void Parser::validateMainWrapper() {
    if (!usesMainWrapper) {
        addError("ERROR: Code must be wrapped in 'int main() { ... return 0; }'");
    }
}

// PHASE 2: VALIDATION - Enforce #include <iostream> directive
void Parser::validateIncludeStatements() {
    if (includeDirectives.empty()) {
        addError("ERROR: Code must contain at least one #include directive (e.g., #include <iostream>)");
        return;
    }
    
    bool hasValidInclude = false;
    for (const auto& inc : includeDirectives) {
        if (!inc.angled) continue;  // Only check angle-bracket includes
        if (inc.headerLeft == "iostream" || inc.headerLeft == "string" ||
            inc.headerLeft == "stdio" || inc.headerLeft == "stdlib") {
            hasValidInclude = true;
            break;
        }
    }
    
    if (!hasValidInclude) {
        addError("ERROR: Code must include <iostream> or <string> header");
    }
}

// Reconstruct source code from parse tree (all terminals in order)
std::string Parser::reconstructSourceFromParseTree() const {
    if (!parseTree) return "";
    
    std::string result;
    
    std::function<void(ParseTreeNode*)> collectTerminals = [&](ParseTreeNode* node) {
        if (!node) return;
        
        if (node->isTerminal) {
            if (result.length() > 0 && result.back() != ' ' && result.back() != '\n' && 
                result.back() != '{' && result.back() != '(' && node->label[0] != ')' && 
                node->label[0] != '}' && node->label[0] != ';' && node->label[0] != ':' &&
                node->label[0] != ',') {
                result += " ";
            }
            result += node->label;
        } else {
            for (const auto& child : node->children) {
                collectTerminals(child.get());
            }
        }
    };
    
    collectTerminals(parseTree.get());
    return result;
}


// ============================================================================
// AST Conversion: Program -> TranslationUnit
// ============================================================================

std::unique_ptr<TranslationUnit> Parser::convertToTranslationUnit(Program* program) {
    if (!program) return nullptr;
    
    auto tu = std::make_unique<TranslationUnit>();
    auto mainFunc = std::make_unique<FunctionDecl>("main", "int");
    auto body = std::make_unique<CompoundStmt>();
    
    // Convert pre-switch statements
    for (auto& stmt : program->preSwitchStatements) {
        auto converted = convertStatement(stmt.get());
        if (converted) {
            body->statements.push_back(std::move(converted));
        }
    }
    
    // Convert switch statement
    if (program->switchStmt) {
        auto switchStmt = std::make_unique<SwitchStatement>(
            convertExpression(program->switchStmt->condition.get()),
            program->switchStmt->line,
            program->switchStmt->column
        );
        
        // Create CompoundStmt for switch body
        auto switchBody = std::make_unique<CompoundStmt>();
        
        // Convert cases to CaseStmt
        for (auto& caseClause : program->switchStmt->cases) {
            auto caseStmt = std::make_unique<CaseStmt>(
                caseClause->caseValue,
                caseClause->line,
                caseClause->column
            );
            
            // Convert statements in case
            for (auto& stmt : caseClause->statements) {
                auto converted = convertStatement(stmt.get());
                if (converted) {
                    caseStmt->statements.push_back(std::move(converted));
                }
            }
            
            // Add break statement
            caseStmt->statements.push_back(std::make_unique<BreakStmt>(
                caseClause->line, caseClause->column
            ));
            caseStmt->hasBreak = true;
            
            switchBody->statements.push_back(std::move(caseStmt));
        }
        
        // Convert default case
        if (program->switchStmt->defaultCase) {
            auto defaultStmt = std::make_unique<CaseStmt>(
                true,
                program->switchStmt->defaultCase->line,
                program->switchStmt->defaultCase->column
            );
            
            for (auto& stmt : program->switchStmt->defaultCase->statements) {
                auto converted = convertStatement(stmt.get());
                if (converted) {
                    defaultStmt->statements.push_back(std::move(converted));
                }
            }
            
            defaultStmt->statements.push_back(std::make_unique<BreakStmt>(
                program->switchStmt->defaultCase->line,
                program->switchStmt->defaultCase->column
            ));
            defaultStmt->hasBreak = true;
            
            switchBody->statements.push_back(std::move(defaultStmt));
        }
        
        switchStmt->body = std::move(switchBody);
        body->statements.push_back(std::move(switchStmt));
    }
    
    // Add return statement
    body->statements.push_back(std::make_unique<ReturnStmt>(
        std::make_unique<IntegerLiteral>(parsedReturnValue)
    ));
    
    mainFunc->body = std::move(body);
    tu->mainFunction = std::move(mainFunc);
    
    return tu;
}

std::unique_ptr<Expression> Parser::convertExpression(Expression* expr) {
    if (!expr) return nullptr;
    
    if (auto* binExpr = dynamic_cast<BinaryExpression*>(expr)) {
        return std::make_unique<BinaryExpression>(
            binExpr->op,
            convertExpression(binExpr->left.get()),
            convertExpression(binExpr->right.get()),
            binExpr->line,
            binExpr->column
        );
    }
    
    if (auto* ident = dynamic_cast<Identifier*>(expr)) {
        return std::make_unique<DeclRefExpr>(
            ident->name,
            ident->annotatedType,
            ident->line,
            ident->column
        );
    }
    
    if (auto* constant = dynamic_cast<Constant*>(expr)) {
        return std::make_unique<IntegerLiteral>(
            constant->value,
            constant->line,
            constant->column
        );
    }
    
    if (auto* strLit = dynamic_cast<StringLiteral*>(expr)) {
        return std::make_unique<StringLiteral>(
            strLit->value,
            strLit->line,
            strLit->column
        );
    }
    
    return nullptr;
}

std::unique_ptr<Statement> Parser::convertStatement(Statement* stmt) {
    if (!stmt) return nullptr;
    
    if (auto* declStmt = dynamic_cast<DeclarationStatement*>(stmt)) {
        auto varDecl = std::make_unique<VarDecl>(
            declStmt->variableType,
            declStmt->variableName,
            convertExpression(declStmt->initializer.get()),
            declStmt->line,
            declStmt->column
        );
        
        return std::make_unique<DeclStmt>(
            std::move(varDecl),
            declStmt->line,
            declStmt->column
        );
    }
    
    if (auto* assignStmt = dynamic_cast<AssignmentStatement*>(stmt)) {
        return std::make_unique<AssignmentStatement>(
            assignStmt->variableName,
            convertExpression(assignStmt->expression.get()),
            assignStmt->line,
            assignStmt->column
        );
    }
    
    if (auto* cinStmt = dynamic_cast<CinStatement*>(stmt)) {
        return std::make_unique<CinStatement>(
            cinStmt->variableName,
            cinStmt->line,
            cinStmt->column
        );
    }
    
    if (auto* coutStmt = dynamic_cast<CoutStatement*>(stmt)) {
        return std::make_unique<CoutStatement>(
            convertExpression(coutStmt->expression.get()),
            coutStmt->line,
            coutStmt->column
        );
    }
    
    return nullptr;
}
