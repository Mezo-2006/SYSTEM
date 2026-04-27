import re

with open(r'c:\Users\Mizo\OneDrive - Arab Academy for Science and Technology\Desktop\SYSTEM\SwitchCaseCompiler\src\gui\ThreeColumnView.cpp', 'r', encoding='utf-8') as f:
    text = f.read()

pattern = r"// Display TAC in Tables.*?populateTacTable\(tacTable, optimizedTac, true\);"
replacement = r"""// Diff logic for side-by-side TAC tables
    int m = originalTac.size(), n = optimizedTac.size();
    std::vector<std::vector<int>> dp(m + 1, std::vector<int>(n + 1, 0));
    
    for (int i = 1; i <= m; i++) {
        for (int j = 1; j <= n; j++) {
            if (originalTac[i-1].toString() == optimizedTac[j-1].toString()) {
                dp[i][j] = dp[i-1][j-1] + 1;
            } else {
                dp[i][j] = std::max(dp[i-1][j], dp[i][j-1]);
            }
        }
    }
    
    std::vector<int> alignOrig, alignOpt;
    int i = m, j = n;
    
    while (i > 0 && j > 0) {
        if (originalTac[i-1].toString() == optimizedTac[j-1].toString()) {
            alignOrig.push_back(i-1);
            alignOpt.push_back(j-1);
            i--; j--;
        } else if (dp[i-1][j] > dp[i][j-1]) {
            alignOrig.push_back(i-1);
            alignOpt.push_back(-1);
            i--;
        } else {
            alignOrig.push_back(-1);
            alignOpt.push_back(j-1);
            j--;
        }
    }
    while (i > 0) {
        alignOrig.push_back(i-1);
        alignOpt.push_back(-1);
        i--;
    }
    while (j > 0) {
        alignOrig.push_back(-1);
        alignOpt.push_back(j-1);
        j--;
    }
    
    std::reverse(alignOrig.begin(), alignOrig.end());
    std::reverse(alignOpt.begin(), alignOpt.end());

    int totalRows = alignOrig.size();
    originalTacTable->setRowCount(totalRows);
    tacTable->setRowCount(totalRows);

    auto createEmptyRow = [](QTableWidget* table, int row) {
        for (int c = 0; c < 5; ++c) {
            QTableWidgetItem* item = new QTableWidgetItem("");
            item->setBackground(QColor("#1E293B"));
            table->setItem(row, c, item);
        }
    };

    auto fillRow = [](QTableWidget* table, int row, const TACInstruction& inst, bool isOptimized, bool isModified) {
        QTableWidgetItem* numItem = new QTableWidgetItem(QString::number(row + 1));
        numItem->setTextAlignment(Qt::AlignCenter);
        numItem->setForeground(QColor("#64748B"));
        
        QColor bgColor = isOptimized ? QColor("#064E3B") : QColor("#451A03");
        if (isModified) {
            bgColor = isOptimized ? QColor("#059669") : QColor("#DC2626"); // brighter green for new, red for removed
        }
        
        numItem->setBackground(bgColor);
        
        QString opStr = QString::fromStdString(inst.opcodeToString());
        QTableWidgetItem* opItem = new QTableWidgetItem(opStr);
        if (inst.opcode == TACOpcode::LABEL) {
            opStr = "LABEL";
            opItem->setText(opStr);
            opItem->setForeground(QColor("#FBBF24")); 
        } else if (inst.opcode == TACOpcode::GOTO || inst.opcode == TACOpcode::IF_GOTO || inst.opcode == TACOpcode::IF_FALSE_GOTO) {
            opItem->setForeground(QColor("#F472B6")); 
        } else {
            opItem->setForeground(QColor("#C084FC")); 
        }
        
        QTableWidgetItem* resItem = new QTableWidgetItem(QString::fromStdString(inst.result));
        if (inst.opcode == TACOpcode::LABEL) {
            resItem->setForeground(QColor("#FBBF24"));
        } else {
            resItem->setForeground(QColor("#60A5FA")); 
        }
        
        QTableWidgetItem* arg1Item = new QTableWidgetItem(QString::fromStdString(inst.arg1));
        arg1Item->setForeground(QColor("#34D399")); 
        
        QTableWidgetItem* arg2Item = new QTableWidgetItem(QString::fromStdString(inst.arg2));
        arg2Item->setForeground(QColor("#34D399")); 
        
        if (isModified && !isOptimized) {
            // Strike-out removed instructions
            QFont strike = opItem->font();
            strike.setStrikeOut(true);
            opItem->setFont(strike);
            resItem->setFont(strike);
            arg1Item->setFont(strike);
            arg2Item->setFont(strike);
        } else if (isModified && isOptimized) {
            // Bold new instructions
            QFont bold = opItem->font();
            bold.setBold(true);
            opItem->setFont(bold);
            resItem->setFont(bold);
            arg1Item->setFont(bold);
            arg2Item->setFont(bold);
        }

        table->setItem(row, 0, numItem);
        table->setItem(row, 1, opItem);
        table->setItem(row, 2, resItem);
        table->setItem(row, 3, arg1Item);
        table->setItem(row, 4, arg2Item);

        // subtle background color for the row fields if modified
        if (isModified) {
            QColor rowBg = isOptimized ? QColor("#064E3B") : QColor("#4a1f1f"); // dark green or dark red
            for(int c=1; c<5; ++c) {
                if(table->item(row, c)) table->item(row, c)->setBackground(rowBg);
            }
        }
    };

    for (int r = 0; r < totalRows; ++r) {
        int origIdx = alignOrig[r];
        int optIdx = alignOpt[r];
        
        if (origIdx != -1) {
            fillRow(originalTacTable, r, originalTac[origIdx], false, optIdx == -1);
        } else {
            createEmptyRow(originalTacTable, r);
        }
        
        if (optIdx != -1) {
            fillRow(tacTable, r, optimizedTac[optIdx], true, origIdx == -1);
        } else {
            createEmptyRow(tacTable, r);
        }
    }"""

text = re.sub(pattern, replacement, text, flags=re.DOTALL)

with open(r'c:\Users\Mizo\OneDrive - Arab Academy for Science and Technology\Desktop\SYSTEM\SwitchCaseCompiler\src\gui\ThreeColumnView.cpp', 'w', encoding='utf-8') as f:
    f.write(text)
