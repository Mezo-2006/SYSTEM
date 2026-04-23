#include "OptimizationDiff.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QSplitter>
#include <QFont>
#include <algorithm>

static QString tableStyle() {
    return R"(
        QTableWidget{background:#1E293B;alternate-background-color:#111827;
            color:#F1F5F9;border:1px solid #334155;border-radius:8px;
            font-family:Consolas,monospace;font-size:12px;}
        QHeaderView::section{background:#0F172A;color:#94A3B8;padding:7px;
            border:none;border-bottom:2px solid #334155;font-weight:bold;}
        QTableWidget::item{padding:5px 8px;border-bottom:1px solid #1E293B;}
    )";
}

OptimizationDiff::OptimizationDiff(QWidget* parent) : QWidget(parent) {
    setStyleSheet("background:#0F172A;color:#F1F5F9;");

    // ── header ────────────────────────────────────────────────────────────
    QLabel* title = new QLabel("⚡ Optimization Diff — Before vs After");
    title->setStyleSheet(
        "font-size:17px;font-weight:bold;color:#A78BFA;"
        "background:#1E1B4B;padding:10px 14px;border-radius:8px;"
        "border-left:5px solid #7C3AED;");

    // ── pass selector ─────────────────────────────────────────────────────
    QLabel* passLbl = new QLabel("Pass:");
    passLbl->setStyleSheet("color:#94A3B8;font-size:12px;");
    passCombo = new QComboBox();
    passCombo->setStyleSheet(
        "QComboBox{background:#1E293B;color:#F1F5F9;border:1px solid #334155;"
        "border-radius:6px;padding:4px 10px;font-size:12px;}"
        "QComboBox::drop-down{border:none;}"
        "QComboBox QAbstractItemView{background:#1E293B;color:#F1F5F9;border:1px solid #334155;}");
    passCombo->setMinimumWidth(220);

    statsLabel = new QLabel("Compile code to see optimization results");
    statsLabel->setStyleSheet("color:#64748B;font-size:11px;font-style:italic;");

    QHBoxLayout* topRow = new QHBoxLayout();
    topRow->addWidget(passLbl);
    topRow->addWidget(passCombo);
    topRow->addStretch();
    topRow->addWidget(statsLabel);

    // ── column headers ────────────────────────────────────────────────────
    auto makeColHeader = [](const QString& t, const QString& col) {
        QLabel* l = new QLabel(t);
        l->setStyleSheet(QString("color:%1;font-weight:bold;font-size:13px;"
                                 "padding:4px 0;").arg(col));
        return l;
    };

    // ── tables ────────────────────────────────────────────────────────────
    beforeTable = new QTableWidget();
    afterTable  = new QTableWidget();
    for (auto* tbl : {beforeTable, afterTable}) {
        tbl->setColumnCount(1);
        tbl->setHorizontalHeaderLabels({"Instruction"});
        tbl->horizontalHeader()->setStretchLastSection(true);
        tbl->verticalHeader()->setVisible(false);
        tbl->setEditTriggers(QAbstractItemView::NoEditTriggers);
        tbl->setSelectionMode(QAbstractItemView::NoSelection);
        tbl->setShowGrid(false);
        tbl->setAlternatingRowColors(false);
        tbl->setStyleSheet(tableStyle());
    }

    QWidget* beforePane = new QWidget();
    QVBoxLayout* bl = new QVBoxLayout(beforePane);
    bl->setContentsMargins(0,0,0,0);
    bl->addWidget(makeColHeader("📋  Before", "#F87171"));
    bl->addWidget(beforeTable, 1);

    QWidget* afterPane = new QWidget();
    QVBoxLayout* al = new QVBoxLayout(afterPane);
    al->setContentsMargins(0,0,0,0);
    al->addWidget(makeColHeader("✨  After", "#4ADE80"));
    al->addWidget(afterTable, 1);

    QSplitter* split = new QSplitter(Qt::Horizontal);
    split->setStyleSheet("QSplitter::handle{background:#334155;width:2px;}");
    split->addWidget(beforePane);
    split->addWidget(afterPane);

    // ── legend ────────────────────────────────────────────────────────────
    QHBoxLayout* legend = new QHBoxLayout();
    auto chip = [](const QString& t, const QString& bg, const QString& fg) {
        QLabel* l = new QLabel(t);
        l->setStyleSheet(QString("background:%1;color:%2;border-radius:4px;"
                                 "padding:2px 8px;font-size:11px;").arg(bg,fg));
        return l;
    };
    legend->addWidget(chip("  Removed", "#7F1D1D", "#FCA5A5"));
    legend->addWidget(chip("  Modified", "#78350F", "#FDE68A"));
    legend->addWidget(chip("  Unchanged", "#1E293B", "#94A3B8"));
    legend->addStretch();

    // ── root layout ───────────────────────────────────────────────────────
    QVBoxLayout* root = new QVBoxLayout(this);
    root->setContentsMargins(12,12,12,12);
    root->setSpacing(10);
    root->addWidget(title);
    root->addLayout(topRow);
    root->addLayout(legend);
    root->addWidget(split, 1);

    connect(passCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &OptimizationDiff::onPassSelected);
}

void OptimizationDiff::setResults(const std::vector<OptimizationResult>& results) {
    m_results = results;
    passCombo->blockSignals(true);
    passCombo->clear();
    for (const auto& r : results)
        passCombo->addItem(QString::fromStdString(r.optimizationName));
    passCombo->blockSignals(false);

    if (!results.empty()) {
        passCombo->setCurrentIndex(0);
        buildDiff(results[0]);
    }
}

void OptimizationDiff::onPassSelected(int idx) {
    if (idx >= 0 && idx < (int)m_results.size())
        buildDiff(m_results[idx]);
}

void OptimizationDiff::addRow(QTableWidget* tbl, const QString& text,
                               const QColor& bg, const QColor& fg) {
    int row = tbl->rowCount();
    tbl->insertRow(row);
    auto* it = new QTableWidgetItem(text);
    it->setBackground(bg);
    it->setForeground(fg);
    it->setFont(QFont("Consolas", 11));
    tbl->setItem(row, 0, it);
}

void OptimizationDiff::buildDiff(const OptimizationResult& result) {
    beforeTable->setRowCount(0);
    afterTable->setRowCount(0);

    // Build sets for diff
    std::vector<std::string> bStrs, aStrs;
    for (const auto& i : result.before) bStrs.push_back(i.toString());
    for (const auto& i : result.after)  aStrs.push_back(i.toString());

    // Simple LCS-based diff: mark removed (red), added (green), unchanged (normal)
    // For simplicity: iterate both, mark lines not in after as removed
    std::set<std::string> afterSet(aStrs.begin(), aStrs.end());
    std::set<std::string> beforeSet(bStrs.begin(), bStrs.end());

    for (const auto& s : bStrs) {
        bool kept = afterSet.count(s) > 0;
        addRow(beforeTable,
               QString::fromStdString(s),
               kept ? QColor("#1E293B") : QColor("#7F1D1D"),
               kept ? QColor("#94A3B8") : QColor("#FCA5A5"));
    }

    for (const auto& s : aStrs) {
        bool isNew = beforeSet.count(s) == 0;
        addRow(afterTable,
               QString::fromStdString(s),
               isNew ? QColor("#14532D") : QColor("#1E293B"),
               isNew ? QColor("#86EFAC") : QColor("#94A3B8"));
    }

    // Pad shorter table
    while (beforeTable->rowCount() < afterTable->rowCount())
        addRow(beforeTable, "", QColor("#0F172A"), QColor("#0F172A"));
    while (afterTable->rowCount() < beforeTable->rowCount())
        addRow(afterTable, "", QColor("#0F172A"), QColor("#0F172A"));

    int removed  = result.removedInstructions;
    int modified = result.modifiedInstructions;
    statsLabel->setText(QString("Removed: %1  |  Modified: %2  |  Before: %3  →  After: %4 instructions")
                        .arg(removed).arg(modified)
                        .arg(result.before.size()).arg(result.after.size()));
    statsLabel->setStyleSheet("color:#A78BFA;font-size:11px;font-weight:bold;");
}
