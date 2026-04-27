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

    // TAC Table Setup
    tacTable->setColumnCount(5);
    tacTable->setHorizontalHeaderLabels({"#", "Opcode", "Result", "Arg 1", "Arg 2"});
    tacTable->verticalHeader()->setVisible(false);
    tacTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tacTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    tacTable->setSelectionMode(QAbstractItemView::SingleSelection);
    tacTable->setShowGrid(false);
    tacTable->setAlternatingRowColors(true);
    
    tacTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    tacTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
    tacTable->setColumnWidth(0, 40);
    tacTable->setColumnWidth(1, 90);
    tacTable->setColumnWidth(2, 80);
    tacTable->setColumnWidth(3, 80);
    tacTable->horizontalHeader()->setStretchLastSection(true);

    tacTable->setStyleSheet(R"(
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
    )");

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
    
    tacTabWidget->addTab(tacTable, "📋 Table View");
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

    // Display TAC in Table
    tacTable->setRowCount(0);
    tacTable->setRowCount(optimizedTac.size());
    for (size_t i = 0; i < optimizedTac.size(); i++) {
        const auto& inst = optimizedTac[i];
        
        QTableWidgetItem* numItem = new QTableWidgetItem(QString::number(i + 1));
        numItem->setTextAlignment(Qt::AlignCenter);
        numItem->setForeground(QColor("#64748B"));
        
        QString opStr = QString::fromStdString(inst.opcodeToString());
        QTableWidgetItem* opItem = new QTableWidgetItem(opStr);
        if (inst.opcode == TACOpcode::LABEL) {
            opItem->setForeground(QColor("#FBBF24")); // Label gold
        } else if (inst.opcode == TACOpcode::GOTO || inst.opcode == TACOpcode::IF_GOTO || inst.opcode == TACOpcode::IF_FALSE_GOTO) {
            opItem->setForeground(QColor("#F472B6")); // Flow control pink
        } else {
            opItem->setForeground(QColor("#C084FC")); // Opcode purple
        }
        
        QTableWidgetItem* resItem = new QTableWidgetItem(QString::fromStdString(inst.result));
        resItem->setForeground(QColor("#60A5FA")); // Result blue
        
        QTableWidgetItem* arg1Item = new QTableWidgetItem(QString::fromStdString(inst.arg1));
        arg1Item->setForeground(QColor("#34D399")); // Arg green
        
        QTableWidgetItem* arg2Item = new QTableWidgetItem(QString::fromStdString(inst.arg2));
        arg2Item->setForeground(QColor("#34D399")); // Arg green
        
        // If it's a LABEL, set it properly
        if (inst.opcode == TACOpcode::LABEL) {
            resItem->setText(QString::fromStdString(inst.result));
            resItem->setForeground(QColor("#FBBF24"));
            
            opItem->setText("LABEL");
        }

        tacTable->setItem(i, 0, numItem);
        tacTable->setItem(i, 1, opItem);
        tacTable->setItem(i, 2, resItem);
        tacTable->setItem(i, 3, arg1Item);
        tacTable->setItem(i, 4, arg2Item);
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
