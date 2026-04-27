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
    
    // Add new optimization checkboxes
    algebraicCheckbox = new QCheckBox("🔢 Algebraic Simplification");
    algebraicCheckbox->setChecked(true);
    copyPropCheckbox = new QCheckBox("📋 Copy Propagation");
    copyPropCheckbox->setChecked(true);
    strengthReductionCheckbox = new QCheckBox("💪 Strength Reduction");
    strengthReductionCheckbox->setChecked(true);

    controlLayout->addWidget(constantFoldingCheckbox);
    controlLayout->addWidget(deadCodeCheckbox);
    controlLayout->addWidget(cseCheckbox);
    controlLayout->addWidget(algebraicCheckbox);
    controlLayout->addWidget(copyPropCheckbox);
    controlLayout->addWidget(strengthReductionCheckbox);
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
        // Check if any changes occurred
        bool hasChanges = false;
        if (result.before.size() != result.after.size() || result.removedInstructions > 0 || result.modifiedInstructions > 0) {
            hasChanges = true;
        } else {
            for (size_t i = 0; i < result.before.size(); ++i) {
                if (result.before[i].toString() != result.after[i].toString()) {
                    hasChanges = true;
                    break;
                }
            }
        }

        // Headers for before
        beforeTable->insertRow(bRow);
        QTableWidgetItem* bHeader = new QTableWidgetItem("▼ " + QString::fromStdString(result.optimizationName));
        bHeader->setBackground(QColor("#2d2d30"));
        bHeader->setForeground(QColor("#dcdcaa"));
        QFont f = bHeader->font(); f.setBold(true); bHeader->setFont(f);
        beforeTable->setItem(bRow++, 0, bHeader);
        
        // Headers for after
        afterTable->insertRow(aRow);
        QTableWidgetItem* aHeader = new QTableWidgetItem("▼ " + QString::fromStdString(result.optimizationName));
        aHeader->setBackground(QColor("#2d2d30"));
        aHeader->setForeground(QColor("#4ec9b0"));
        aHeader->setFont(f);
        afterTable->setItem(aRow++, 0, aHeader);

        if (!hasChanges) {
            beforeTable->insertRow(bRow);
            QTableWidgetItem* bOpt = new QTableWidgetItem("✨ Already Optimal (No changes needed)");
            bOpt->setForeground(QColor("#64748B"));
            QFont iFont = bOpt->font(); iFont.setItalic(true); bOpt->setFont(iFont);
            beforeTable->setItem(bRow++, 0, bOpt);
            
            afterTable->insertRow(aRow);
            QTableWidgetItem* aOpt = new QTableWidgetItem("✨ Already Optimal (No changes needed)");
            aOpt->setForeground(QColor("#64748B"));
            aOpt->setFont(iFont);
            afterTable->setItem(aRow++, 0, aOpt);
            
            // Spacer
            beforeTable->insertRow(bRow); beforeTable->setItem(bRow++, 0, new QTableWidgetItem(""));
            afterTable->insertRow(aRow); afterTable->setItem(aRow++, 0, new QTableWidgetItem(""));
            continue;
        }

        // Find which lines were removed
        // Basic diff heuristic for display: check string matching
        std::vector<bool> bRemoved(result.before.size(), true);
        std::vector<bool> aNew(result.after.size(), true);
        
        for (size_t i = 0; i < result.before.size(); ++i) {
            for (size_t j = 0; j < result.after.size(); ++j) {
                if (result.before[i].toString() == result.after[j].toString() && aNew[j]) {
                    bRemoved[i] = false;
                    aNew[j] = false;
                    break;
                }
            }
        }

        for (size_t i = 0; i < result.before.size(); ++i) {
            beforeTable->insertRow(bRow);
            QTableWidgetItem* item = new QTableWidgetItem(QString::fromStdString(result.before[i].toString()));
            if (bRemoved[i]) {
                item->setForeground(QColor("#f48771")); // Redish for removed
                item->setBackground(QColor("#4a1f1f")); // Dark red background
                QFont strike = item->font();
                strike.setStrikeOut(true);
                item->setFont(strike);
            } else {
                item->setForeground(QColor("#d4d4d4"));
            }
            beforeTable->setItem(bRow++, 0, item);
        }
        
        // Add removed instructions spacer
        beforeTable->insertRow(bRow);
        QTableWidgetItem* bSpacer = new QTableWidgetItem(QString("✨ Saved: %1 instructions!").arg(result.removedInstructions));
        bSpacer->setForeground(QColor("#FBBF24")); // Gold color
        bSpacer->setBackground(QColor("#1e1e1e"));
        QFont iFont = bSpacer->font(); iFont.setItalic(true); iFont.setBold(true); bSpacer->setFont(iFont);
        beforeTable->setItem(bRow++, 0, bSpacer);
        
        beforeTable->insertRow(bRow);
        beforeTable->setItem(bRow++, 0, new QTableWidgetItem(""));

        for (size_t j = 0; j < result.after.size(); ++j) {
            afterTable->insertRow(aRow);
            QTableWidgetItem* item = new QTableWidgetItem(QString::fromStdString(result.after[j].toString()));
            if (aNew[j]) {
                item->setForeground(QColor("#34D399")); // Greenish for new/modified
                item->setBackground(QColor("#102a1c")); // Dark green background
                QFont bold = item->font();
                bold.setBold(true);
                item->setFont(bold);
            } else {
                item->setForeground(QColor("#d4d4d4"));
            }
            afterTable->setItem(aRow++, 0, item);
        }
        
        // Match lengths to keep diff aligned
        while (aRow < bRow) {
            afterTable->insertRow(aRow);
            QTableWidgetItem* emptyItem = new QTableWidgetItem("");
            emptyItem->setBackground(QColor("#1e1e1e"));
            afterTable->setItem(aRow++, 0, emptyItem);
        }
        while (bRow < aRow) {
            beforeTable->insertRow(bRow);
            QTableWidgetItem* emptyItem = new QTableWidgetItem("");
            emptyItem->setBackground(QColor("#1e1e1e"));
            beforeTable->setItem(bRow++, 0, emptyItem);
        }
    }
}
bool OptimizationPanel::isAlgebraicSimplificationEnabled() const {
    return algebraicCheckbox && algebraicCheckbox->isChecked();
}

bool OptimizationPanel::isCopyPropagationEnabled() const {
    return copyPropCheckbox && copyPropCheckbox->isChecked();
}

bool OptimizationPanel::isStrengthReductionEnabled() const {
    return strengthReductionCheckbox && strengthReductionCheckbox->isChecked();
}