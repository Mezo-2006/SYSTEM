import os

def insert_after_in_file(path, search, insertion):
    with open(path, 'r', encoding='utf-8') as f:
        content = f.read()
    if search in content and insertion not in content:
        content = content.replace(search, search + insertion)
        with open(path, 'w', encoding='utf-8') as f:
            f.write(content)

def replace_in_file(path, search, replacement):
    with open(path, 'r', encoding='utf-8') as f:
        content = f.read()
    if search in content:
        content = content.replace(search, replacement)
        with open(path, 'w', encoding='utf-8') as f:
            f.write(content)

base_dir = r"c:\Users\Mizo\OneDrive - Arab Academy for Science and Technology\Desktop\systems 2\SwitchCaseCompiler"

# 1. AST.h
ast_h = os.path.join(base_dir, 'src', 'core', 'AST.h')
cin_cout_def = """

// I/O Statements
class CinStatement : public Statement {
public:
    std::string variableName;

    CinStatement(const std::string& varName, int ln = 0, int col = 0)
        : Statement(ln, col), variableName(varName) {}

    void accept(ASTVisitor* visitor) override;
    std::string toString() const override;
};

class CoutStatement : public Statement {
public:
    std::unique_ptr<Expression> expression;

    CoutStatement(std::unique_ptr<Expression> expr, int ln = 0, int col = 0)
        : Statement(ln, col), expression(std::move(expr)) {}

    void accept(ASTVisitor* visitor) override;
    std::string toString() const override;
};
"""
insert_after_in_file(ast_h, '// Case clause', cin_cout_def)
insert_after_in_file(ast_h, 'virtual void visit(DeclarationStatement* node) = 0;', 
                     '\n    virtual void visit(CinStatement* node) = 0;\n    virtual void visit(CoutStatement* node) = 0;')

# 2. AST.cpp
ast_cpp = os.path.join(base_dir, 'src', 'core', 'AST.cpp')
cin_cout_impl = """
// CinStatement
void CinStatement::accept(ASTVisitor* visitor) {
    visitor->visit(this);
}

std::string CinStatement::toString() const {
    return "cin >> " + variableName + ";";
}

// CoutStatement
void CoutStatement::accept(ASTVisitor* visitor) {
    visitor->visit(this);
}

std::string CoutStatement::toString() const {
    return "cout << " + expression->toString() + ";";
}
"""
insert_after_in_file(ast_cpp, '// CaseClause', cin_cout_impl)

# 3. Parser.cpp
parser_cpp = os.path.join(base_dir, 'src', 'core', 'Parser.cpp')

o_cin = """    // Treat cin as an assignment for now (from semantic perspective)
    auto expr = std::make_unique<Identifier>(varName, startLine, startCol);
    return std::make_unique<AssignmentStatement>(varName, std::move(expr), startLine, startCol);"""
n_cin = """    return std::make_unique<CinStatement>(varName, startLine, startCol);"""
replace_in_file(parser_cpp, o_cin, n_cin)

o_cout = """    // Treat cout as a declaration for semantic purposes (no-op)
    return std::make_unique<DeclarationStatement>("void", "__cout__", std::move(expr), startLine, startCol);"""
n_cout = """    return std::make_unique<CoutStatement>(std::move(expr), startLine, startCol);"""
replace_in_file(parser_cpp, o_cout, n_cout)

v_old = """    auto isValidStatement = [](const Statement* stmt) -> bool {
        return dynamic_cast<const DeclarationStatement*>(stmt) ||
               dynamic_cast<const AssignmentStatement*>(stmt);
        // cout/cin are handled during parsing via parseCoutStatement/parseCinStatement
    };"""
v_new = """    auto isValidStatement = [](const Statement* stmt) -> bool {
        return dynamic_cast<const DeclarationStatement*>(stmt) ||
               dynamic_cast<const AssignmentStatement*>(stmt) ||
               dynamic_cast<const CinStatement*>(stmt) ||
               dynamic_cast<const CoutStatement*>(stmt);
    };"""
replace_in_file(parser_cpp, v_old, v_new)

b_old = """        auto unknownNode = makeNonTerminal("statement");"""
b_new = """        if (auto cinStmt = dynamic_cast<const CinStatement*>(stmt)) {
            auto cinNode = makeNonTerminal("cin_stmt");
            cinNode->children.push_back(makeTerminal("cin"));
            cinNode->children.push_back(makeTerminal(">>"));
            cinNode->children.push_back(makeTerminal(cinStmt->variableName));
            cinNode->children.push_back(makeTerminal(";"));
            return cinNode;
        }

        if (auto coutStmt = dynamic_cast<const CoutStatement*>(stmt)) {
            auto coutNode = makeNonTerminal("cout_stmt");
            coutNode->children.push_back(makeTerminal("cout"));
            coutNode->children.push_back(makeTerminal("<<"));
            coutNode->children.push_back(buildExpr(coutStmt->expression.get()));
            coutNode->children.push_back(makeTerminal(";"));
            return coutNode;
        }
        
        auto unknownNode = makeNonTerminal("statement");"""
replace_in_file(parser_cpp, b_old, b_new)

nt_old = '"switch_stmt", "case_list", "case_clause", "default_clause", "stmt_list",'
nt_new = '"switch_stmt", "case_list", "case_clause", "default_clause", "stmt_list",\n        "cin_stmt", "cout_stmt",'
replace_in_file(parser_cpp, nt_old, nt_new)

# 4. SemanticAnalyzer.h / cpp
sema_h = os.path.join(base_dir, 'src', 'core', 'SemanticAnalyzer.h')
insert_after_in_file(sema_h, 'void visit(DeclarationStatement* node) override;', '\n    void visit(CinStatement* node) override;\n    void visit(CoutStatement* node) override;')

sema_cpp = os.path.join(base_dir, 'src', 'core', 'SemanticAnalyzer.cpp')
sema_impl = """
void SemanticAnalyzer::visit(CinStatement* node) {
    if (!symbolTable.isDeclared(node->variableName)) {
        addError("Variable '" + node->variableName + "' is used before it is declared", node->line, node->column);
    } else if (!symbolTable.isInitialized(node->variableName)) {
        symbolTable.markInitialized(node->variableName);
    }
}

void SemanticAnalyzer::visit(CoutStatement* node) {
    if (node->expression) {
        node->expression->accept(this);
    }
}
"""
insert_after_in_file(sema_cpp, 'void SemanticAnalyzer::visit(SwitchStatement* node) {', sema_impl)

# 5. TACGenerator.h / cpp
tac_h = os.path.join(base_dir, 'src', 'core', 'TACGenerator.h')
insert_after_in_file(tac_h, 'void visit(DeclarationStatement* node) override;', '\n    void visit(CinStatement* node) override;\n    void visit(CoutStatement* node) override;')
replace_in_file(tac_h, 'LABEL,       // L1:', 'CIN,         // cin >> x\n    COUT,        // cout << x\n    LABEL,       // L1:')

tac_cpp = os.path.join(base_dir, 'src', 'core', 'TACGenerator.cpp')
tac_impl = """
void TACGenerator::visit(CinStatement* node) {
    emit(TACOpcode::CIN, node->variableName);
}

void TACGenerator::visit(CoutStatement* node) {
    node->expression->accept(this);
    emit(TACOpcode::COUT, currentTemp);
}
"""
insert_after_in_file(tac_cpp, 'void TACGenerator::visit(SwitchStatement* node) {', tac_impl)
replace_in_file(tac_cpp, 'case TACOpcode::CALL:        opStr = "call"; break;', 'case TACOpcode::CIN:         opStr = "in"; break;\n        case TACOpcode::COUT:        opStr = "out"; break;\n        case TACOpcode::CALL:        opStr = "call"; break;')

# 6. CodeGenerator.cpp
code_cpp = os.path.join(base_dir, 'src', 'core', 'CodeGenerator.cpp')
cg_impl = """
        case TACOpcode::CIN: {
            std::string reg = allocateRegister(inst.result);
            emitAsm("IN", reg, "", "Read input into " + inst.result);
            break;
        }
        case TACOpcode::COUT: {
            std::string reg = getRegister(inst.result);
            emitAsm("OUT", reg, "", "Print output");
            freeRegister(inst.result);
            break;
        }"""
insert_after_in_file(code_cpp, 'case TACOpcode::ASSIGN: {', cg_impl)

opt_cpp = os.path.join(base_dir, 'src', 'core', 'Optimizer.cpp')
opt_impl = """
        case TACOpcode::CIN:         return "CIN " + inst.result;
        case TACOpcode::COUT:        return "COUT " + inst.result;"""
insert_after_in_file(opt_cpp, 'case TACOpcode::GOTO:', opt_impl)

print('Patch applied successfully')
