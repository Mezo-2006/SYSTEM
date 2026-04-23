#include "TACSimulator.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QFont>
#include <QScrollBar>
#include <cmath>
#include <stdexcept>

static const QString DARK_BG   = "#0F172A";
static const QString CARD_BG   = "#1E293B";
static const QString BORDER    = "#334155";
static const QString ACCENT    = "#6366F1";   // indigo
static const QString GREEN     = "#22C55E";
static const QString RED       = "#EF4444";
static const QString AMBER     = "#F59E0B";
static const QString TEXT      = "#F1F5F9";
static const QString MUTED     = "#64748B";

TACSimulator::TACSimulator(QWidget* parent) : QWidget(parent) {
    setStyleSheet(QString("background:%1; color:%2;").arg(DARK_BG, TEXT));

    // ── top toolbar ───────────────────────────────────────────────────────
    btnBack  = new QPushButton("◀ Back");
    btnStep  = new QPushButton("Step ▶");
    btnPlay  = new QPushButton("▶ Auto");
    btnReset = new QPushButton("⟳ Reset");
    pcLabel  = new QLabel("PC: 0");
    statusLabel = new QLabel("Load a compiled program to begin simulation");

    for (auto* b : {btnBack, btnStep, btnPlay, btnReset}) {
        b->setFixedHeight(32);
        b->setStyleSheet(QString(
            "QPushButton{background:%1;color:%2;border:1px solid %3;"
            "border-radius:6px;padding:0 14px;font-weight:600;font-size:12px;}"
            "QPushButton:hover{background:%4;}"
            "QPushButton:disabled{opacity:0.4;}")
            .arg(CARD_BG, TEXT, BORDER, ACCENT));
    }
    btnPlay->setStyleSheet(btnPlay->styleSheet().replace(CARD_BG, ACCENT));

    pcLabel->setStyleSheet(QString(
        "background:%1;color:%2;border:1px solid %3;"
        "border-radius:6px;padding:2px 12px;font-family:Consolas;font-weight:bold;font-size:13px;")
        .arg(CARD_BG, AMBER, BORDER));

    statusLabel->setStyleSheet(QString("color:%1;font-size:12px;font-style:italic;").arg(MUTED));

    QLabel* speedLbl = new QLabel("Speed:");
    speedLbl->setStyleSheet(QString("color:%1;font-size:11px;").arg(MUTED));
    speedSlider = new QSlider(Qt::Horizontal);
    speedSlider->setRange(1, 10);
    speedSlider->setValue(5);
    speedSlider->setFixedWidth(90);
    speedSlider->setStyleSheet(
        "QSlider::groove:horizontal{background:#334155;height:4px;border-radius:2px;}"
        "QSlider::handle:horizontal{background:#6366F1;width:14px;height:14px;"
        "margin:-5px 0;border-radius:7px;}"
    );

    QHBoxLayout* toolbar = new QHBoxLayout();
    toolbar->setSpacing(8);
    toolbar->addWidget(btnBack);
    toolbar->addWidget(btnStep);
    toolbar->addWidget(btnPlay);
    toolbar->addWidget(btnReset);
    toolbar->addWidget(pcLabel);
    toolbar->addWidget(speedLbl);
    toolbar->addWidget(speedSlider);
    toolbar->addStretch();
    toolbar->addWidget(statusLabel);

    // ── TAC instruction table ─────────────────────────────────────────────
    tacTable = new QTableWidget();
    tacTable->setColumnCount(5);
    tacTable->setHorizontalHeaderLabels({"#", "Opcode", "Result", "Arg1", "Arg2"});
    tacTable->verticalHeader()->setVisible(false);
    tacTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tacTable->setSelectionMode(QAbstractItemView::NoSelection);
    tacTable->setShowGrid(false);
    tacTable->setAlternatingRowColors(true);
    tacTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
    tacTable->setColumnWidth(0, 36);
    tacTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    tacTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    tacTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    tacTable->horizontalHeader()->setStretchLastSection(true);
    tacTable->setStyleSheet(QString(
        "QTableWidget{background:%1;alternate-background-color:#111827;"
        "color:%2;border:1px solid %3;border-radius:8px;"
        "font-family:Consolas,monospace;font-size:12px;}"
        "QHeaderView::section{background:#0F172A;color:%4;padding:6px;"
        "border:none;border-bottom:2px solid %5;font-weight:bold;}"
        "QTableWidget::item{padding:5px;border-bottom:1px solid #1E293B;}")
        .arg(CARD_BG, TEXT, BORDER, MUTED, ACCENT));

    // ── variable / register state table ──────────────────────────────────
    varTable = new QTableWidget();
    varTable->setColumnCount(2);
    varTable->setHorizontalHeaderLabels({"Variable / Temp", "Value"});
    varTable->verticalHeader()->setVisible(false);
    varTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    varTable->setSelectionMode(QAbstractItemView::NoSelection);
    varTable->setShowGrid(false);
    varTable->horizontalHeader()->setStretchLastSection(true);
    varTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    varTable->setStyleSheet(tacTable->styleSheet());

    // ── execution log ─────────────────────────────────────────────────────
    logView = new QTextEdit();
    logView->setReadOnly(true);
    logView->setFont(QFont("Consolas", 10));
    logView->setStyleSheet(QString(
        "QTextEdit{background:%1;color:%2;border:1px solid %3;border-radius:8px;padding:6px;}")
        .arg(CARD_BG, TEXT, BORDER));
    logView->setMaximumHeight(160);

    // ── labels ────────────────────────────────────────────────────────────
    auto makeHeader = [](const QString& t, const QString& col) {
        QLabel* l = new QLabel(t);
        l->setStyleSheet(QString("color:%1;font-weight:bold;font-size:13px;margin-bottom:4px;").arg(col));
        return l;
    };

    // ── layout ────────────────────────────────────────────────────────────
    QSplitter* split = new QSplitter(Qt::Horizontal);
    split->setStyleSheet("QSplitter::handle{background:#334155;width:2px;}");

    QWidget* leftPane = new QWidget();
    QVBoxLayout* leftL = new QVBoxLayout(leftPane);
    leftL->setContentsMargins(0,0,0,0);
    leftL->addWidget(makeHeader("📋  TAC Instructions", ACCENT));
    leftL->addWidget(tacTable, 1);

    QWidget* rightPane = new QWidget();
    QVBoxLayout* rightL = new QVBoxLayout(rightPane);
    rightL->setContentsMargins(0,0,0,0);
    rightL->addWidget(makeHeader("🗂  Variable State", GREEN));
    rightL->addWidget(varTable, 1);
    rightL->addWidget(makeHeader("📜  Execution Log", AMBER));
    rightL->addWidget(logView);

    split->addWidget(leftPane);
    split->addWidget(rightPane);
    split->setSizes({550, 350});

    QVBoxLayout* root = new QVBoxLayout(this);
    root->setContentsMargins(12, 12, 12, 12);
    root->setSpacing(10);
    root->addLayout(toolbar);
    root->addWidget(split, 1);

    // ── timer ─────────────────────────────────────────────────────────────
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &TACSimulator::tick);

    // ── signals ───────────────────────────────────────────────────────────
    connect(btnStep,  &QPushButton::clicked, this, &TACSimulator::stepForward);
    connect(btnBack,  &QPushButton::clicked, this, &TACSimulator::stepBack);
    connect(btnPlay,  &QPushButton::clicked, this, &TACSimulator::autoPlay);
    connect(btnReset, &QPushButton::clicked, this, &TACSimulator::reset);
    connect(speedSlider, &QSlider::valueChanged, this, &TACSimulator::onSpeedChanged);

    reset();
}

// ─────────────────────────────────────────────────────────────────────────────
void TACSimulator::loadProgram(const std::vector<TACInstruction>& instructions) {
    program = instructions;
    reset();

    tacTable->setRowCount(static_cast<int>(program.size()));
    for (int i = 0; i < static_cast<int>(program.size()); ++i) {
        const auto& inst = program[i];

        auto makeItem = [](const QString& t, const QColor& c = QColor("#F1F5F9")) {
            auto* it = new QTableWidgetItem(t);
            it->setForeground(c);
            it->setTextAlignment(Qt::AlignCenter);
            return it;
        };

        QColor opColor = QColor("#C084FC");
        if (inst.opcode == TACOpcode::LABEL)                                opColor = QColor("#FBBF24");
        else if (inst.opcode == TACOpcode::GOTO ||
                 inst.opcode == TACOpcode::IF_GOTO ||
                 inst.opcode == TACOpcode::IF_FALSE_GOTO)                   opColor = QColor("#F472B6");

        tacTable->setItem(i, 0, makeItem(QString::number(i+1), QColor(MUTED)));
        tacTable->setItem(i, 1, makeItem(QString::fromStdString(inst.opcodeToString()), opColor));
        tacTable->setItem(i, 2, makeItem(QString::fromStdString(inst.result), QColor("#60A5FA")));
        tacTable->setItem(i, 3, makeItem(QString::fromStdString(inst.arg1),   QColor("#34D399")));
        tacTable->setItem(i, 4, makeItem(QString::fromStdString(inst.arg2),   QColor("#34D399")));
    }

    statusLabel->setText(QString("Program loaded: %1 instructions. Press Step ▶ to begin.")
                         .arg(program.size()));
    logEvent("▶ Program loaded — " + QString::number(program.size()) + " instructions", QColor(ACCENT));
    highlightPC();
}

void TACSimulator::reset() {
    timer->stop();
    running = false;
    pc = 0;
    vars.clear();
    history.clear();
    pcHistory.clear();
    btnPlay->setText("▶ Auto");
    refreshUI();
    if (!program.empty()) highlightPC();
}

// ─────────────────────────────────────────────────────────────────────────────
std::string TACSimulator::resolve(const std::string& s) const {
    if (s.empty()) return "";
    auto it = vars.find(s);
    if (it != vars.end()) return it->second;
    return s;   // treat as literal
}

int TACSimulator::resolveInt(const std::string& s) const {
    std::string v = resolve(s);
    try { return std::stoi(v); } catch (...) { return 0; }
}

void TACSimulator::executeStep() {
    if (pc < 0 || pc >= static_cast<int>(program.size())) return;

    // Save snapshot for undo
    history.push_back(vars);
    pcHistory.push_back(pc);

    const TACInstruction& inst = program[pc];
    int nextPc = pc + 1;

    switch (inst.opcode) {
    case TACOpcode::ASSIGN:
        vars[inst.result] = resolve(inst.arg1);
        logEvent(QString("  %1 ← %2  (= %3)")
                 .arg(QString::fromStdString(inst.result))
                 .arg(QString::fromStdString(inst.arg1))
                 .arg(QString::fromStdString(vars[inst.result])), QColor(GREEN));
        break;

    case TACOpcode::ADD: {
        int r = resolveInt(inst.arg1) + resolveInt(inst.arg2);
        vars[inst.result] = std::to_string(r);
        logEvent(QString("  %1 ← %2 + %3 = %4")
                 .arg(QString::fromStdString(inst.result))
                 .arg(QString::fromStdString(inst.arg1))
                 .arg(QString::fromStdString(inst.arg2))
                 .arg(r), QColor(GREEN));
        break;
    }
    case TACOpcode::SUB: {
        int r = resolveInt(inst.arg1) - resolveInt(inst.arg2);
        vars[inst.result] = std::to_string(r);
        logEvent(QString("  %1 ← %2 - %3 = %4")
                 .arg(QString::fromStdString(inst.result))
                 .arg(QString::fromStdString(inst.arg1))
                 .arg(QString::fromStdString(inst.arg2))
                 .arg(r), QColor(GREEN));
        break;
    }
    case TACOpcode::MUL: {
        int r = resolveInt(inst.arg1) * resolveInt(inst.arg2);
        vars[inst.result] = std::to_string(r);
        logEvent(QString("  %1 ← %2 * %3 = %4")
                 .arg(QString::fromStdString(inst.result))
                 .arg(QString::fromStdString(inst.arg1))
                 .arg(QString::fromStdString(inst.arg2))
                 .arg(r), QColor(GREEN));
        break;
    }
    case TACOpcode::DIV: {
        int d = resolveInt(inst.arg2);
        int r = (d != 0) ? resolveInt(inst.arg1) / d : 0;
        vars[inst.result] = std::to_string(r);
        logEvent(QString("  %1 ← %2 / %3 = %4%5")
                 .arg(QString::fromStdString(inst.result))
                 .arg(QString::fromStdString(inst.arg1))
                 .arg(QString::fromStdString(inst.arg2))
                 .arg(r)
                 .arg(d == 0 ? "  ⚠ div/0" : ""), QColor(GREEN));
        break;
    }
    case TACOpcode::EQ: {
        int r = (resolveInt(inst.arg1) == resolveInt(inst.arg2)) ? 1 : 0;
        vars[inst.result] = std::to_string(r);
        logEvent(QString("  %1 ← (%2 == %3) → %4")
                 .arg(QString::fromStdString(inst.result))
                 .arg(QString::fromStdString(inst.arg1))
                 .arg(QString::fromStdString(inst.arg2))
                 .arg(r), QColor(AMBER));
        break;
    }
    case TACOpcode::NEQ: {
        int r = (resolveInt(inst.arg1) != resolveInt(inst.arg2)) ? 1 : 0;
        vars[inst.result] = std::to_string(r);
        logEvent(QString("  %1 ← (%2 != %3) → %4")
                 .arg(QString::fromStdString(inst.result))
                 .arg(QString::fromStdString(inst.arg1))
                 .arg(QString::fromStdString(inst.arg2))
                 .arg(r), QColor(AMBER));
        break;
    }
    case TACOpcode::LT: {
        int r = (resolveInt(inst.arg1) < resolveInt(inst.arg2)) ? 1 : 0;
        vars[inst.result] = std::to_string(r);
        logEvent(QString("  %1 ← (%2 < %3) → %4")
                 .arg(QString::fromStdString(inst.result))
                 .arg(QString::fromStdString(inst.arg1))
                 .arg(QString::fromStdString(inst.arg2))
                 .arg(r), QColor(AMBER));
        break;
    }
    case TACOpcode::GT: {
        int r = (resolveInt(inst.arg1) > resolveInt(inst.arg2)) ? 1 : 0;
        vars[inst.result] = std::to_string(r);
        logEvent(QString("  %1 ← (%2 > %3) → %4")
                 .arg(QString::fromStdString(inst.result))
                 .arg(QString::fromStdString(inst.arg1))
                 .arg(QString::fromStdString(inst.arg2))
                 .arg(r), QColor(AMBER));
        break;
    }
    case TACOpcode::CIN:
        // In simulation, assign a default value of 0 (no real stdin)
        vars[inst.result] = "0";
        logEvent(QString("  📥 cin >> %1  (simulated = 0)")
                 .arg(QString::fromStdString(inst.result)), QColor("#6366F1"));
        break;

    case TACOpcode::COUT:
        logEvent(QString("  📤 cout << %1  (= \"%2\")")
                 .arg(QString::fromStdString(inst.result))
                 .arg(QString::fromStdString(resolve(inst.result))), QColor("#6366F1"));
        break;

    case TACOpcode::LABEL:
        logEvent(QString("  📍 Label: %1").arg(QString::fromStdString(inst.result)), QColor(AMBER));
        break;

    case TACOpcode::GOTO: {
        bool found = false;
        for (int i = 0; i < static_cast<int>(program.size()); ++i) {
            if (program[i].opcode == TACOpcode::LABEL && program[i].result == inst.result) {
                nextPc = i;
                found = true;
                logEvent(QString("  ↪ goto %1 (→ line %2)")
                         .arg(QString::fromStdString(inst.result)).arg(i+1), QColor("#F472B6"));
                break;
            }
        }
        if (!found)
            logEvent(QString("  ⚠ goto %1 — label not found, continuing")
                     .arg(QString::fromStdString(inst.result)), QColor(RED));
        break;
    }
    case TACOpcode::IF_GOTO: {
        int cond = resolveInt(inst.arg1);
        if (cond != 0) {
            bool found = false;
            for (int i = 0; i < static_cast<int>(program.size()); ++i) {
                if (program[i].opcode == TACOpcode::LABEL && program[i].result == inst.result) {
                    nextPc = i; found = true; break;
                }
            }
            logEvent(QString("  ✅ if %1 (%2) → jump to %3%4")
                     .arg(QString::fromStdString(inst.arg1)).arg(cond)
                     .arg(QString::fromStdString(inst.result))
                     .arg(found ? "" : " ⚠ label not found"), QColor(GREEN));
        } else {
            logEvent(QString("  ❌ if %1 (%2) → no jump")
                     .arg(QString::fromStdString(inst.arg1)).arg(cond), QColor(RED));
        }
        break;
    }
    case TACOpcode::IF_FALSE_GOTO: {
        int cond = resolveInt(inst.arg1);
        if (cond == 0) {
            bool found = false;
            for (int i = 0; i < static_cast<int>(program.size()); ++i) {
                if (program[i].opcode == TACOpcode::LABEL && program[i].result == inst.result) {
                    nextPc = i; found = true; break;
                }
            }
            logEvent(QString("  ✅ ifFalse %1 (%2) → jump to %3%4")
                     .arg(QString::fromStdString(inst.arg1)).arg(cond)
                     .arg(QString::fromStdString(inst.result))
                     .arg(found ? "" : " ⚠ label not found"), QColor(GREEN));
        } else {
            logEvent(QString("  ❌ ifFalse %1 (%2) → no jump")
                     .arg(QString::fromStdString(inst.arg1)).arg(cond), QColor(RED));
        }
        break;
    }
    default:
        logEvent(QString("  [%1]").arg(QString::fromStdString(inst.toString())), QColor(MUTED));
        break;
    }

    pc = nextPc;
    refreshUI();
    highlightPC();

    if (pc >= static_cast<int>(program.size())) {
        timer->stop();
        running = false;
        btnPlay->setText("▶ Auto");
        statusLabel->setText("✅ Execution complete");
        logEvent("━━━ Execution finished ━━━", QColor(ACCENT));
    }
}

void TACSimulator::stepForward() {
    if (pc >= static_cast<int>(program.size())) return;
    executeStep();
}

void TACSimulator::stepBack() {
    if (history.empty()) return;
    vars = history.back(); history.pop_back();
    pc   = pcHistory.back(); pcHistory.pop_back();
    refreshUI();
    highlightPC();
    logEvent("◀ Stepped back", QColor(AMBER));
}

void TACSimulator::autoPlay() {
    if (running) {
        timer->stop();
        running = false;
        btnPlay->setText("▶ Auto");
    } else {
        if (pc >= static_cast<int>(program.size())) reset();
        running = true;
        btnPlay->setText("⏸ Pause");
        int ms = 1100 - speedSlider->value() * 100;
        timer->start(ms);
    }
}

void TACSimulator::tick() {
    if (pc >= static_cast<int>(program.size())) {
        timer->stop(); running = false; btnPlay->setText("▶ Auto");
        return;
    }
    executeStep();
}

void TACSimulator::onSpeedChanged(int v) {
    if (running) timer->setInterval(1100 - v * 100);
}

// ─────────────────────────────────────────────────────────────────────────────
void TACSimulator::refreshUI() {
    pcLabel->setText(QString("PC: %1").arg(pc));

    // Rebuild variable table
    varTable->setRowCount(0);
    for (const auto& [name, val] : vars) {
        int row = varTable->rowCount();
        varTable->insertRow(row);
        bool isTemp = (!name.empty() && name[0] == 't');
        QColor nameCol = isTemp ? QColor("#94A3B8") : QColor("#60A5FA");
        QColor valCol  = QColor(GREEN);

        auto* ni = new QTableWidgetItem(QString::fromStdString(name));
        ni->setForeground(nameCol);
        ni->setFont(QFont("Consolas", 11, isTemp ? QFont::Normal : QFont::Bold));

        auto* vi = new QTableWidgetItem(QString::fromStdString(val));
        vi->setForeground(valCol);
        vi->setFont(QFont("Consolas", 11, QFont::Bold));
        vi->setTextAlignment(Qt::AlignCenter);

        varTable->setItem(row, 0, ni);
        varTable->setItem(row, 1, vi);
    }

    if (pc < static_cast<int>(program.size()))
        statusLabel->setText(QString("Executing instruction %1 / %2")
                             .arg(pc + 1).arg(program.size()));
}

void TACSimulator::highlightPC() {
    // Clear all row backgrounds
    for (int i = 0; i < tacTable->rowCount(); ++i) {
        QColor bg = (i % 2 == 0) ? QColor(CARD_BG) : QColor("#111827");
        for (int c = 0; c < tacTable->columnCount(); ++c) {
            if (auto* it = tacTable->item(i, c))
                it->setBackground(bg);
        }
    }
    // Highlight current PC row
    if (pc >= 0 && pc < tacTable->rowCount()) {
        QColor highlight(ACCENT);
        highlight.setAlpha(60);
        for (int c = 0; c < tacTable->columnCount(); ++c) {
            if (auto* it = tacTable->item(pc, c))
                it->setBackground(highlight);
        }
        tacTable->scrollToItem(tacTable->item(pc, 0));
    }
}

void TACSimulator::logEvent(const QString& msg, const QColor& col) {
    logView->append(QString("<span style='color:%1;font-family:Consolas;font-size:11px;'>%2</span>")
                    .arg(col.name(), msg.toHtmlEscaped()));
    logView->verticalScrollBar()->setValue(logView->verticalScrollBar()->maximum());
}
