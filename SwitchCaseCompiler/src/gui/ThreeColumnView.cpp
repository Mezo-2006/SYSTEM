#include "ThreeColumnView.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QFont>
#include <QGroupBox>
#include <QHeaderView>
#include <QSplitter>
#include <QFileDialog>
#include <QTextStream>
#include <QMessageBox>

ThreeColumnView::ThreeColumnView(QWidget* parent) : QWidget(parent) {
    setupUI();
}

void ThreeColumnView::setupUI() {
    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(10, 10, 10, 10);
    layout->setSpacing(15);

    sourceColumn = new QTextEdit(this);
    originalTacTable = new QTableWidget(this);
    tacTable = new QTableWidget(this);
    assemblyColumn = new QTextEdit(this);

    // Set modern styling for text views
    QString textStyle = R"(
        QTextEdit {
            background-color: #1E293B;
            color: #E2E8F0;
            border: 1px solid #334155;
            border-radius: 6px;
            font-family: 'Consolas', 'Courier New', monospace;
            font-size: 13px;
            padding: 8px;
            line-height: 1.4;
        }
        QTextEdit:focus {
            border: 1px solid #3B82F6;
        }
    )";

    sourceColumn->setReadOnly(true);
    sourceColumn->setStyleSheet(textStyle);
    
    assemblyColumn->setReadOnly(true);
    assemblyColumn->setStyleSheet(textStyle);

    // TAC Tables Setup
    auto setupTable = [](QTableWidget* table) {
        table->setColumnCount(5);
        table->setHorizontalHeaderLabels({"#", "Opcode", "Result", "Arg 1", "Arg 2"});
        table->verticalHeader()->setVisible(false);
        table->setEditTriggers(QAbstractItemView::NoEditTriggers);
        table->setSelectionBehavior(QAbstractItemView::SelectRows);
        table->setSelectionMode(QAbstractItemView::SingleSelection);
        table->setShowGrid(false);
        table->setAlternatingRowColors(true);
        table->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
        table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
        table->setColumnWidth(0, 40);
        table->setColumnWidth(1, 90);
        table->setColumnWidth(2, 80);
        table->setColumnWidth(3, 80);
        table->horizontalHeader()->setStretchLastSection(true);
    };

    setupTable(originalTacTable);
    setupTable(tacTable);

    QString tableStyle = R"(
        QTableWidget {
            background-color: #1E293B;
            alternate-background-color: #0F172A;
            color: #E2E8F0;
            border: 1px solid #334155;
            border-radius: 6px;
            font-family: 'Consolas', 'Courier New', monospace;
            font-size: 13px;
            selection-background-color: #334155;
            selection-color: #ffffff;
        }
        QHeaderView::section {
            background-color: #0F172A;
            color: #94A3B8;
            padding: 5px;
            border: none;
            border-bottom: 2px solid #334155;
            font-weight: bold;
            text-align: left;
        }
        QTableWidget::item {
            padding: 4px;
            border-bottom: 1px solid #1E293B;
        }
    )";
    originalTacTable->setStyleSheet(tableStyle);
    tacTable->setStyleSheet(tableStyle);

    // Create styled group boxes
    QString groupStyle = R"(
        QGroupBox {
            font-weight: bold;
            color: #94A3B8;
            font-size: 12px;
            border: none;
            margin-top: 1ex;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            subcontrol-position: top left;
            padding: 0 5px;
            background-color: transparent;
        }
    )";

    auto* grpSource = new QGroupBox("📄 Source Code", this);
    grpSource->setStyleSheet(groupStyle);
    auto* srcLayout = new QVBoxLayout(grpSource);
    srcLayout->setContentsMargins(0, 15, 0, 0);
    srcLayout->addWidget(sourceColumn);

    auto* grpTac = new QGroupBox("🔄 Three-Address Code (IR)", this);
    grpTac->setStyleSheet(groupStyle);
    auto* tacLayout = new QVBoxLayout(grpTac);
    tacLayout->setContentsMargins(0, 15, 0, 0);
    
    auto* exportReportBtn = new QPushButton("Export Full Report", grpTac);
    exportReportBtn->setStyleSheet("background-color: #3B82F6; color: white; border-radius: 4px; padding: 4px 8px;");
    exportReportBtn->setCursor(Qt::PointingHandCursor);
    connect(exportReportBtn, &QPushButton::clicked, this, &ThreeColumnView::onExportReport);
    
    animPlayBtn = new QPushButton("▶ Play Animation", grpTac);
    animPlayBtn->setStyleSheet("background-color: #8B5CF6; color: white; border-radius: 4px; padding: 4px 8px; font-weight: bold;");
    animPlayBtn->setCursor(Qt::PointingHandCursor);
    
    auto* tacHeaderLayout = new QHBoxLayout();
    tacHeaderLayout->addWidget(animPlayBtn);
    tacHeaderLayout->addStretch();
    tacHeaderLayout->addWidget(exportReportBtn);
    tacLayout->addLayout(tacHeaderLayout);
    
    tacTabWidget = new QTabWidget(this);
    tacTabWidget->setStyleSheet(R"(
        QTabWidget::pane { border: 1px solid #334155; border-radius: 4px; }
        QTabBar::tab { background: #1E293B; color: #94A3B8; padding: 8px 12px; margin-right: 2px; border-top-left-radius: 4px; border-top-right-radius: 4px; }
        QTabBar::tab:selected { background: #334155; color: #E2E8F0; font-weight: bold; border-bottom: 2px solid #3B82F6; }
        QTabBar::tab:hover:!selected { background: #26354B; }
    )");
    
    irFlowDiagram = new IRFlowDiagram(this);
    connect(animPlayBtn, &QPushButton::clicked, irFlowDiagram, &IRFlowDiagram::startExecution);
    
    QSplitter* tacSplitter = new QSplitter(Qt::Horizontal, this);
    tacSplitter->setStyleSheet("QSplitter::handle { background-color: #334155; width: 2px; }");
    
    QWidget* originalContainer = new QWidget(this);
    QVBoxLayout* origLayout = new QVBoxLayout(originalContainer);
    origLayout->setContentsMargins(0, 0, 0, 0);
    QLabel* origLabel = new QLabel("📋 Before Optimization", this);
    origLabel->setStyleSheet("color: #F87171; font-weight: bold; margin-bottom: 4px;");
    origLayout->addWidget(origLabel);
    origLayout->addWidget(originalTacTable);
    
    QWidget* optimizedContainer = new QWidget(this);
    QVBoxLayout* optLayout = new QVBoxLayout(optimizedContainer);
    optLayout->setContentsMargins(0, 0, 0, 0);
    QLabel* optLabel = new QLabel("✨ After Optimization", this);
    optLabel->setStyleSheet("color: #4ADE80; font-weight: bold; margin-bottom: 4px;");
    optLayout->addWidget(optLabel);
    optLayout->addWidget(tacTable);
    
    tacSplitter->addWidget(originalContainer);
    tacSplitter->addWidget(optimizedContainer);
    
    tacTabWidget->addTab(tacSplitter, "📋 Table View");
    tacTabWidget->addTab(irFlowDiagram, "🔀 Interactive Flow");
    
    tacLayout->addWidget(tacTabWidget);

    auto* grpAsm = new QGroupBox("⚙️ x86 Assembly", this);
    grpAsm->setStyleSheet(groupStyle);
    auto* asmLayout = new QVBoxLayout(grpAsm);
    asmLayout->setContentsMargins(0, 15, 0, 0);
    
    auto* exportAsmBtn = new QPushButton("Export .asm", grpAsm);
    exportAsmBtn->setStyleSheet("background-color: #10B981; color: white; border-radius: 4px; padding: 4px 8px;");
    exportAsmBtn->setCursor(Qt::PointingHandCursor);
    connect(exportAsmBtn, &QPushButton::clicked, this, &ThreeColumnView::onExportAsm);
    
    auto* asmHeaderLayout = new QHBoxLayout();
    asmHeaderLayout->addStretch();
    asmHeaderLayout->addWidget(exportAsmBtn);
    asmLayout->addLayout(asmHeaderLayout);
    asmLayout->addWidget(assemblyColumn);

    auto* splitter = new QSplitter(Qt::Horizontal, this);
    splitter->setStyleSheet(R"(
        QSplitter::handle {
            background-color: #334155;
            width: 2px;
        }
        QSplitter::handle:hover {
            background-color: #64748B;
        }
    )");

    splitter->addWidget(grpSource);
    splitter->addWidget(grpTac);
    splitter->addWidget(grpAsm);

    splitter->setSizes(QList<int>() << 250 << 450 << 300);

    layout->addWidget(splitter);
}

void ThreeColumnView::setData(const std::string& source,
                              const std::vector<TACInstruction>& originalTac,
                              const std::vector<TACInstruction>& optimizedTac,
                              const std::vector<AssemblyInstruction>& assembly) {
    tacInstructions = optimizedTac;
    assemblyInstructions = assembly;

    // Display source
    QString sourceHtml;
    QStringList sourceLines = QString::fromStdString(source).split('\n');       
    for (int i = 0; i < sourceLines.size(); ++i) {
        if (!sourceLines[i].trimmed().isEmpty()) {
            sourceHtml += QString("<span style='color: #64748B; margin-right: 10px;'>%1</span> <span style='color: #E2E8F0;'>%2</span><br>")
                .arg(i + 1, 3, 10, QChar('0'))
                .arg(sourceLines[i].toHtmlEscaped());
        }
    }
    sourceColumn->setHtml(sourceHtml);

    // Diff logic for side-by-side TAC tables
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
    }
    
    // Pass to IR Flow Diagram (Show before/after optimization diff visually)
    irFlowDiagram->setOptimizedTAC(originalTac, optimizedTac);
    irFlowDiagram->setShowOptimized(true);

    // Display Assembly
    QString asmHtml;
    for (const auto& inst : assembly) {
        QString line = QString::fromStdString(inst.toString());
        if (line.endsWith(":")) {
            asmHtml += QString("<br><span style='color: #FBBF24; font-weight: bold;'>%1</span><br>").arg(line.toHtmlEscaped());
        } else if (line.startsWith("\t")) {
            QStringList parts = line.trimmed().split(" ", Qt::SkipEmptyParts);  
            if (parts.size() > 0) {
                QString op = parts[0];
                parts.removeFirst();
                asmHtml += QString("&nbsp;&nbsp;&nbsp;&nbsp;<span style='color: #C084FC;'>%1</span> <span style='color: #A7F3D0;'>%2</span><br>")
                    .arg(op.toHtmlEscaped())
                    .arg(parts.join(" ").toHtmlEscaped());
            } else {
                 asmHtml += QString("&nbsp;&nbsp;&nbsp;&nbsp;<span style='color: #E2E8F0;'>%1</span><br>").arg(line.trimmed().toHtmlEscaped());
            }
        } else {
            asmHtml += QString("<span style='color: #E2E8F0;'>%1</span><br>").arg(line.toHtmlEscaped());
        }
    }
    assemblyColumn->setHtml(asmHtml);
}

void ThreeColumnView::updateDisplay() {
}

void ThreeColumnView::onExportAsm() {
    if (assemblyInstructions.empty()) {
        QMessageBox::warning(this, "Export Failed", "No assembly code available to export.");
        return;
    }

    QString fileName = QFileDialog::getSaveFileName(this, "Export Assembly", "output.asm", "Assembly Files (*.asm);;All Files (*)");
    if (fileName.isEmpty()) return;

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Error", "Could not save file!");
        return;
    }

    QTextStream out(&file);
    out << ";========================================================\n";
    out << "; Generated by Switch-Case Compiler\n";
    out << ";========================================================\n\n";
    out << ".data\n";
    out << "    ; (Variables and constants would go here)\n\n";
    out << ".text\n";
    out << "global _main\n";
    out << "_main:\n";
    for (const auto& inst : assemblyInstructions) {
        out << QString::fromStdString(inst.toString()) << "\n";
    }
    
    file.close();
    QMessageBox::information(this, "Export Success", "Assembly code exported successfully to:\n" + fileName);
}

void ThreeColumnView::onExportReport() {
    if (tacInstructions.empty() || assemblyInstructions.empty()) {
        QMessageBox::warning(this, "Export Failed", "Compilation must be complete to export the report.");
        return;
    }

    QString fileName = QFileDialog::getSaveFileName(this, "Export Compilation Report", "CompilationReport.txt", "Text Files (*.txt);;All Files (*)");
    if (fileName.isEmpty()) return;

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Error", "Could not save file!");
        return;
    }

    QTextStream out(&file);
    out << "========================================================\n";
    out << "               COMPILATION REPORT                       \n";
    out << "========================================================\n\n";
    
    out << "--- SOURCE CODE ---\n";
    out << sourceColumn->toPlainText() << "\n\n";
    
    out << "--- THREE ADDRESS CODE (TAC) ---\n";
    for (size_t i = 0; i < tacInstructions.size(); i++) {
        const auto& inst = tacInstructions[i];
        out << QString("%1: ").arg(i + 1, 3, 10, QChar('0')) << QString::fromStdString(inst.toString()) << "\n";
    }
    out << "\n";
    
    out << "--- TARGET ASSEMBLY (x86) ---\n";
    for (const auto& inst : assemblyInstructions) {
        out << QString::fromStdString(inst.toString()) << "\n";
    }
    
    file.close();
    QMessageBox::information(this, "Export Success", "Full compilation report exported successfully to:\n" + fileName);
}
