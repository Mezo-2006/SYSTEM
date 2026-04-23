import sys

cpp_code = r"""#include "ThreeColumnView.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QFont>
#include <QGroupBox>
#include <QHeaderView>
#include <QSplitter>

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
    tacLayout->addWidget(tacTable);

    auto* grpAsm = new QGroupBox("⚙️ x86 Assembly", this);
    grpAsm->setStyleSheet(groupStyle);
    auto* asmLayout = new QVBoxLayout(grpAsm);
    asmLayout->setContentsMargins(0, 15, 0, 0);
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
                              const std::vector<TACInstruction>& tac,
                              const std::vector<AssemblyInstruction>& assembly) {
    tacInstructions = tac;
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
    tacTable->setRowCount(tac.size());
    for (size_t i = 0; i < tac.size(); i++) {
        const auto& inst = tac[i];
        
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
"""

with open(r'c:\Users\Mizo\OneDrive - Arab Academy for Science and Technology\Desktop\systems 2\SwitchCaseCompiler\src\gui\ThreeColumnView.cpp', 'w', encoding='utf-8') as f:
    f.write(cpp_code)
