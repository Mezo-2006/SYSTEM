import sys

cpp_code = r"""#include "OptimizationPanel.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QFont>
#include <QGroupBox>
#include <QHeaderView>

OptimizationPanel::OptimizationPanel(QWidget* parent) : QWidget(parent) {       
    setStyleSheet(
        "OptimizationPanel { background-color: #0F172A; }"
        "QGroupBox {"
        "  color: #94A3B8; font-weight: bold; border: 1px solid #334155; border-radius: 6px; margin-top: 10px;"
        "}"
        "QGroupBox::title { subcontrol-origin: margin; left: 10px; padding: 0 5px; }"
        "QCheckBox {"
        "  color: #E2E8F0; font-size: 14px; padding: 5px;"
        "}"
        "QCheckBox::indicator { width: 18px; height: 18px; }"
    );
    setupUI();
}

bool OptimizationPanel::isConstantFoldingEnabled() const {
    return constantFoldingCheckbox && constantFoldingCheckbox->isChecked();     
}

bool OptimizationPanel::isDeadCodeEliminationEnabled() const {
    return deadCodeCheckbox && deadCodeCheckbox->isChecked();
}

bool OptimizationPanel::isCseEnabled() const {
    return cseCheckbox && cseCheckbox->isChecked();
}

void OptimizationPanel::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    // Optimization controls
    QGroupBox* controlGroup = new QGroupBox("Optimization Toggles:");
    QHBoxLayout* controlLayout = new QHBoxLayout(controlGroup);
    controlLayout->setContentsMargins(15, 15, 15, 15);

    constantFoldingCheckbox = new QCheckBox("🧮 Constant Folding");
    constantFoldingCheckbox->setChecked(true);
    deadCodeCheckbox = new QCheckBox("🗑️ Dead Code Elimination");
    deadCodeCheckbox->setChecked(true);
    cseCheckbox = new QCheckBox("🔄 Common Subexpression Elimination");
    cseCheckbox->setChecked(true);

    controlLayout->addWidget(constantFoldingCheckbox);
    controlLayout->addWidget(deadCodeCheckbox);
    controlLayout->addWidget(cseCheckbox);
    controlLayout->addStretch();

    mainLayout->addWidget(controlGroup);

    // Before/After display
    QHBoxLayout* compareLayout = new QHBoxLayout();

    QString tableStyle = R"(
        QTableWidget {
            background-color: #1E293B;
            alternate-background-color: #0F172A;
            color: #E2E8F0;
            border: 1px solid #334155;
            border-radius: 6px;
            font-family: 'Consolas', 'Courier New', monospace;
            font-size: 13px;
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

    // Before panel
    QVBoxLayout* beforeLayout = new QVBoxLayout();
    QLabel* bLabel = new QLabel("📋 Before Optimization:");
    bLabel->setStyleSheet("color: #94A3B8; font-weight: bold; font-size: 14px; margin-bottom: 5px;");
    beforeLayout->addWidget(bLabel);
    
    beforeTable = new QTableWidget();
    beforeTable->setStyleSheet(tableStyle);
    beforeTable->setColumnCount(1);
    beforeTable->setHorizontalHeaderLabels({"Instruction"});
    beforeTable->horizontalHeader()->setStretchLastSection(true);
    beforeTable->verticalHeader()->setVisible(false);
    beforeTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    beforeTable->setShowGrid(false);
    beforeLayout->addWidget(beforeTable);

    // After panel
    QVBoxLayout* afterLayout = new QVBoxLayout();
    QLabel* aLabel = new QLabel("✨ After Optimization:");
    aLabel->setStyleSheet("color: #10B981; font-weight: bold; font-size: 14px; margin-bottom: 5px;");
    afterLayout->addWidget(aLabel);
    
    afterTable = new QTableWidget();
    afterTable->setStyleSheet(tableStyle);
    afterTable->setColumnCount(1);
    afterTable->setHorizontalHeaderLabels({"Instruction"});
    afterTable->horizontalHeader()->setStretchLastSection(true);
    afterTable->verticalHeader()->setVisible(false);
    afterTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    afterTable->setShowGrid(false);
    afterLayout->addWidget(afterTable);

    compareLayout->addLayout(beforeLayout);
    compareLayout->addLayout(afterLayout);
    
    mainLayout->addLayout(compareLayout);
}

void OptimizationPanel::setOptimizationResults(
    const std::vector<OptimizationResult>& optimizationResults) {

    results = optimizationResults;
    displayResults();
}

void OptimizationPanel::displayResults() {
    beforeTable->setRowCount(0);
    afterTable->setRowCount(0);

    if (results.empty()) {
        beforeTable->setRowCount(1);
        QTableWidgetItem* emptyBefore = new QTableWidgetItem("No optimizations recorded.");
        emptyBefore->setForeground(QColor("#64748B"));
        beforeTable->setItem(0, 0, emptyBefore);
        
        afterTable->setRowCount(1);
        QTableWidgetItem* emptyAfter = new QTableWidgetItem("Compile code to view optimizations.");
        emptyAfter->setForeground(QColor("#64748B"));
        afterTable->setItem(0, 0, emptyAfter);
        return;
    }

    int bRow = 0;
    int aRow = 0;

    for (const auto& result : results) {
        // Headers for before
        beforeTable->insertRow(bRow);
        QTableWidgetItem* bHeader = new QTableWidgetItem("=== " + QString::fromStdString(result.optimizationName) + " ===");
        bHeader->setForeground(QColor("#FBBF24"));
        QFont f = bHeader->font(); f.setBold(true); bHeader->setFont(f);
        beforeTable->setItem(bRow++, 0, bHeader);
        
        // Headers for after
        afterTable->insertRow(aRow);
        QTableWidgetItem* aHeader = new QTableWidgetItem("=== " + QString::fromStdString(result.optimizationName) + " ===");
        aHeader->setForeground(QColor("#34D399"));
        aHeader->setFont(f);
        afterTable->setItem(aRow++, 0, aHeader);

        for (const auto& inst : result.before) {
            beforeTable->insertRow(bRow);
            QTableWidgetItem* item = new QTableWidgetItem(QString::fromStdString(inst.toString()));
            
            // If we are doing Dead code elimination and it's removed, we'll mark the extra ones in red/strikethrough later if we could match them exactly,
            // For now, let's keep them styled standard before format
            item->setForeground(QColor("#94A3B8"));
            beforeTable->setItem(bRow++, 0, item);
        }
        
        // Add removed instructions spacer
        beforeTable->insertRow(bRow);
        QTableWidgetItem* bSpacer = new QTableWidgetItem(QString("↳ Removed: %1 instructions").arg(result.removedInstructions));
        bSpacer->setForeground(QColor("#F87171"));
        QFont iFont = bSpacer->font(); iFont.setItalic(true); bSpacer->setFont(iFont);
        beforeTable->setItem(bRow++, 0, bSpacer);
        
        beforeTable->insertRow(bRow);
        beforeTable->setItem(bRow++, 0, new QTableWidgetItem(""));

        for (const auto& inst : result.after) {
            afterTable->insertRow(aRow);
            QTableWidgetItem* item = new QTableWidgetItem(QString::fromStdString(inst.toString()));
            item->setForeground(QColor("#E2E8F0"));
            afterTable->setItem(aRow++, 0, item);
        }
        
        // Match lengths to keep diff aligned
        while (aRow < bRow) {
            afterTable->insertRow(aRow);
            afterTable->setItem(aRow++, 0, new QTableWidgetItem(""));
        }
        while (bRow < aRow) {
            beforeTable->insertRow(bRow);
            beforeTable->setItem(bRow++, 0, new QTableWidgetItem(""));
        }
    }
}
"""

with open(r'c:\Users\Mizo\OneDrive - Arab Academy for Science and Technology\Desktop\systems 2\SwitchCaseCompiler\src\gui\OptimizationPanel.cpp', 'w', encoding='utf-8') as f:
    f.write(cpp_code)
