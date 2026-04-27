import re
import sys

def main():
    file_path = 'src/core/Parser.cpp'
    with open(file_path, 'r', encoding='utf-8') as f:
        content = f.read()

    start_sig = 'std::unique_ptr<ParseTreeNode> Parser::buildParseTree() {'
    start_idx = content.find(start_sig)
    if start_idx == -1:
        print("Could not find buildParseTree()")
        sys.exit(1)

    # find the end of the function
    open_braces = 0
    end_idx = -1
    for i in range(start_idx, len(content)):
        if content[i] == '{':
            open_braces += 1
        elif content[i] == '}':
            open_braces -= 1
            if open_braces == 0:
                end_idx = i + 1
                break
                
    if end_idx == -1:
        print("Could not find end of buildParseTree()")
        sys.exit(1)

    new_func = """std::unique_ptr<ParseTreeNode> Parser::buildParseTree() {
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
            if (bin->op == "+" || bin->op == "-") {
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
            strNode->children.push_back(makeTerminal("\\"" + str->value + "\\""));
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

    auto buildSwitchStmt = [&](const SwitchStatement* switchStmt) {
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
}"""

    new_content = content[:start_idx] + new_func + content[end_idx:]
    with open(file_path, 'w', encoding='utf-8') as f:
        f.write(new_content)
    print("Successfully replaced buildParseTree()")

if __name__ == '__main__':
    main()
