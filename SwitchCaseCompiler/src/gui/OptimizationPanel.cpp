#include "OptimizationPanel.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QFont>
#include <QGroupBox>
#include <QHeaderView>

OptimizationPanel::OptimizationPanel(QWidget* parent) : QWidget(parent) {
    setStyleSheet(
        "OptimizationPanel { background-color: #1e1e1e; }"
        "QGroupBox {"
        "  color: #d4d4d4; font-weight: bold; border: 1px solid #3c3c3c; border-radius: 4px; margin-top: 10px;"
        "}"
        "QGroupBox::title { subcontrol-origin: margin; left: 10px; padding: 0 5px; }"
        "QCheckBox {"
        "  color: #cccccc; font-size: 13px; padding: 5px;"
        "}"
        "QCheckBox::indicator { width: 14px; height: 14px; }"
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
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(10);

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
            background-color: #252526;
            alternate-background-color: #1e1e1e;
            color: #d4d4d4;
            border: 1px solid #3c3c3c;
            border-radius: 4px;
            font-family: 'Consolas', 'Courier New', monospace;
            font-size: 13px;
        }
        QHeaderView::section {
            background-color: #333333;
            color: #969696;
            padding: 5px;
            border: none;
            border-bottom: 2px solid #569cd6;
            font-weight: bold;
            text-align: left;
        }
        QTableWidget::item {
            padding: 6px;
            border-bottom: 1px solid #3c3c3c;
        }
    )";

    // Before panel
    QVBoxLayout* beforeLayout = new QVBoxLayout();
    QLabel* bLabel = new QLabel("📋 Before Optimization:");
    bLabel->setStyleSheet("color: #d4d4d4; font-weight: bold; font-size: 13px; margin-bottom: 5px;");
    beforeLayout->addWidget(bLabel);
    
    beforeTable = new QTableWidget();
    beforeTable->setStyleSheet(tableStyle);
    beforeTable->setColumnCount(1);
    beforeTable->setHorizontalHeaderLabels({"Instruction"});
    beforeTable->horizontalHeader()->setStretchLastSection(true);
    beforeTable->verticalHeader()->setVisible(false);
    beforeTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    beforeTable->setSelectionMode(QAbstractItemView::NoSelection);
    beforeTable->setShowGrid(false);
    beforeTable->setAlternatingRowColors(true);
    beforeTable->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    beforeTable->setMinimumHeight(300);
    beforeLayout->addWidget(beforeTable, 1);

    // After panel
    QVBoxLayout* afterLayout = new QVBoxLayout();
    QLabel* aLabel = new QLabel("✨ After Optimization:");
    aLabel->setStyleSheet("color: #4ec9b0; font-weight: bold; font-size: 13px; margin-bottom: 5px;");
    afterLayout->addWidget(aLabel);
    
    afterTable = new QTableWidget();
    afterTable->setStyleSheet(tableStyle);
    afterTable->setColumnCount(1);
    afterTable->setHorizontalHeaderLabels({"Instruction"});
    afterTable->horizontalHeader()->setStretchLastSection(true);
    afterTable->verticalHeader()->setVisible(false);
    afterTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    afterTable->setSelectionMode(QAbstractItemView::NoSelection);
    afterTable->setShowGrid(false);
    afterTable->setAlternatingRowColors(true);
    afterTable->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    afterTable->setMinimumHeight(300);
    afterLayout->addWidget(afterTable, 1);

    compareLayout->addLayout(beforeLayout, 1);
    compareLayout->addLayout(afterLayout, 1);
    
    mainLayout->addLayout(compareLayout, 1);
    
    // Add default rows at startup
    beforeTable->setRowCount(1);
    beforeTable->setItem(0, 0, new QTableWidgetItem("Ready."));
    afterTable->setRowCount(1);
    afterTable->setItem(0, 0, new QTableWidgetItem("Ready."));
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
        emptyBefore->setForeground(QColor("#969696"));
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
        bHeader->setForeground(QColor("#dcdcaa"));
        QFont f = bHeader->font(); f.setBold(true); bHeader->setFont(f);
        beforeTable->setItem(bRow++, 0, bHeader);
        
        // Headers for after
        afterTable->insertRow(aRow);
        QTableWidgetItem* aHeader = new QTableWidgetItem("=== " + QString::fromStdString(result.optimizationName) + " ===");
        aHeader->setForeground(QColor("#4ec9b0"));
        aHeader->setFont(f);
        afterTable->setItem(aRow++, 0, aHeader);

        for (const auto& inst : result.before) {
            beforeTable->insertRow(bRow);
            QTableWidgetItem* item = new QTableWidgetItem(QString::fromStdString(inst.toString()));
            item->setForeground(QColor("#d4d4d4"));
            beforeTable->setItem(bRow++, 0, item);
        }
        
        // Add removed instructions spacer
        beforeTable->insertRow(bRow);
        QTableWidgetItem* bSpacer = new QTableWidgetItem(QString("↳ Removed: %1 instructions").arg(result.removedInstructions));
        bSpacer->setForeground(QColor("#f48771"));
        QFont iFont = bSpacer->font(); iFont.setItalic(true); bSpacer->setFont(iFont);
        beforeTable->setItem(bRow++, 0, bSpacer);
        
        beforeTable->insertRow(bRow);
        beforeTable->setItem(bRow++, 0, new QTableWidgetItem(""));

        for (const auto& inst : result.after) {
            afterTable->insertRow(aRow);
            QTableWidgetItem* item = new QTableWidgetItem(QString::fromStdString(inst.toString()));
            item->setForeground(QColor("#d4d4d4"));
            afterTable->setItem(aRow++, 0, item);
        }
        
        // Add vertical spacing
        // ...
        
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
