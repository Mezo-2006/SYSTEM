#include "OptimizationPanel.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QFont>
#include <QGroupBox>
#include <QHeaderView>
#include <QSplitter>
#include <QButtonGroup>
#include <QScrollBar>
#include <algorithm>

// ── Construction ──────────────────────────────────────────────────────────────

OptimizationPanel::OptimizationPanel(QWidget* parent) : QWidget(parent) {
    setStyleSheet(
        "OptimizationPanel {"
        "  background: qlineargradient(x1:0, y1:0, x2:1, y2:1,"
        "    stop:0 #0F172A, stop:0.5 #1E1B4B, stop:1 #020617);"
        "}"
        "QGroupBox {"
        "  color: #C4B5FD; font-weight: bold;"
        "  border: 1px solid rgba(139,92,246,0.3);"
        "  border-radius: 8px; margin-top: 12px;"
        "  background: rgba(30,27,75,0.5);"
        "}"
        "QGroupBox::title { subcontrol-origin: margin; left: 12px; padding: 0 6px; }"
        "QCheckBox { color: #E2E8F0; font-size: 12px; padding: 2px 4px; }"
        "QCheckBox::indicator { width: 14px; height: 14px; }"
        "QCheckBox::indicator:checked { background-color: #8B5CF6; border: 1px solid #A78BFA; border-radius: 3px; }"
        "QCheckBox::indicator:unchecked { background-color: #1E293B; border: 1px solid #475569; border-radius: 3px; }"
    );
    setupUI();
}

// ── Accessors ─────────────────────────────────────────────────────────────────

bool OptimizationPanel::isConstantFoldingEnabled() const { return constantFoldingCheckbox && constantFoldingCheckbox->isChecked(); }
bool OptimizationPanel::isDeadCodeEliminationEnabled() const { return deadCodeCheckbox && deadCodeCheckbox->isChecked(); }
bool OptimizationPanel::isCseEnabled() const { return cseCheckbox && cseCheckbox->isChecked(); }
bool OptimizationPanel::isAlgebraicSimplificationEnabled() const { return algebraicCheckbox && algebraicCheckbox->isChecked(); }
bool OptimizationPanel::isCopyPropagationEnabled() const { return copyPropCheckbox && copyPropCheckbox->isChecked(); }
bool OptimizationPanel::isStrengthReductionEnabled() const { return strengthReductionCheckbox && strengthReductionCheckbox->isChecked(); }

// ── UI Setup ──────────────────────────────────────────────────────────────────

void OptimizationPanel::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(8, 6, 8, 6);
    mainLayout->setSpacing(4);

    // ── Compact Controls ──────────────────────────────────────────────────
    QGroupBox* controlGroup = new QGroupBox("Optimization Passes");
    controlGroup->setMaximumHeight(55);
    QHBoxLayout* cl = new QHBoxLayout(controlGroup);
    cl->setContentsMargins(8, 14, 8, 4);
    cl->setSpacing(6);
    constantFoldingCheckbox = new QCheckBox("Constant Folding"); constantFoldingCheckbox->setChecked(true);
    deadCodeCheckbox = new QCheckBox("Dead Code"); deadCodeCheckbox->setChecked(true);
    cseCheckbox = new QCheckBox("CSE"); cseCheckbox->setChecked(true);
    algebraicCheckbox = new QCheckBox("Algebraic"); algebraicCheckbox->setChecked(true);
    copyPropCheckbox = new QCheckBox("Copy Prop"); copyPropCheckbox->setChecked(true);
    strengthReductionCheckbox = new QCheckBox("Strength Red."); strengthReductionCheckbox->setChecked(true);
    cl->addWidget(constantFoldingCheckbox); cl->addWidget(deadCodeCheckbox);
    cl->addWidget(cseCheckbox); cl->addWidget(algebraicCheckbox);
    cl->addWidget(copyPropCheckbox); cl->addWidget(strengthReductionCheckbox);
    cl->addStretch();
    mainLayout->addWidget(controlGroup);

    // ── Summary + View Mode ───────────────────────────────────────────────
    QHBoxLayout* infoRow = new QHBoxLayout();
    summaryLabel = new QLabel("Compile code to see optimizations");
    summaryLabel->setStyleSheet("color:#A78BFA; font-size:11px; font-weight:bold;"
        "background:rgba(139,92,246,0.12); padding:4px 10px;"
        "border:1px solid rgba(139,92,246,0.25); border-radius:5px;");
    showAllRadio = new QRadioButton("Overall"); showAllRadio->setChecked(true);
    showPassRadio = new QRadioButton("Per-Pass");
    showAllRadio->setStyleSheet("QRadioButton{color:#C4B5FD;font-size:10px;}");
    showPassRadio->setStyleSheet("QRadioButton{color:#C4B5FD;font-size:10px;}");
    QButtonGroup* vg = new QButtonGroup(this);
    vg->addButton(showAllRadio); vg->addButton(showPassRadio);
    passSelector = new QComboBox(); passSelector->setEnabled(false);
    passSelector->setMinimumWidth(180); passSelector->setMaximumHeight(24);
    passSelector->setStyleSheet("QComboBox{background:#1E293B;color:#F1F5F9;border:1px solid #334155;"
        "border-radius:4px;padding:2px 8px;font-size:10px;}"
        "QComboBox::drop-down{border:none;}"
        "QComboBox QAbstractItemView{background:#1E293B;color:#F1F5F9;border:1px solid #334155;}");
    infoRow->addWidget(summaryLabel, 1);
    infoRow->addWidget(showAllRadio); infoRow->addWidget(showPassRadio);
    infoRow->addWidget(passSelector);
    mainLayout->addLayout(infoRow);
    connect(showAllRadio, &QRadioButton::toggled, this, &OptimizationPanel::onViewModeChanged);
    connect(showPassRadio, &QRadioButton::toggled, this, &OptimizationPanel::onViewModeChanged);
    connect(passSelector, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &OptimizationPanel::onPassSelected);

    // ── Stats Table (compact) ─────────────────────────────────────────────
    statsTable = new QTableWidget();
    statsTable->setColumnCount(6);
    statsTable->setHorizontalHeaderLabels({"Pass", "Before", "After", "Removed", "Modified", "%"});
    statsTable->horizontalHeader()->setStretchLastSection(true);
    statsTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    for (int i = 1; i < 6; ++i) statsTable->horizontalHeader()->setSectionResizeMode(i, QHeaderView::ResizeToContents);
    statsTable->verticalHeader()->setVisible(false);
    statsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    statsTable->setSelectionMode(QAbstractItemView::NoSelection);
    statsTable->setShowGrid(false); statsTable->setMaximumHeight(110);
    statsTable->verticalHeader()->setDefaultSectionSize(18);
    statsTable->setStyleSheet(
        "QTableWidget{background:#1E293B;color:#F1F5F9;border:1px solid #334155;border-radius:4px;font-family:Consolas;font-size:10px;}"
        "QHeaderView::section{background:#0F172A;color:#94A3B8;padding:3px;font-weight:bold;font-size:9px;border:none;border-bottom:2px solid #8B5CF6;}"
        "QTableWidget::item{padding:2px 6px;border-bottom:1px solid #1E293B;}");
    mainLayout->addWidget(statsTable);

    // ── Impact Bar ────────────────────────────────────────────────────────
    impactBar = new QWidget(); impactBar->setFixedHeight(12);
    impactBar->setStyleSheet("background:transparent;");
    mainLayout->addWidget(impactBar);

    // ── Before/After Headers ──────────────────────────────────────────────
    QHBoxLayout* hdr = new QHBoxLayout();
    QLabel* bL = new QLabel("BEFORE — Original Unoptimized TAC");
    bL->setStyleSheet("color:#F87171;font-weight:bold;font-size:13px;padding:4px 8px;"
        "background:rgba(127,29,29,0.3);border-radius:4px;border-left:3px solid #EF4444;");
    QLabel* aL = new QLabel("AFTER — Optimized TAC");
    aL->setStyleSheet("color:#4ADE80;font-weight:bold;font-size:13px;padding:4px 8px;"
        "background:rgba(20,83,45,0.3);border-radius:4px;border-left:3px solid #22C55E;");
    hdr->addWidget(bL, 1); hdr->addWidget(aL, 1);
    mainLayout->addLayout(hdr);

    // ── Before/After Tables (DOMINANT) ────────────────────────────────────
    QString ts = "QTableWidget{background:#0F172A;color:#F1F5F9;border:1px solid #334155;border-radius:6px;"
        "font-family:Consolas,monospace;font-size:13px;}"
        "QHeaderView::section{background:#0F172A;color:#94A3B8;padding:6px;font-weight:bold;border:none;border-bottom:2px solid #334155;}"
        "QTableWidget::item{padding:6px 10px;border-bottom:1px solid rgba(51,65,85,0.5);}"
        "QScrollBar:vertical{background:#0F172A;width:10px;border-radius:5px;}"
        "QScrollBar::handle:vertical{background:#334155;border-radius:5px;min-height:30px;}"
        "QScrollBar::add-line:vertical,QScrollBar::sub-line:vertical{height:0;}";

    beforeTable = new QTableWidget(); beforeTable->setStyleSheet(ts);
    beforeTable->setColumnCount(2);
    beforeTable->setHorizontalHeaderLabels({"#", "TAC Instruction (Before)"});
    beforeTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
    beforeTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    beforeTable->setColumnWidth(0, 35);
    beforeTable->verticalHeader()->setVisible(false);
    beforeTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    beforeTable->setSelectionMode(QAbstractItemView::NoSelection);
    beforeTable->setShowGrid(false); beforeTable->verticalHeader()->setDefaultSectionSize(26);

    afterTable = new QTableWidget(); afterTable->setStyleSheet(ts);
    afterTable->setColumnCount(2);
    afterTable->setHorizontalHeaderLabels({"#", "TAC Instruction (After)"});
    afterTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
    afterTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    afterTable->setColumnWidth(0, 35);
    afterTable->verticalHeader()->setVisible(false);
    afterTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    afterTable->setSelectionMode(QAbstractItemView::NoSelection);
    afterTable->setShowGrid(false); afterTable->verticalHeader()->setDefaultSectionSize(26);

    connect(beforeTable->verticalScrollBar(), &QScrollBar::valueChanged, afterTable->verticalScrollBar(), &QScrollBar::setValue);
    connect(afterTable->verticalScrollBar(), &QScrollBar::valueChanged, beforeTable->verticalScrollBar(), &QScrollBar::setValue);

    QSplitter* sp = new QSplitter(Qt::Horizontal);
    sp->setStyleSheet("QSplitter::handle{background:#7C3AED;width:3px;border-radius:1px;}");
    sp->addWidget(beforeTable); sp->addWidget(afterTable);
    mainLayout->addWidget(sp, 10);

    // ── Legend ─────────────────────────────────────────────────────────────
    QHBoxLayout* leg = new QHBoxLayout(); leg->setSpacing(6);
    auto chip = [](const QString& t, const QString& bg, const QString& fg) {
        QLabel* l = new QLabel(t);
        l->setStyleSheet(QString("background:%1;color:%2;border-radius:3px;padding:2px 8px;font-size:9px;font-weight:600;").arg(bg,fg));
        return l;
    };
    leg->addWidget(chip("Removed (struck)", "#7F1D1D", "#FCA5A5"));
    leg->addWidget(chip("Changed", "#78350F", "#FDE68A"));
    leg->addWidget(chip("New", "#14532D", "#86EFAC"));
    leg->addWidget(chip("Unchanged", "#1E293B", "#64748B"));
    leg->addStretch();
    mainLayout->addLayout(leg);
}

// ── Data ──────────────────────────────────────────────────────────────────────

void OptimizationPanel::setOptimizationResults(const std::vector<OptimizationResult>& optimizationResults) {
    results = optimizationResults;
    passSelector->blockSignals(true);
    passSelector->clear();
    for (const auto& r : results) passSelector->addItem(QString::fromStdString(r.optimizationName));
    passSelector->blockSignals(false);
    displayResults();
}

void OptimizationPanel::displayResults() {
    updateStatsTable();
    updateImpactBar();
    if (showAllRadio->isChecked()) populateAllPasses();
    else if (!results.empty()) populateSinglePass(passSelector->currentIndex());
}

void OptimizationPanel::onViewModeChanged() { passSelector->setEnabled(showPassRadio->isChecked()); displayResults(); }
void OptimizationPanel::onPassSelected(int idx) { if (showPassRadio->isChecked() && idx >= 0 && idx < (int)results.size()) populateSinglePass(idx); }

// ── Stats ─────────────────────────────────────────────────────────────────────

void OptimizationPanel::updateStatsTable() {
    statsTable->setRowCount(0);
    if (results.empty()) { summaryLabel->setText("No optimizations recorded."); return; }
    int origSize = (int)results.front().before.size();
    int finalSize = (int)results.back().after.size();
    int totRem = 0, totMod = 0;
    for (const auto& r : results) {
        int row = statsTable->rowCount(); statsTable->insertRow(row);
        int b = (int)r.before.size(), a = (int)r.after.size(), rem = b - a, mod = r.modifiedInstructions;
        double pct = b > 0 ? (double)rem / b * 100.0 : 0;
        totRem += rem; totMod += mod;
        auto mk = [](const QString& t, const QColor& c) { auto* i = new QTableWidgetItem(t); i->setForeground(c); i->setTextAlignment(Qt::AlignCenter); i->setFont(QFont("Consolas",9)); return i; };
        auto* nm = new QTableWidgetItem(QString::fromStdString(r.optimizationName)); nm->setForeground(QColor("#C4B5FD")); nm->setFont(QFont("Consolas",9,QFont::Bold));
        statsTable->setItem(row,0,nm);
        statsTable->setItem(row,1,mk(QString::number(b),QColor("#F87171")));
        statsTable->setItem(row,2,mk(QString::number(a),QColor("#4ADE80")));
        statsTable->setItem(row,3,mk(rem>0?QString("-%1").arg(rem):"0",rem>0?QColor("#FBBF24"):QColor("#64748B")));
        statsTable->setItem(row,4,mk(QString::number(mod),mod>0?QColor("#FDE68A"):QColor("#64748B")));
        statsTable->setItem(row,5,mk(pct>0?QString("%1%").arg(pct,0,'f',1):"0%",pct>0?QColor("#4ADE80"):QColor("#64748B")));
    }
    double tp = origSize > 0 ? (double)(origSize - finalSize) / origSize * 100.0 : 0;
    summaryLabel->setText(QString("%1 -> %2 instr | -%3 removed | %4 modified | %5% reduction").arg(origSize).arg(finalSize).arg(totRem).arg(totMod).arg(tp,0,'f',1));
}

void OptimizationPanel::updateImpactBar() {
    if (results.empty()) { impactBar->setStyleSheet("background:#1E293B;border-radius:3px;"); return; }
    int o = (int)results.front().before.size(), f = (int)results.back().after.size();
    if (o <= 0) o = 1;
    double r = (double)f / o;
    impactBar->setStyleSheet(QString("background:qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #4ADE80,stop:%1 #4ADE80,stop:%2 #EF4444,stop:1 #EF4444);border-radius:3px;border:1px solid #334155;")
        .arg(qBound(0.01,r-0.005,0.99)).arg(qBound(0.01,r+0.005,0.99)));
}

// ── Line-by-line display (NO diff algorithm — just show the data plainly) ─────

static void fillTable(QTableWidget* tbl, const std::vector<TACInstruction>& code) {
    tbl->setRowCount((int)code.size());
    for (int i = 0; i < (int)code.size(); ++i) {
        QString text = QString::fromStdString(code[i].toString());
        bool isLabel = (code[i].opcode == TACOpcode::LABEL);
        bool isJump = (code[i].opcode == TACOpcode::GOTO || code[i].opcode == TACOpcode::IF_GOTO || code[i].opcode == TACOpcode::IF_FALSE_GOTO);

        auto* numItem = new QTableWidgetItem(QString::number(i + 1));
        numItem->setTextAlignment(Qt::AlignCenter);
        numItem->setForeground(QColor("#64748B"));
        numItem->setFont(QFont("Consolas", 11));
        tbl->setItem(i, 0, numItem);

        auto* item = new QTableWidgetItem(text);
        item->setFont(QFont("Consolas", 12));
        if (isLabel) item->setForeground(QColor("#FBBF24"));
        else if (isJump) item->setForeground(QColor("#F472B6"));
        else item->setForeground(QColor("#E2E8F0"));
        tbl->setItem(i, 1, item);
    }
}

// ── Mark differences between before and after using LCS alignment ─────────────

void OptimizationPanel::populateBeforeAfter(
    const std::vector<TACInstruction>& before,
    const std::vector<TACInstruction>& after,
    const std::string& /*passName*/) {

    beforeTable->setRowCount(0);
    afterTable->setRowCount(0);

    // Build string vectors
    int m = (int)before.size(), n = (int)after.size();
    std::vector<std::string> bS(m), aS(n);
    for (int i = 0; i < m; ++i) bS[i] = before[i].toString();
    for (int i = 0; i < n; ++i) aS[i] = after[i].toString();

    // LCS to align rows
    std::vector<std::vector<int>> dp(m+1, std::vector<int>(n+1, 0));
    for (int i = 1; i <= m; ++i)
        for (int j = 1; j <= n; ++j)
            dp[i][j] = (bS[i-1] == aS[j-1]) ? dp[i-1][j-1]+1 : std::max(dp[i-1][j], dp[i][j-1]);

    // Backtrack to get aligned pairs: (beforeIdx, afterIdx), -1 means gap
    struct AlignedRow { int bi; int ai; };
    std::vector<AlignedRow> rows;
    int i = m, j = n;
    while (i > 0 && j > 0) {
        if (bS[i-1] == aS[j-1]) { rows.push_back({i-1, j-1}); i--; j--; }
        else if (dp[i-1][j] > dp[i][j-1]) { rows.push_back({i-1, -1}); i--; }
        else { rows.push_back({-1, j-1}); j--; }
    }
    while (i > 0) { rows.push_back({i-1, -1}); i--; }
    while (j > 0) { rows.push_back({-1, j-1}); j--; }
    std::reverse(rows.begin(), rows.end());

    int total = (int)rows.size();
    beforeTable->setRowCount(total);
    afterTable->setRowCount(total);

    for (int r = 0; r < total; ++r) {
        int bi = rows[r].bi;
        int ai = rows[r].ai;

        // ── BEFORE column ─────────────────────────────────────────────
        if (bi >= 0) {
            auto* numB = new QTableWidgetItem(QString::number(bi + 1));
            numB->setTextAlignment(Qt::AlignCenter);
            numB->setFont(QFont("Consolas", 11));

            auto* txtB = new QTableWidgetItem(QString::fromStdString(bS[bi]));
            txtB->setFont(QFont("Consolas", 12));

            if (ai < 0) {
                // This line was REMOVED by optimization
                numB->setForeground(QColor("#FCA5A5"));
                numB->setBackground(QColor("#7F1D1D"));
                txtB->setForeground(QColor("#FCA5A5"));
                txtB->setBackground(QColor("#7F1D1D"));
                QFont f = txtB->font(); f.setStrikeOut(true); txtB->setFont(f);
            } else if (bS[bi] != aS[ai]) {
                // Line was MODIFIED
                numB->setForeground(QColor("#FDE68A"));
                numB->setBackground(QColor("#78350F"));
                txtB->setForeground(QColor("#FDE68A"));
                txtB->setBackground(QColor("#78350F"));
            } else {
                // Unchanged
                numB->setForeground(QColor("#64748B"));
                txtB->setForeground(QColor("#94A3B8"));
            }
            beforeTable->setItem(r, 0, numB);
            beforeTable->setItem(r, 1, txtB);
        } else {
            // Empty row (this line only exists in After)
            auto* e1 = new QTableWidgetItem(""); e1->setBackground(QColor("#0F172A"));
            auto* e2 = new QTableWidgetItem(""); e2->setBackground(QColor("#0F172A"));
            beforeTable->setItem(r, 0, e1);
            beforeTable->setItem(r, 1, e2);
        }

        // ── AFTER column ──────────────────────────────────────────────
        if (ai >= 0) {
            auto* numA = new QTableWidgetItem(QString::number(ai + 1));
            numA->setTextAlignment(Qt::AlignCenter);
            numA->setFont(QFont("Consolas", 11));

            auto* txtA = new QTableWidgetItem(QString::fromStdString(aS[ai]));
            txtA->setFont(QFont("Consolas", 12));

            if (bi < 0) {
                // This line is NEW (added by optimization)
                numA->setForeground(QColor("#86EFAC"));
                numA->setBackground(QColor("#14532D"));
                txtA->setForeground(QColor("#86EFAC"));
                txtA->setBackground(QColor("#14532D"));
                QFont f = txtA->font(); f.setBold(true); txtA->setFont(f);
            } else if (bS[bi] != aS[ai]) {
                // Line was MODIFIED
                numA->setForeground(QColor("#FDE68A"));
                numA->setBackground(QColor("#78350F"));
                txtA->setForeground(QColor("#FDE68A"));
                txtA->setBackground(QColor("#78350F"));
                QFont f = txtA->font(); f.setBold(true); txtA->setFont(f);
            } else {
                // Unchanged
                numA->setForeground(QColor("#64748B"));
                txtA->setForeground(QColor("#94A3B8"));
            }
            afterTable->setItem(r, 0, numA);
            afterTable->setItem(r, 1, txtA);
        } else {
            // Empty row (this line was removed from Before)
            auto* e1 = new QTableWidgetItem(""); e1->setBackground(QColor("#0F172A"));
            auto* e2 = new QTableWidgetItem(""); e2->setBackground(QColor("#0F172A"));
            afterTable->setItem(r, 0, e1);
            afterTable->setItem(r, 1, e2);
        }
    }
}

// ── Show All: Original TAC vs Final Optimized TAC ─────────────────────────────

void OptimizationPanel::populateAllPasses() {
    if (results.empty()) {
        beforeTable->setRowCount(1); afterTable->setRowCount(1);
        auto* b = new QTableWidgetItem("Compile code to see TAC"); b->setForeground(QColor("#64748B"));
        auto* a = new QTableWidgetItem("Compile code to see TAC"); a->setForeground(QColor("#64748B"));
        beforeTable->setItem(0,0,new QTableWidgetItem("")); beforeTable->setItem(0,1,b);
        afterTable->setItem(0,0,new QTableWidgetItem("")); afterTable->setItem(0,1,a);
        return;
    }
    // Before = original unoptimized TAC, After = final fully optimized TAC
    populateBeforeAfter(results.front().before, results.back().after, "All");
}

// ── Show Single Pass ──────────────────────────────────────────────────────────

void OptimizationPanel::populateSinglePass(int index) {
    if (index < 0 || index >= (int)results.size()) return;
    populateBeforeAfter(results[index].before, results[index].after, results[index].optimizationName);
}