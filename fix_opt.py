import re

with open(r'c:\Users\Mizo\OneDrive - Arab Academy for Science and Technology\Desktop\SYSTEM\SwitchCaseCompiler\src\gui\OptimizationPanel.cpp', 'r', encoding='utf-8') as f:
    text = f.read()

# Update the style sheet
css_pattern = r"setStyleSheet\([\s\S]*?check\.svg\); }\"\n    \);"
css_replacement = r"""setStyleSheet(
        "OptimizationPanel { background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #111827, stop:1 #0F172A); }"
        "QGroupBox {"
        "  color: #38BDF8; font-weight: bold; border: 1px solid rgba(51, 65, 85, 0.7); border-radius: 8px; margin-top: 15px; background: rgba(30, 41, 59, 0.6);"
        "}"
        "QGroupBox::title { subcontrol-origin: margin; left: 15px; padding: 0 8px; color: #7DD3FC; background-color: transparent; border-radius: 4px; }"
        "QCheckBox {"
        "  color: #E2E8F0; font-size: 13px; padding: 6px; font-weight: bold; background: transparent;"
        "}"
        "QCheckBox:hover { color: #38BDF8; }"
        "QCheckBox::indicator { width: 16px; height: 16px; border-radius: 4px; border: 2px solid #64748B; background: rgba(15, 23, 42, 0.8); }"
        "QCheckBox::indicator:checked { background-color: #3B82F6; border-color: #3B82F6; image: url(:/icons/check.svg); }"
    );"""

text = re.sub(css_pattern, css_replacement, text)

# Update Headers
header_pattern = r"// Before panel[\s\S]*?CompareLayout->addLayout\(afterLayout, 1\);"
header_replacement = r"""// Before panel
    QVBoxLayout* beforeLayout = new QVBoxLayout();
    QLabel* bLabel = new QLabel("📋 Before Optimization");
    bLabel->setStyleSheet("color: white; font-weight: bold; font-size: 15px; margin-bottom: 8px; background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #991b1b, stop:1 #7f1d1d); padding: 8px 12px; border-radius: 6px; border-left: 4px solid #ef4444; letter-spacing: 1px;");
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
    QLabel* aLabel = new QLabel("✨ After Optimization");
    aLabel->setStyleSheet("color: white; font-weight: bold; font-size: 15px; margin-bottom: 8px; background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #064e3b, stop:1 #065f46); padding: 8px 12px; border-radius: 6px; border-left: 4px solid #10b981; letter-spacing: 1px;");
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
    compareLayout->addLayout(afterLayout, 1);"""

text = re.sub(r'// Before panel.*?compareLayout->addLayout\(afterLayout, 1\);', header_replacement, text, flags=re.DOTALL)

with open(r'c:\Users\Mizo\OneDrive - Arab Academy for Science and Technology\Desktop\SYSTEM\SwitchCaseCompiler\src\gui\OptimizationPanel.cpp', 'w', encoding='utf-8') as f:
    f.write(text)
