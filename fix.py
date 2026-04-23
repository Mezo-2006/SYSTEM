import re
import codecs

with open(r'c:\Users\Mizo\OneDrive - Arab Academy for Science and Technology\Desktop\systems 2\SwitchCaseCompiler\src\gui\MainWindow.cpp', 'r', encoding='utf-8') as f:
    text = f.read()

replacement = """void MainWindow::updateTokenTable() {
    tokenTable->setRowCount(0);

    const auto& tokens = lexer->getTokens();
    for (const auto& token : tokens) {
        int row = tokenTable->rowCount();
        tokenTable->insertRow(row);
        
        QTableWidgetItem* lexItem = new QTableWidgetItem(QString::fromStdString(token.lexeme));
        QTableWidgetItem* typeItem = new QTableWidgetItem(QString::fromStdString(token.typeToString()));
        QTableWidgetItem* lineItem = new QTableWidgetItem(QString::number(token.line));
        QTableWidgetItem* colItem = new QTableWidgetItem(QString::number(token.column));

        // Let's add brilliant colors to token types
        QString typeStr = QString::fromStdString(token.typeToString());
        if (typeStr == "Keyword") {
            typeItem->setForeground(QColor("#A78BFA")); // Purple
            typeItem->setFont(QFont("Consolas", 13, QFont::Bold));
        } else if (typeStr == "Identifier") {
            typeItem->setForeground(QColor("#38BDF8")); // Cyan
        } else if (typeStr == "Number") {
            typeItem->setForeground(QColor("#FBBF24")); // Amber
        } else if (typeStr == "Operator") {
            typeItem->setForeground(QColor("#F87171")); // Red
        } else if (typeStr.contains("Symbol")) {
            typeItem->setForeground(QColor("#94A3B8")); // Grey
        }

        lexItem->setTextAlignment(Qt::AlignCenter);
        typeItem->setTextAlignment(Qt::AlignCenter);
        lineItem->setTextAlignment(Qt::AlignCenter);
        colItem->setTextAlignment(Qt::AlignCenter);

        tokenTable->setItem(row, 0, lexItem);
        tokenTable->setItem(row, 1, typeItem);
        tokenTable->setItem(row, 2, lineItem);
        tokenTable->setItem(row, 3, colItem);
    }
}

void MainWindow::updateSymbolTable() {
    symbolTable->setRowCount(0);
    
    const auto& symbols = lexer->getSymbolTable();
    for (const auto& entry : symbols) {
        const auto& sym = entry.second;
        int row = symbolTable->rowCount();
        symbolTable->insertRow(row);
        
        QTableWidgetItem* nameItem = new QTableWidgetItem(QString::fromStdString(sym.name));
        QTableWidgetItem* typeItem = new QTableWidgetItem(QString::fromStdString(sym.type));
        QTableWidgetItem* valItem = new QTableWidgetItem(QString::fromStdString(sym.value));
        QTableWidgetItem* scopeItem = new QTableWidgetItem(QString::number(sym.scope));
        QTableWidgetItem* lineItem = new QTableWidgetItem(QString::number(sym.line));
        QTableWidgetItem* colItem = new QTableWidgetItem(QString::number(sym.column));

        // Let's add brilliant colors
        nameItem->setForeground(QColor("#38BDF8")); // Cyan
        nameItem->setFont(QFont("Consolas", 14, QFont::Bold));

        if (sym.type == "int") {
            typeItem->setForeground(QColor("#34D399")); // Green
        } else if (sym.type == "string") {
            typeItem->setForeground(QColor("#FBBF24")); // Amber
        }

        if (sym.value == "uninitialized") {
            valItem->setForeground(QColor("#F87171")); // Red
            valItem->setFont(QFont("Consolas", 12, QFont::StyleItalic));
        } else {
            valItem->setForeground(QColor("#A78BFA")); // Purple
        }

        nameItem->setTextAlignment(Qt::AlignCenter);
        typeItem->setTextAlignment(Qt::AlignCenter);
        valItem->setTextAlignment(Qt::AlignCenter);
        scopeItem->setTextAlignment(Qt::AlignCenter);
        lineItem->setTextAlignment(Qt::AlignCenter);
        colItem->setTextAlignment(Qt::AlignCenter);

        symbolTable->setItem(row, 0, nameItem);
        symbolTable->setItem(row, 1, typeItem);
        symbolTable->setItem(row, 2, valItem);
        symbolTable->setItem(row, 3, scopeItem);
        symbolTable->setItem(row, 4, lineItem);
        symbolTable->setItem(row, 5, colItem);
    }
}
"""

pattern = r'void MainWindow::updateTokenTable\(\).*?void MainWindow::updateErrorTable\(\)\s*\{'

new_text = re.sub(pattern, replacement + '\nvoid MainWindow::updateErrorTable() {', text, flags=re.DOTALL)

with codecs.open(r'c:\Users\Mizo\OneDrive - Arab Academy for Science and Technology\Desktop\systems 2\SwitchCaseCompiler\src\gui\MainWindow.cpp', 'w', encoding='utf-8') as f:
    f.write(new_text)

print('Done replacing.')
