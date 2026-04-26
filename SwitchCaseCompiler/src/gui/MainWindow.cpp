#include "MainWindow.h"
#include "DerivationViewer.h"
#include "ParseTreeView.h"
#include "ASTView.h"
#include "ThreeColumnView.h"
#include "OptimizationPanel.h"
#include "SyntaxHighlighter.h"
#include "CodeEditor.h"
#include "IRFlowDiagram.h"
#include "PerformanceMetrics.h"
#include "CompileReport.h"
#include "TokenChart.h"
#include "PipelineDiagram.h"
#include "LifetimeDiagram.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QFont>
#include <QFrame>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QEasingCurve>
#include <QTimer>
#include <QIcon>
#include <QElapsedTimer>
#include <QApplication>
#include <QScreen>
#include <QStyleFactory>
#include <QFile>
#include <QToolTip>
#include <QCursor>
#include <QTextCursor>
#include <QComboBox>
#include <QScrollArea>
#include <QObject>
#include <algorithm>
#include <set>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    // Initialize compiler components
    lexer = std::make_unique<Lexer>();
    parser = std::make_unique<Parser>();
    semanticAnalyzer = std::make_unique<SemanticAnalyzer>();
    tacGenerator = std::make_unique<TACGenerator>();
    optimizer = std::make_unique<Optimizer>();
    codeGenerator = std::make_unique<CodeGenerator>();
    targetOptimizer = std::make_unique<TargetCodeOptimizer>();
    
    // Initialize real-time parsing timer
    parseTimer = new QTimer(this);
    parseTimer->setInterval(500);  // 500ms debounce
    connect(parseTimer, &QTimer::timeout, this, &MainWindow::performRealTimeParse);
    
    setupUI();
    connectSignals();

    setWindowIcon(QIcon(":/icons/cpp_icon.svg"));

    // MODERN PROFESSIONAL STYLESHEET
    // Load from QSS resource file or apply inline
    QApplication::setStyle(QStyleFactory::create("Fusion"));
    
    // Load professional stylesheet
    QFile styleFile(":/styles/modern_style.qss");
    if (styleFile.open(QFile::ReadOnly)) {
        QString style = QLatin1String(styleFile.readAll());
        qApp->setStyleSheet(style);
        styleFile.close();
    } else {
        // Fallback to embedded stylesheet
        qApp->setStyleSheet(
            "QMainWindow { background-color: #0f1117; color: #e8eaf2; }"
            "QTabWidget::pane { border: 1px solid rgba(255,255,255,0.06); background-color: #1a1d27; border-radius: 8px; }"
            "QTabBar::tab { background-color: #1a1d27; color: #9199b8; padding: 12px 20px; border-bottom: 2px solid transparent; }"
            "QTabBar::tab:selected { background-color: #22263a; color: #7c9ef5; border-bottom-color: #7c9ef5; }"
            "QTextEdit, QPlainTextEdit { background-color: #22263a; color: #e8eaf2; border: 1px solid rgba(255,255,255,0.06); border-radius: 6px; padding: 8px 12px; }"
            "QTextEdit:focus { border: 1px solid #7c9ef5; }"
            "QPushButton { background-color: #a78bfa; color: #ffffff; border: none; border-radius: 6px; padding: 8px 16px; font-weight: 600; }"
            "QPushButton:hover { background-color: #c4b5fd; }"
            "QPushButton:pressed { background-color: #9370db; }"
            "QTableWidget { background-color: #1a1d27; color: #e8eaf2; gridline-color: rgba(255,255,255,0.06); border: 1px solid rgba(255,255,255,0.06); border-radius: 6px; }"
            "QHeaderView::section { background-color: #22263a; color: #9199b8; padding: 6px 12px; font-weight: 600; }"
            "QLabel { color: #e8eaf2; }"
        );
    }

    auto* fadeEffect = new QGraphicsOpacityEffect(tabWidget);
    tabWidget->setGraphicsEffect(fadeEffect);
    fadeEffect->setOpacity(0.0);

    QTimer::singleShot(40, this, [this, fadeEffect]() {
        auto* fadeIn = new QPropertyAnimation(fadeEffect, "opacity", this);
        fadeIn->setDuration(700);
        fadeIn->setStartValue(0.0);
        fadeIn->setEndValue(1.0);
        fadeIn->setEasingCurve(QEasingCurve::OutCubic);
        fadeIn->start(QAbstractAnimation::DeleteWhenStopped);
    });
    
    setWindowTitle("Switch-Case Compiler - Educational Tool");
    const QRect available = QGuiApplication::primaryScreen()->availableGeometry();
    const int targetWidth = std::max(1100, static_cast<int>(available.width() * 0.84));
    const int targetHeight = std::max(700, static_cast<int>(available.height() * 0.82));
    resize(std::min(targetWidth, available.width()), std::min(targetHeight, available.height()));
    setMinimumSize(1024, 680);
}

MainWindow::~MainWindow() {}

void MainWindow::setupUI() {
    tabWidget = new QTabWidget(this);
    setCentralWidget(tabWidget);
    
    setupEditorTab();
    setupDerivationTab();
    setupParseTreeTab();
    setupASTTab();
    setupCFGTab();
    setupTokenTab();
    setupSymbolTab();
    setupTraceTab();
    setupOptimizationTab();
    setupTokenInsightsTab();
    setupLifetimeTab();
    setupPipelineTab();
    setupErrorTab();
    setupConsoleTab();
    setupPerformanceTab();
}

void MainWindow::setupEditorTab() {
    editorTab = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(editorTab);

    auto* heroFrame = new QFrame();
    heroFrame->setStyleSheet(
        "QFrame {"
        "  background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #264653, stop:1 #2A9D8F);"
        "  border: 1px solid #84A98C;"
        "  border-radius: 10px;"
        "}"
    );
    QVBoxLayout* heroLayout = new QVBoxLayout(heroFrame);
    auto* title = new QLabel("Switch-Case Lab");
    title->setStyleSheet("QLabel { color: #F1FAEE; font-family: 'Bahnschrift'; font-size: 18pt; font-weight: 700; }");
    auto* subtitle = new QLabel("Educational subset compiler (not full C++): int declarations, assignments, arithmetic, and switch-case.");
    subtitle->setStyleSheet("QLabel { color: #E9F5DB; font-family: 'Bahnschrift'; font-size: 10pt; }");
    heroLayout->addWidget(title);
    heroLayout->addWidget(subtitle);
    
    // Source editor with line numbers and error underlining
    sourceEditor = new CodeEditor();
    sourceEditor->setPlaceholderText(
        "Enter simplified switch-case code...\n\n"
        "Example:\n"
        "#include <iostream>\n"
        "\n"
        "int main() {\n"
        "  int day = 1;\n"
        "  int result = 0;\n"
        "  switch (day) {\n"
        "    case 1: result = 7; break;\n"
        "    case 2: result = day + 10; break;\n"
        "    default: result = 0; break;\n"
        "  }\n"
        "  return 0;\n"
        "}"
    );
    
    // Error list panel
    editorErrorTable = new QTableWidget();
    editorErrorTable->setColumnCount(4);
    editorErrorTable->setHorizontalHeaderLabels({"Line", "Column", "Severity", "Message"});
    editorErrorTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    editorErrorTable->setMaximumHeight(100);
    editorErrorTable->setStyleSheet(
        "QTableWidget {"
        "  background-color: #1A1D27;"
        "  color: #e8eaf2;"
        "  alternate-background-color: #22263a;"
        "}"
        "QTableWidget::item { padding: 4px; }"
        "QHeaderView::section { background-color: #22263a; color: #9199b8; padding: 6px; }"
    );

    compileButton = new QPushButton("Compile Pipeline");
    compileButton->setStyleSheet(
        "QPushButton {"
        "   background-color: #8A2BE2;"
        "   color: #FFFFFF;"
        "   padding: 12px 24px;"
        "   font-size: 13px;"
        "   font-weight: bold;"
        "   border: none;"
        "   border-radius: 10px;"
        "}"
        "QPushButton:hover {"
        "   background-color: #9932CC;"
        "}"
        "QPushButton:pressed {"
        "   background-color: #9400D3;"
        "}"
    );

    QPushButton* quickSimpleButton = new QPushButton("Insert Starter");
    QPushButton* quickNestedButton = new QPushButton("Insert Nested Expr");
    quickSimpleButton->setStyleSheet("QPushButton { background-color: #E9C46A; color: #1A1A1A; }");
    quickNestedButton->setStyleSheet("QPushButton { background-color: #8AB17D; color: #1A1A1A; }");

    connect(quickSimpleButton, &QPushButton::clicked, this, [this]() {
        sourceEditor->setPlainText(
            "#include <iostream>\n"
            "using namespace std;\n\n"
            "int main() {\n"
            "  int x = 1;\n"
            "  int y = 0;\n"
            "  switch (x) {\n"
            "    case 1: y = 2 + 3; break;\n"
            "    case 2: y = 7 * 4; break;\n"
            "    default: y = 0; break;\n"
            "  }\n"
            "  return 0;\n"
            "}\n"
        );
    });

    connect(quickNestedButton, &QPushButton::clicked, this, [this]() {
        sourceEditor->setPlainText(
            "#include <iostream>\n"
            "using namespace std;\n\n"
            "int main() {\n"
            "  int z = 0;\n"
            "  switch (1) {\n"
            "    case 1: z = (2 + 3) * (4 - 1); break;\n"
            "    case 2: z = (8 / 2) + (5 * 3); break;\n"
            "    default: z = 42; break;\n"
            "  }\n"
            "  return 0;\n"
            "}\n"
        );
    });

    // PHASE 1: Code Examples Dropdown
    QComboBox* examplesCombo = new QComboBox();
    examplesCombo->addItem(QIcon(":/icons/code.svg"), "Load Example...");
    examplesCombo->addItem("Simple Switch");
    examplesCombo->addItem("Arithmetic Ops");
    examplesCombo->addItem("Multiple Cases");
    examplesCombo->addItem("Nested Expr");
    examplesCombo->addItem("With I/O");
    examplesCombo->setMaximumWidth(160);
    examplesCombo->setStyleSheet(
        "QComboBox { background-color: #2A3F5F; color: #E0E1DD; padding: 6px; "
        "  border: 1px solid #3A506B; border-radius: 4px; } "
        "QComboBox::drop-down { border: none; }"
    );

    auto loadExample = [this, examplesCombo]() {
        int idx = examplesCombo->currentIndex();
        if (idx <= 0) return;
        
        QString code;
        switch (idx) {
            case 1:  // Simple
                code = "#include <iostream>\nusing namespace std;\n\nint main() {\n"
                       "    int x = 1;\n    switch (x) {\n"
                       "        case 1: x = 10; break;\n"
                       "        case 2: x = 20; break;\n"
                       "        default: x = 0; break;\n"
                       "    }\n    return 0;\n}\n";
                break;
            case 2:  // Arithmetic
                code = "#include <iostream>\nusing namespace std;\n\nint main() {\n"
                       "    int a = 5, b = 3;\n"
                       "    int result = a + b * 2;\n    switch (result) {\n"
                       "        case 11: result = 100; break;\n"
                       "        default: result = 0; break;\n"
                       "    }\n    return 0;\n}\n";
                break;
            case 3:  // Multiple
                code = "#include <iostream>\nusing namespace std;\n\nint main() {\n"
                       "    int m = 3;\n    switch (m) {\n"
                       "        case 1: m = 31; break;\n"
                       "        case 2: m = 28; break;\n"
                       "        case 3: m = 31; break;\n"
                       "        default: m = 0; break;\n"
                       "    }\n    return 0;\n}\n";
                break;
            case 4:  // Nested
                code = "#include <iostream>\nusing namespace std;\n\nint main() {\n"
                       "    int x = 5, y = 3;\n"
                       "    int z = (x + y) * (x - y);\n    switch (z) {\n"
                       "        case 16: z = (2 + 3) * (4 - 1); break;\n"
                       "        default: z = 0; break;\n"
                       "    }\n    return 0;\n}\n";
                break;
            case 5:  // I/O
                code = "#include <iostream>\nusing namespace std;\n\nint main() {\n"
                       "    int x = 0;\n    switch (x) {\n"
                       "        case 1: x = 100; break;\n"
                       "        case 2: x = 200; break;\n"
                       "        default: x = 999; break;\n"
                       "    }\n    return 0;\n}\n";
                break;
        }
        sourceEditor->setPlainText(code);
        examplesCombo->setCurrentIndex(0);
    };
    
    connect(examplesCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), 
            this, loadExample);

    // Clear button
    QPushButton* clearButton = new QPushButton("Clear Editor");
    clearButton->setStyleSheet("QPushButton { background-color: #E76F51; color: #FFFFFF; }");
    connect(clearButton, &QPushButton::clicked, this, [this]() {
        sourceEditor->clear();
    });

    // About Button
    auto aboutBtn = new QPushButton(QIcon(":/icons/info.svg"), "");
    aboutBtn->setIconSize(QSize(24, 24));
    aboutBtn->setFlat(true);
    aboutBtn->setToolTip("About this application");
    aboutBtn->setCursor(Qt::PointingHandCursor);
    aboutBtn->setStyleSheet("QPushButton { border: none; padding: 4px; }");
    
    connect(aboutBtn, &QPushButton::clicked, this, [this]() {
        QMessageBox msg(this);
        msg.setWindowTitle("About Switch-Case Compiler");
        msg.setIcon(QMessageBox::Information);
        msg.setText(
            "<h3>🔧 Switch-Case Compiler v1.0</h3>"
            "<p><b>LL(1) Educational Compiler</b> for C++ switch-case subset</p>"
            "<hr>"
            "<p><b>✅ Features:</b> #include, using, int main, real-time parsing, "
            "parse tree viz, derivation steps, auto-complete, dark/light theme</p>"
            "<p><b>❌ No:</b> loops (for/while), full C++, function defs</p>"
        );
        msg.exec();
    });

    
    statusLabel = new QLabel("Ready");
    statusLabel->setStyleSheet(
        "QLabel {"
        "   padding: 8px 12px;"
        "   background-color: #1C2541;"
        "   color: #E0E1DD;"
        "   border: 1px solid #3A506B;"
        "   border-radius: 8px;"
        "   font-size: 12px;"
        "}"
    );

    validityLabel = new QLabel("✓ Valid Syntax");
    validityLabel->setStyleSheet(
        "QLabel {"
        "   padding: 8px 12px;"
        "   background-color: #1F3B2D;"
        "   color: #CFEEDB;"
        "   border: 1px solid #2A9D8F;"
        "   border-radius: 8px;"
        "   font-size: 12px;"
        "   font-weight: 600;"
        "}"
    );
    validityLabel->hide();

    phaseProgressBar = new QProgressBar();
    phaseProgressBar->setRange(0, 7);
    phaseProgressBar->setValue(0);
    phaseProgressBar->setTextVisible(true);
    phaseProgressBar->setFormat("Phase %v / %m");
    phaseProgressBar->setStyleSheet(
        "QProgressBar {"
        "   border: 1px solid #3A506B;"
        "   border-radius: 8px;"
        "   background-color: #1C2541;"
        "   color: #E0E1DD;"
        "   text-align: center;"
        "   padding: 2px;"
        "}"
        "QProgressBar::chunk {"
        "   border-radius: 6px;"
        "   background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #2A9D8F, stop:1 #E9C46A);"
        "}"
    );

    phaseTimelineFrame = new QFrame();
    phaseTimelineFrame->setStyleSheet(
        "QFrame {"
        "  background-color: rgba(12, 32, 58, 180);"
        "  border: 1px solid #3A506B;"
        "  border-radius: 10px;"
        "}"
    );

    QHBoxLayout* phaseTimelineLayout = new QHBoxLayout(phaseTimelineFrame);
    phaseTimelineLayout->setContentsMargins(8, 8, 8, 8);
    phaseTimelineLayout->setSpacing(6);

    const QStringList phaseShortNames = {"Lex", "Parse", "Semantic", "TAC", "Optimize", "Codegen", "TargetOpt"};
    for (int i = 0; i < 7; ++i) {
        QWidget* phaseCard = new QWidget();
        QVBoxLayout* phaseCardLayout = new QVBoxLayout(phaseCard);
        phaseCardLayout->setContentsMargins(4, 2, 4, 2);
        phaseCardLayout->setSpacing(2);

        phaseBadges[i] = new QLabel(QString("%1 %2").arg(i + 1).arg(phaseShortNames[i]));
        phaseBadges[i]->setAlignment(Qt::AlignCenter);
        phaseBadges[i]->setStyleSheet(
            "QLabel {"
            "  background-color: #1F2A44;"
            "  color: #B8C4D6;"
            "  border: 1px solid #3B4A67;"
            "  border-radius: 10px;"
            "  padding: 4px 8px;"
            "  font-family: 'Bahnschrift';"
            "  font-size: 9pt;"
            "  font-weight: 600;"
            "}"
        );

        phaseDurations[i] = new QLabel("-- ms");
        phaseDurations[i]->setAlignment(Qt::AlignCenter);
        phaseDurations[i]->setStyleSheet("QLabel { color: #9FB3C8; font-size: 8pt; }");

        phaseCardLayout->addWidget(phaseBadges[i]);
        phaseCardLayout->addWidget(phaseDurations[i]);
        phaseTimelineLayout->addWidget(phaseCard);
    }

    QHBoxLayout* actionRow = new QHBoxLayout();
    actionRow->addWidget(new QLabel("<b>Code Snippets:</b>"));
    actionRow->addWidget(quickSimpleButton);
    actionRow->addWidget(quickNestedButton);
    actionRow->addWidget(examplesCombo);
    actionRow->addWidget(clearButton);
    actionRow->addStretch();
    actionRow->addWidget(aboutBtn);
    actionRow->addWidget(compileButton);

    layout->addWidget(heroFrame);

    QSplitter* editorSplitter = new QSplitter(Qt::Vertical);
    editorSplitter->setOpaqueResize(true);

    QWidget* topHalf = new QWidget();
    QVBoxLayout* topHalfLayout = new QVBoxLayout(topHalf);
    topHalfLayout->setContentsMargins(0, 0, 0, 0);
    topHalfLayout->addWidget(new QLabel("<b>Source Code Editor (Line Numbers + Syntax Highlighting)</b>"));
    topHalfLayout->addWidget(sourceEditor, 1);

    QWidget* bottomHalf = new QWidget();
    QVBoxLayout* bottomHalfLayout = new QVBoxLayout(bottomHalf);
    bottomHalfLayout->setContentsMargins(0, 0, 0, 0);
    bottomHalfLayout->addWidget(new QLabel("<b>Errors</b>"));
    bottomHalfLayout->addWidget(editorErrorTable, 1);

    editorSplitter->addWidget(topHalf);
    editorSplitter->addWidget(bottomHalf);
    // Give editor 75% space, errors 25%
    editorSplitter->setStretchFactor(0, 3);
    editorSplitter->setStretchFactor(1, 1);

    layout->addWidget(editorSplitter, 1);

    layout->addLayout(actionRow);
    QHBoxLayout* statusLayout = new QHBoxLayout();
    statusLayout->addWidget(statusLabel);
    statusLayout->addWidget(validityLabel);
    statusLayout->addStretch();
    layout->addLayout(statusLayout);
    layout->addWidget(phaseProgressBar);
    layout->addWidget(phaseTimelineFrame);
    
    tabWidget->addTab(editorTab, "📝 Source Editor");
}

void MainWindow::setupDerivationTab() {
    derivationTab = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(derivationTab);
    
    derivationViewer = new DerivationViewer();
    layout->addWidget(derivationViewer);
    
    tabWidget->addTab(derivationTab, "⭐ Derivation Steps");
}

void MainWindow::setupParseTreeTab() {
    parseTreeTab = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(parseTreeTab);

    QHBoxLayout* toolbar = new QHBoxLayout();
    QPushButton* zoomOutBtn = new QPushButton("−");
    QPushButton* zoomInBtn = new QPushButton("+");
    QPushButton* resetViewBtn = new QPushButton("Reset View");
    QLabel* zoomLabel = new QLabel("Zoom: 100%");
    zoomOutBtn->setFixedWidth(32);
    zoomInBtn->setFixedWidth(32);
    toolbar->addWidget(new QLabel("Tree Controls:"));
    toolbar->addWidget(zoomOutBtn);
    toolbar->addWidget(zoomInBtn);
    toolbar->addWidget(resetViewBtn);
    toolbar->addWidget(zoomLabel);
    toolbar->addStretch();
     
    parseTreeView = new ParseTreeView();
    connect(zoomInBtn, &QPushButton::clicked, this, [this, zoomLabel]() {
        parseTreeView->zoomIn();
        zoomLabel->setText(QString("Zoom: %1%").arg(parseTreeView->zoomPercent()));
    });
    connect(zoomOutBtn, &QPushButton::clicked, this, [this, zoomLabel]() {
        parseTreeView->zoomOut();
        zoomLabel->setText(QString("Zoom: %1%").arg(parseTreeView->zoomPercent()));
    });
    connect(resetViewBtn, &QPushButton::clicked, this, [this, zoomLabel]() {
        parseTreeView->resetView();
        zoomLabel->setText(QString("Zoom: %1%").arg(parseTreeView->zoomPercent()));
    });
    layout->addLayout(toolbar);
    layout->addWidget(parseTreeView);
    
    tabWidget->addTab(parseTreeTab, "🌲 Parse Tree");
}

void MainWindow::setupASTTab() {
    astTab = new QWidget();
    astTab->setStyleSheet("background-color: #0F172A;");
    QVBoxLayout* layout = new QVBoxLayout(astTab);
    layout->setContentsMargins(15, 15, 15, 15);

    QLabel* header = new QLabel("🌳 Abstract Syntax Tree (Clang-style)");
    header->setStyleSheet(
        "font-size: 18px; font-weight: bold; color: #7C3AED; "
        "background-color: #1E293B; padding: 10px; border-radius: 8px; "
        "border-left: 5px solid #A78BFA; margin-bottom: 10px;"
    );
    layout->addWidget(header);

    QHBoxLayout* toolbar = new QHBoxLayout();
    QPushButton* zoomOutBtn = new QPushButton("−");
    QPushButton* zoomInBtn = new QPushButton("+");
    QPushButton* resetViewBtn = new QPushButton("Reset View");
    QLabel* zoomLabel = new QLabel("Zoom: 100%");
    zoomOutBtn->setFixedWidth(32);
    zoomInBtn->setFixedWidth(32);
    zoomOutBtn->setStyleSheet("QPushButton { background-color: #374151; color: #F9FAFB; border: 1px solid #6B7280; border-radius: 4px; }");
    zoomInBtn->setStyleSheet("QPushButton { background-color: #374151; color: #F9FAFB; border: 1px solid #6B7280; border-radius: 4px; }");
    resetViewBtn->setStyleSheet("QPushButton { background-color: #7C3AED; color: #FFFFFF; border: none; border-radius: 4px; padding: 4px 8px; }");
    zoomLabel->setStyleSheet("QLabel { color: #E5E7EB; }");
    
    toolbar->addWidget(new QLabel("AST Controls:"));
    toolbar->addWidget(zoomOutBtn);
    toolbar->addWidget(zoomInBtn);
    toolbar->addWidget(resetViewBtn);
    toolbar->addWidget(zoomLabel);
    toolbar->addStretch();
     
    astView = new ASTView();
    connect(zoomInBtn, &QPushButton::clicked, this, [this, zoomLabel]() {
        astView->zoomIn();
        zoomLabel->setText(QString("Zoom: %1%").arg(astView->zoomPercent()));
    });
    connect(zoomOutBtn, &QPushButton::clicked, this, [this, zoomLabel]() {
        astView->zoomOut();
        zoomLabel->setText(QString("Zoom: %1%").arg(astView->zoomPercent()));
    });
    connect(resetViewBtn, &QPushButton::clicked, this, [this, zoomLabel]() {
        astView->resetView();
        zoomLabel->setText(QString("Zoom: %1%").arg(astView->zoomPercent()));
    });
    layout->addLayout(toolbar);
    layout->addWidget(astView);
    
    tabWidget->addTab(astTab, "🌳 AST View");
}

void MainWindow::setupCFGTab() {
    cfgTab = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(cfgTab);

    cfgTextView = new QTextEdit();
    cfgTextView->setReadOnly(true);
    cfgTextView->setFont(QFont("Consolas", 10));
    cfgTextView->setStyleSheet(
        "QTextEdit {"
        "   background-color: #0B132B;"
        "   color: #E0E1DD;"
        "   border: 1px solid #3A506B;"
        "   border-radius: 8px;"
        "   padding: 8px;"
        "}"
    );

    cfgTextView->setPlainText(
        "Professional CFG (LL(1)-Friendly, No Left Recursion)\n"
        "====================================================\n\n"
        "Scope:\n"
        "  Educational switch-case subset (not full C++).\n"
        "  Preprocessor lines (e.g., #include <string>) are handled by lexical pre-pass and ignored by parser.\n\n"
        "Syntax Non-terminals:\n"
        "  program, preamble_opt, using_opt, pre_stmt_list, pre_stmt, declaration, type_spec,\n"
        "  decl_init_opt, assignment, switch_stmt, case_list, case_clause, default_clause,\n"
        "  stmt_list, stmt, expr, expr_tail, term, term_tail, factor\n\n"
        "Syntax Terminals:\n"
        "  using, namespace, std, int, string, switch, case, default, break,\n"
        "  identifier, int_constant, string_literal, =, +, -, *, /, (, ), {, }, :, ;\n\n"
        "Production Rules (No Left Recursion):\n"
        "  R1   program         -> preamble_opt pre_stmt_list switch_stmt\n"
        "  R2   preamble_opt    -> using_opt\n"
        "  R3   using_opt       -> using namespace std ;\n"
        "  R4   using_opt       -> ε\n"
        "  R5   pre_stmt_list   -> pre_stmt pre_stmt_list\n"
        "  R6   pre_stmt_list   -> ε\n"
        "  R7   pre_stmt        -> declaration\n"
        "  R8   pre_stmt        -> assignment\n"
        "  R9   declaration     -> type_spec identifier decl_init_opt ;\n"
        "  R10  type_spec       -> int\n"
        "  R11  type_spec       -> string\n"
        "  R12  decl_init_opt   -> = expr\n"
        "  R13  decl_init_opt   -> ε\n"
        "  R14  assignment      -> identifier = expr ;\n"
        "  R15  switch_stmt     -> switch ( expr ) { case_list default_clause }\n"
        "  R16  case_list       -> case_clause case_list\n"
        "  R17  case_list       -> ε\n"
        "  R18  case_clause     -> case int_constant : stmt_list break ;\n"
        "  R19  default_clause  -> default : stmt_list break ;\n"
        "  R20  default_clause  -> ε\n"
        "  R21  stmt_list       -> stmt stmt_list\n"
        "  R22  stmt_list       -> ε\n"
        "  R23  stmt            -> declaration\n"
        "  R24  stmt            -> assignment\n"
        "  R25  expr            -> term expr_tail\n"
        "  R26  expr_tail       -> + term expr_tail\n"
        "  R27  expr_tail       -> - term expr_tail\n"
        "  R28  expr_tail       -> ε\n"
        "  R29  term            -> factor term_tail\n"
        "  R30  term_tail       -> * factor term_tail\n"
        "  R31  term_tail       -> / factor term_tail\n"
        "  R32  term_tail       -> ε\n"
        "  R33  factor          -> ( expr )\n"
        "  R34  factor          -> identifier\n"
        "  R35  factor          -> int_constant\n"
        "  R36  factor          -> string_literal\n\n"
        "Lexical Formation Rules (Alphabet and Numbers):\n"
        "  R37  identifier      -> letter ident_tail\n"
        "  R38  ident_tail      -> letter ident_tail\n"
        "  R39  ident_tail      -> digit ident_tail\n"
        "  R40  ident_tail      -> _ ident_tail\n"
        "  R41  ident_tail      -> ε\n"
        "  R42  int_constant    -> digit digits\n"
        "  R43  digits          -> digit digits\n"
        "  R44  digits          -> ε\n"
        "  R45  letter          -> a | b | ... | z | A | B | ... | Z\n"
        "  R46  digit           -> 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9\n\n"
        "Correctness Notes:\n"
        "  - No left-recursive productions are used in this displayed CFG.\n"
        "  - Operator precedence is encoded by expr/term/factor levels.\n"
        "  - The parser implementation enforces left-associative evaluation for +, -, *, /.\n"
        "  - This removes the practical ambiguity seen in naive arithmetic grammars.\n\n"
        "Semantic Constraints:\n"
        "  - Variable must be declared before assignment/use.\n"
        "  - Declaration/assignment types must match (int vs string).\n"
        "  - String support is intentionally limited to direct assignment semantics.\n"
        "  - Allowed string RHS forms: string_literal or identifier (no operators).\n"
        "  - String operands with +, -, *, / are intentionally rejected.\n"
        "  - Duplicate case values are rejected.\n"
        "  - Duplicate declarations in the same scope are rejected.\n"
        "  - Loops and full C++ features are intentionally rejected.\n\n"
        "Visualization Alignment:\n"
        "  - Parse tree view uses compact leaf labels (id/constant/string_literal) equivalent to identifier/int_constant/string_literal tokens.\n"
        "  - Derivation view supports explicit step-by-step navigation and parse-tree step jump.\n"
    );

    layout->addWidget(new QLabel("CFG / Language Rules"));
    layout->addWidget(cfgTextView);

    tabWidget->addTab(cfgTab, "📚 CFG");
}

void MainWindow::setupTokenTab() {
    tokenTab = new QWidget();
    tokenTab->setStyleSheet("background-color: #0F172A;");
    QVBoxLayout* layout = new QVBoxLayout(tokenTab);
    layout->setContentsMargins(15, 15, 15, 15);

    QLabel* header = new QLabel("🔤 Lexical Analyzer Token Stream");
    header->setStyleSheet(
        "font-size: 18px; font-weight: bold; color: #38BDF8; "
        "background-color: #1E293B; padding: 10px; border-radius: 8px; "
        "border-left: 5px solid #0EA5E9; margin-bottom: 10px;"
    );
    layout->addWidget(header);

    tokenCountLabel = new QLabel("Lexical Tokens: 0 | EOF Markers: 0 | Total Rows: 0");
    tokenCountLabel->setStyleSheet(
        "QLabel {"
        "  color: #CBD5E1;"
        "  background-color: #1E293B;"
        "  border: 1px solid #334155;"
        "  border-radius: 6px;"
        "  padding: 8px 10px;"
        "  font-family: 'Consolas';"
        "  font-size: 11pt;"
        "  margin-bottom: 8px;"
        "}"
    );
    layout->addWidget(tokenCountLabel);

    tokenTable = new QTableWidget();
    tokenTable->setColumnCount(4);
    tokenTable->setHorizontalHeaderLabels({"Lexeme (Token)", "Token Type", "Line No.", "Column No."});
    tokenTable->horizontalHeader()->setStretchLastSection(true);
    tokenTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    tokenTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    tokenTable->verticalHeader()->setVisible(false);
    tokenTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tokenTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    tokenTable->setAlternatingRowColors(true);
    tokenTable->setShowGrid(false);

    tokenTable->setStyleSheet(
        "QTableWidget {"
        "  background-color: #1E293B; color: #F8FAFC; border: 1px solid #334155; border-radius: 8px;"
        "  gridline-color: transparent; outline: none; font-family: Consolas, monospace; font-size: 13px;"
        "}"
        "QTableWidget::item { padding: 8px; border-bottom: 1px solid #334155; }"
        "QTableWidget::item:selected { background-color: #0EA5E9; color: white; }"
        "QHeaderView::section {"
        "  background-color: #0F172A; color: #94A3B8; font-weight: bold; padding: 8px;"
        "  border: none; border-bottom: 2px solid #0EA5E9;"
        "}"
        "QTableWidget::item:alternate { background-color: #111827; }"
    );

    layout->addWidget(tokenTable);
    tabWidget->addTab(tokenTab, "🔤 Lexical Phase");
}
void MainWindow::setupSymbolTab() {
    symbolTab = new QWidget();
    symbolTab->setStyleSheet("background-color: #0F172A;");
    QVBoxLayout* layout = new QVBoxLayout(symbolTab);
    layout->setContentsMargins(15, 15, 15, 15);

    QLabel* header = new QLabel("📊 Symbol Table & CFG Correctness Checks");
    header->setStyleSheet(
        "font-size: 18px; font-weight: bold; color: #10B981; "
        "background-color: #1E293B; padding: 10px; border-radius: 8px; "
        "border-left: 5px solid #34D399; margin-bottom: 10px;"
    );
    layout->addWidget(header);

    symbolTable = new QTableWidget();
    symbolTable->setColumnCount(6);
    symbolTable->setHorizontalHeaderLabels({"Identifier Name", "Data Type", "Assigned Value", "Scope Level", "Line", "Column"});
    symbolTable->horizontalHeader()->setStretchLastSection(true);
    symbolTable->verticalHeader()->setVisible(false);
    symbolTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    symbolTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    symbolTable->setAlternatingRowColors(true);
    symbolTable->setShowGrid(false);

    symbolTable->setStyleSheet(
        "QTableWidget {"
        "  background-color: #1E293B; color: #F8FAFC; border: 1px solid #334155; border-radius: 8px;"
        "  gridline-color: transparent; outline: none; font-family: Consolas, monospace; font-size: 13px;"
        "}"
        "QTableWidget::item { padding: 8px; border-bottom: 1px solid #334155; }"
        "QTableWidget::item:selected { background-color: #10B981; color: white; }"
        "QHeaderView::section {"
        "  background-color: #0F172A; color: #94A3B8; font-weight: bold; padding: 8px;"
        "  border: none; border-bottom: 2px solid #10B981;"
        "}"
        "QTableWidget::item:alternate { background-color: #111827; }"
    );

    layout->addWidget(symbolTable);
    tabWidget->addTab(symbolTab, "📊 Semantic Phase");
}
void MainWindow::setupTraceTab() {
    traceTab = new QWidget();
    traceTab->setStyleSheet("background-color: #0F172A;");
    QVBoxLayout* layout = new QVBoxLayout(traceTab);
    layout->setContentsMargins(15, 15, 15, 15);

    QLabel* header = new QLabel("🔄 Intermediate Representation & Target Code Generation");
    header->setStyleSheet(
        "font-size: 18px; font-weight: bold; color: #F59E0B; "
        "background-color: #1E293B; padding: 10px; border-radius: 8px; "
        "border-left: 5px solid #FBBF24; margin-bottom: 10px;"
    );
    layout->addWidget(header);

    threeColumnView = new ThreeColumnView();
    layout->addWidget(threeColumnView);

    tabWidget->addTab(traceTab, "🔍 IR & CodeGen");
}

void MainWindow::setupOptimizationTab() {
    optimizationTab = new QWidget();
    optimizationTab->setStyleSheet("background-color: #0F172A;");
    QVBoxLayout* layout = new QVBoxLayout(optimizationTab);
    layout->setContentsMargins(15, 15, 15, 15);

    QLabel* header = new QLabel("⚡ Intermediate Code Optimization");
    header->setStyleSheet(
        "font-size: 18px; font-weight: bold; color: #8B5CF6; "
        "background-color: #1E293B; padding: 10px; border-radius: 8px; "
        "border-left: 5px solid #A78BFA; margin-bottom: 10px;"
    );
    layout->addWidget(header);

    optimizationPanel = new OptimizationPanel();
    layout->addWidget(optimizationPanel);

    tabWidget->addTab(optimizationTab, "⚡ Optimizer");
}

void MainWindow::setupTokenInsightsTab() {
    tokenInsightsTab = new QWidget();
    tokenInsightsTab->setStyleSheet("background-color: #0F172A;");
    QVBoxLayout* layout = new QVBoxLayout(tokenInsightsTab);
    layout->setContentsMargins(15, 15, 15, 15);

    QLabel* header = new QLabel("📈 Token Insights & Frequency Heatmap");
    header->setStyleSheet(
        "font-size: 18px; font-weight: bold; color: #06B6D4; "
        "background-color: #1E293B; padding: 10px; border-radius: 8px; "
        "border-left: 5px solid #22D3EE; margin-bottom: 10px;"
    );
    layout->addWidget(header);

    tokenChart = new TokenChart();
    layout->addWidget(tokenChart);

    tabWidget->addTab(tokenInsightsTab, "📈 Token Insights");
}

void MainWindow::setupLifetimeTab() {
    lifetimeTab = new QWidget();
    lifetimeTab->setStyleSheet("background-color: #0F172A;");
    QVBoxLayout* layout = new QVBoxLayout(lifetimeTab);
    layout->setContentsMargins(15, 15, 15, 15);

    QLabel* header = new QLabel("🧬 Variable Lifetime & Data Flow");
    header->setStyleSheet(
        "font-size: 18px; font-weight: bold; color: #F59E0B; "
        "background-color: #1E293B; padding: 10px; border-radius: 8px; "
        "border-left: 5px solid #FBBF24; margin-bottom: 10px;"
    );
    layout->addWidget(header);

    lifetimeDiagram = new LifetimeDiagram();
    QScrollArea* scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setWidget(lifetimeDiagram);
    layout->addWidget(scrollArea);

    tabWidget->addTab(lifetimeTab, "🧬 Lifetime");
}

void MainWindow::setupPipelineTab() {
    pipelineTab = new QWidget();
    pipelineTab->setStyleSheet("background-color: #0F172A;");
    QVBoxLayout* layout = new QVBoxLayout(pipelineTab);
    layout->setContentsMargins(15, 15, 15, 15);

    QLabel* header = new QLabel("🚦 Pipeline Studio (Live Phase Diagnostics)");
    header->setStyleSheet(
        "font-size: 18px; font-weight: bold; color: #34D399; "
        "background-color: #1E293B; padding: 10px; border-radius: 8px; "
        "border-left: 5px solid #10B981; margin-bottom: 10px;"
    );
    layout->addWidget(header);

    pipelineDiagram = new PipelineDiagram();
    layout->addWidget(pipelineDiagram);

    tabWidget->addTab(pipelineTab, "🚦 Pipeline Studio");
}

void MainWindow::setupErrorTab() {
    errorTab = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(errorTab);
    
    errorTable = new QTableWidget();
    errorTable->setColumnCount(5);
    errorTable->setHorizontalHeaderLabels({"Phase", "Type", "Line", "Column", "Message"});
    errorTable->horizontalHeader()->setStretchLastSection(true);
    
    layout->addWidget(new QLabel("Errors and Warnings:"));
    layout->addWidget(errorTable);
    
    tabWidget->addTab(errorTab, "❌ Errors");
}

void MainWindow::setupConsoleTab() {
    consoleTab = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(consoleTab);
    
    consoleOutput = new QTextEdit();
    consoleOutput->setReadOnly(true);
    QFont font("Courier New", 9);
    consoleOutput->setFont(font);
    
    layout->addWidget(new QLabel("Compilation Output:"));
    layout->addWidget(consoleOutput);
    
    tabWidget->addTab(consoleTab, "💻 Console");
}

void MainWindow::setupIRFlowTab() {
    irFlowTab = new QWidget();
    irFlowTab->setStyleSheet("background-color: #0F172A;");
    QVBoxLayout* layout = new QVBoxLayout(irFlowTab);
    layout->setContentsMargins(15, 15, 15, 15);

    QLabel* header = new QLabel("🔄 Interactive IR Flow Visualization");
    header->setStyleSheet(
        "font-size: 18px; font-weight: bold; color: #06B6D4; "
        "background-color: #1E293B; padding: 10px; border-radius: 8px; "
        "border-left: 5px solid #22D3EE; margin-bottom: 10px;"
    );
    layout->addWidget(header);

    // Control buttons
    QHBoxLayout* controlsLayout = new QHBoxLayout();
    QPushButton* playBtn = new QPushButton("▶ Play");
    QPushButton* pauseBtn = new QPushButton("⏸ Pause");
    QPushButton* resetBtn = new QPushButton("⏹ Reset");
    QPushButton* stepBtn = new QPushButton("⏭ Step");
    QPushButton* originalBtn = new QPushButton("Original IR");
    QPushButton* optimizedBtn = new QPushButton("Optimized IR");
    
    playBtn->setStyleSheet("QPushButton { background-color: #10B981; color: white; padding: 8px 16px; border-radius: 4px; }");
    pauseBtn->setStyleSheet("QPushButton { background-color: #F59E0B; color: white; padding: 8px 16px; border-radius: 4px; }");
    resetBtn->setStyleSheet("QPushButton { background-color: #EF4444; color: white; padding: 8px 16px; border-radius: 4px; }");
    stepBtn->setStyleSheet("QPushButton { background-color: #8B5CF6; color: white; padding: 8px 16px; border-radius: 4px; }");
    originalBtn->setStyleSheet("QPushButton { background-color: #334155; color: white; padding: 8px 14px; border-radius: 4px; }");
    optimizedBtn->setStyleSheet("QPushButton { background-color: #0EA5E9; color: white; padding: 8px 14px; border-radius: 4px; }");
    
    controlsLayout->addWidget(playBtn);
    controlsLayout->addWidget(pauseBtn);
    controlsLayout->addWidget(resetBtn);
    controlsLayout->addWidget(stepBtn);
    controlsLayout->addSpacing(8);
    controlsLayout->addWidget(originalBtn);
    controlsLayout->addWidget(optimizedBtn);
    controlsLayout->addStretch();
    
    layout->addLayout(controlsLayout);

    irFlowDiagram = new IRFlowDiagram();
    layout->addWidget(irFlowDiagram);

    // Connect controls
    connect(playBtn, &QPushButton::clicked, irFlowDiagram, &IRFlowDiagram::startExecution);
    connect(pauseBtn, &QPushButton::clicked, irFlowDiagram, &IRFlowDiagram::pauseExecution);
    connect(resetBtn, &QPushButton::clicked, irFlowDiagram, &IRFlowDiagram::resetExecution);
    connect(stepBtn, &QPushButton::clicked, irFlowDiagram, &IRFlowDiagram::stepForward);
    connect(originalBtn, &QPushButton::clicked, this, [this]() { irFlowDiagram->setShowOptimized(false); });
    connect(optimizedBtn, &QPushButton::clicked, this, [this]() { irFlowDiagram->setShowOptimized(true); });

    tabWidget->addTab(irFlowTab, "🔄 IR Flow");
}

void MainWindow::setupPerformanceTab() {
    performanceTab = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(performanceTab);
    layout->setContentsMargins(0, 0, 0, 0);

    performanceMetrics = new PerformanceMetrics();
    layout->addWidget(performanceMetrics);

    tabWidget->addTab(performanceTab, "📊 Performance");
}


void MainWindow::connectSignals() {
    connect(compileButton, &QPushButton::clicked, this, &MainWindow::onCompile);
    connect(errorTable, &QTableWidget::cellDoubleClicked, this, &MainWindow::onErrorClicked);
    connect(errorTable, &QTableWidget::cellClicked, this, &MainWindow::onErrorClicked);  // NEW: Single-click also works
    connect(sourceEditor, &QPlainTextEdit::textChanged, this, &MainWindow::onSourceCodeChanged);
    
    // Parse tree interactivity
    connect(parseTreeView, &ParseTreeView::nodeClicked, this, &MainWindow::onParseTreeNodeClicked);
    connect(parseTreeView, &ParseTreeView::nodeHighlightRequested, this, 
            &MainWindow::onParseTreeNodeHighlighted);
    connect(parseTreeView, &ParseTreeView::nodeLexemeRequested, this,
            &MainWindow::onParseTreeLexemeRequested);
    connect(editorErrorTable, &QTableWidget::cellClicked, this, [this](int row, int) {
        const auto* lineItem = editorErrorTable->item(row, 0);
        const auto* colItem = editorErrorTable->item(row, 1);
        if (lineItem && colItem) {
            highlightError(lineItem->text().toInt(), colItem->text().toInt());
        }
    });
}

void MainWindow::onCompile() {
    consoleOutput->clear();
    errorTable->setRowCount(0);
    symbolTable->setRowCount(0);
    if (pipelineDiagram) {
        pipelineDiagram->resetAll();
    }
    
    std::string sourceCode = sourceEditor->toPlainText().toStdString();
    
    if (sourceCode.empty()) {
        QMessageBox::warning(this, "Warning", "Source code is empty!");
        return;
    }
    
    logToConsole("=== COMPILATION STARTED ===\n");

    // Initialize performance metrics
    CompilationMetrics metrics;
    metrics.timestamp = std::chrono::system_clock::now();

    auto animatePhaseProgress = [this](int targetValue) {
        auto* anim = new QPropertyAnimation(phaseProgressBar, "value", this);
        anim->setDuration(260);
        anim->setStartValue(phaseProgressBar->value());
        anim->setEndValue(targetValue);
        anim->setEasingCurve(QEasingCurve::OutCubic);
        anim->start(QAbstractAnimation::DeleteWhenStopped);
    };

    auto setPhaseState = [this](int phaseIndex, const QString& state, qint64 elapsedMs) {
        if (phaseIndex < 0 || phaseIndex >= 7 || !phaseBadges[phaseIndex] || !phaseDurations[phaseIndex]) {
            return;
        }

        QString style;
        if (state == "active") {
            style =
                "QLabel {"
                "  background-color: #E9C46A;"
                "  color: #1A1A1A;"
                "  border: 1px solid #FFD166;"
                "  border-radius: 10px;"
                "  padding: 4px 8px;"
                "  font-family: 'Bahnschrift';"
                "  font-size: 9pt;"
                "  font-weight: 700;"
                "}";
        } else if (state == "done") {
            style =
                "QLabel {"
                "  background-color: #2A9D8F;"
                "  color: #F1FAEE;"
                "  border: 1px solid #58C4B3;"
                "  border-radius: 10px;"
                "  padding: 4px 8px;"
                "  font-family: 'Bahnschrift';"
                "  font-size: 9pt;"
                "  font-weight: 700;"
                "}";
        } else if (state == "failed") {
            style =
                "QLabel {"
                "  background-color: #8D1E2F;"
                "  color: #FCE8EC;"
                "  border: 1px solid #D64562;"
                "  border-radius: 10px;"
                "  padding: 4px 8px;"
                "  font-family: 'Bahnschrift';"
                "  font-size: 9pt;"
                "  font-weight: 700;"
                "}";
        } else {
            style =
                "QLabel {"
                "  background-color: #1F2A44;"
                "  color: #B8C4D6;"
                "  border: 1px solid #3B4A67;"
                "  border-radius: 10px;"
                "  padding: 4px 8px;"
                "  font-family: 'Bahnschrift';"
                "  font-size: 9pt;"
                "  font-weight: 600;"
                "}";
        }

        phaseBadges[phaseIndex]->setStyleSheet(style);
        if (elapsedMs >= 0) {
            phaseDurations[phaseIndex]->setText(QString::number(elapsedMs) + " ms");
        } else {
            phaseDurations[phaseIndex]->setText("-- ms");
        }
    };

    phaseProgressBar->setValue(0);
    for (int i = 0; i < 7; ++i) {
        setPhaseState(i, "pending", -1);
    }

    QElapsedTimer phaseTimer;
    
    // PHASE 1: Lexical Analysis
    setPhaseState(0, "active", -1);
    phaseTimer.restart();
    logToConsole("\n[PHASE 1] Lexical Analysis...");
    lexer->setSource(sourceCode);
    auto tokens = lexer->tokenize();
    metrics.lexerTime = std::chrono::milliseconds(phaseTimer.elapsed());
    const int eofTokenCount = static_cast<int>(std::count_if(tokens.begin(), tokens.end(), [](const Token& token) {
        return token.type == TokenType::END_OF_FILE;
    }));
    const int lexicalTokenCount = static_cast<int>(tokens.size()) - eofTokenCount;
    metrics.tokenCount = lexicalTokenCount;
    metrics.lexicalErrors = static_cast<int>(lexer->getErrors().size());
    animatePhaseProgress(1);
    setPhaseState(0, "done", phaseTimer.elapsed());
    if (pipelineDiagram) {
        pipelineDiagram->setPhaseStatus(0, "done", phaseTimer.elapsed(), lexicalTokenCount);
    }
    logToConsole("  Found " + std::to_string(lexicalTokenCount) +
                 " lexical tokens (" + std::to_string(eofTokenCount) + " EOF marker)");
    updateTokenTable();
    if (tokenChart) {
        tokenChart->setTokens(tokens);
    }
    
    if (!lexer->getErrors().empty()) {
        logToConsole("  Lexical errors found: " + std::to_string(lexer->getErrors().size()));
        for (const auto& err : lexer->getErrors()) {
            int row = errorTable->rowCount();
            errorTable->insertRow(row);
            errorTable->setItem(row, 0, new QTableWidgetItem("Lexer"));
            errorTable->setItem(row, 1, new QTableWidgetItem("Lexical Error"));
            errorTable->setItem(row, 2, new QTableWidgetItem(QString::number(err.line)));
            errorTable->setItem(row, 3, new QTableWidgetItem(QString::number(err.column)));
            errorTable->setItem(row, 4, new QTableWidgetItem(QString::fromStdString(err.message)));
        }

        logToConsole("\n[ERROR] Compilation stopped due to lexical errors.");
        setPhaseState(0, "failed", phaseTimer.elapsed());
        statusLabel->setText("Compilation failed (lexical errors)");
        statusLabel->setStyleSheet("QLabel { padding: 5px; background-color: #ffcccc; color: #cc0000; }");
        return;
    }
    
    // PHASE 2: Syntax Analysis
    setPhaseState(1, "active", -1);
    phaseTimer.restart();
    logToConsole("\n[PHASE 2] Syntax Analysis (Rightmost Derivation)...");
    parser->setTokens(tokens);
    bool parseSuccess = parser->parse();
    metrics.parserTime = std::chrono::milliseconds(phaseTimer.elapsed());
    metrics.derivationSteps = static_cast<int>(parser->getDerivationSteps().size());
    metrics.syntaxErrors = static_cast<int>(parser->getErrors().size());
    animatePhaseProgress(2);
    setPhaseState(1, "done", phaseTimer.elapsed());
    if (pipelineDiagram) {
        pipelineDiagram->setPhaseStatus(1, "done", phaseTimer.elapsed(), metrics.derivationSteps);
    }
    logToConsole("  Generated " + std::to_string(parser->getDerivationSteps().size()) + " derivation steps");
    
    derivationViewer->setDerivationSteps(parser->getDerivationSteps());
    parseTreeView->setParseTree(parser->getParseTree());
    
    // Update AST view with new TranslationUnit if available, otherwise use legacy AST
    if (parser->getTranslationUnit()) {
        astView->setAST(parser->getTranslationUnit());
        // Count AST nodes (simplified)
        metrics.astNodes = 50; // Placeholder - would need proper AST traversal
    } else {
        astView->setAST(parser->getAST());
        metrics.astNodes = 30; // Placeholder
    }
    
    if (!parser->getErrors().empty()) {
        logToConsole("  Parse errors found: " + std::to_string(parser->getErrors().size()));
        std::set<std::string> seenErrors;
        for (const auto& err : parser->getErrors()) {
            const std::string key = std::to_string(err.line) + ":" + std::to_string(err.column) + ":" + err.message;
            if (!seenErrors.insert(key).second) {
                continue;
            }
            int row = errorTable->rowCount();
            errorTable->insertRow(row);
            errorTable->setItem(row, 0, new QTableWidgetItem("Parser"));
            errorTable->setItem(row, 1, new QTableWidgetItem("Syntax Error"));
            errorTable->setItem(row, 2, new QTableWidgetItem(QString::number(err.line)));
            errorTable->setItem(row, 3, new QTableWidgetItem(QString::number(err.column)));
            errorTable->setItem(row, 4, new QTableWidgetItem(QString::fromStdString(err.message)));
        }
    }
    
    if (!parseSuccess) {
        logToConsole("\n[ERROR] Compilation stopped due to syntax errors.");
        setPhaseState(1, "failed", phaseTimer.elapsed());
        statusLabel->setText("Compilation failed (syntax errors)");
        statusLabel->setStyleSheet("QLabel { padding: 5px; background-color: #ffcccc; color: #cc0000; }");
        return;
    }
    
    // VALIDATION: Check C++ compliance (main wrapper and includes)
    logToConsole("\n[VALIDATION] Checking C++ structure...");
    parser->validateMainWrapper();
    parser->validateIncludeStatements();
    
    if (!parser->getErrors().empty()) {
        bool hasComplianceError = false;
        std::set<std::string> seenComplianceErrors;
        for (const auto& err : parser->getErrors()) {
            if (err.message.find("main") != std::string::npos || 
                err.message.find("#include") != std::string::npos) {
                const std::string key = std::to_string(err.line) + ":" + std::to_string(err.column) + ":" + err.message;
                if (!seenComplianceErrors.insert(key).second) {
                    continue;
                }
                hasComplianceError = true;
                int row = errorTable->rowCount();
                errorTable->insertRow(row);
                errorTable->setItem(row, 0, new QTableWidgetItem("Validator"));
                errorTable->setItem(row, 1, new QTableWidgetItem("Compliance"));
                errorTable->setItem(row, 2, new QTableWidgetItem(QString::number(err.line)));
                errorTable->setItem(row, 3, new QTableWidgetItem(QString::number(err.column)));
                errorTable->setItem(row, 4, new QTableWidgetItem(QString::fromStdString(err.message)));
                logToConsole("  ⚠️ " + err.message);
            }
        }
        if (hasComplianceError) {
            logToConsole("\n[ERROR] Compilation stopped due to C++ compliance errors.");
            setPhaseState(1, "failed", phaseTimer.elapsed());
            statusLabel->setText("Compilation failed (C++ compliance)");
            statusLabel->setStyleSheet("QLabel { padding: 5px; background-color: #ffcccc; color: #cc0000; }");
            return;
        }
    }
    logToConsole("  ✅ C++ structure valid");
    
    // PHASE 3: Semantic Analysis
    setPhaseState(2, "active", -1);
    phaseTimer.restart();
    logToConsole("\n[PHASE 3] Semantic Analysis...");
    
    bool semanticSuccess = false;
    if (parser->getTranslationUnit()) {
        // Use new AST
        semanticSuccess = semanticAnalyzer->analyze(parser->getTranslationUnit());
    } else {
        // Fallback to legacy AST
        semanticSuccess = semanticAnalyzer->analyze(parser->getAST());
    }
    
    metrics.semanticTime = std::chrono::milliseconds(phaseTimer.elapsed());
    metrics.symbolTableEntries = static_cast<int>(semanticAnalyzer->getSymbolTable().getAllDeclaredSymbols().size());
    metrics.semanticErrors = static_cast<int>(semanticAnalyzer->getErrors().size());
    animatePhaseProgress(3);
    setPhaseState(2, "done", phaseTimer.elapsed());
    if (pipelineDiagram) {
        pipelineDiagram->setPhaseStatus(2, "done", phaseTimer.elapsed(), metrics.symbolTableEntries);
    }
    
    if (!semanticAnalyzer->getErrors().empty()) {
        logToConsole("  Semantic errors found: " + std::to_string(semanticAnalyzer->getErrors().size()));
        for (const auto& err : semanticAnalyzer->getErrors()) {
            int row = errorTable->rowCount();
            errorTable->insertRow(row);
            errorTable->setItem(row, 0, new QTableWidgetItem("Semantic"));
            errorTable->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(err.errorType)));
            errorTable->setItem(row, 2, new QTableWidgetItem(QString::number(err.line)));
            errorTable->setItem(row, 3, new QTableWidgetItem(QString::number(err.column)));
            errorTable->setItem(row, 4, new QTableWidgetItem(QString::fromStdString(err.message)));
        }
    }

    updateSymbolTable();

    if (!semanticSuccess) {
        logToConsole("\n[ERROR] Compilation stopped due to semantic errors.");
        setPhaseState(2, "failed", phaseTimer.elapsed());
        statusLabel->setText("Compilation failed (semantic errors)");
        statusLabel->setStyleSheet("QLabel { padding: 5px; background-color: #ffcccc; color: #cc0000; }");
        return;
    }
    
    // PHASE 4: TAC Generation
    setPhaseState(3, "active", -1);
    phaseTimer.restart();
    logToConsole("\n[PHASE 4] Intermediate Code Generation (TAC)...");
    
    std::vector<TACInstruction> tacCode;
    if (parser->getTranslationUnit()) {
        // Use new AST
        tacCode = tacGenerator->generate(parser->getTranslationUnit());
    } else {
        // Fallback to legacy AST
        tacCode = tacGenerator->generate(parser->getAST());
    }
    
    metrics.tacTime = std::chrono::milliseconds(phaseTimer.elapsed());
    metrics.tacInstructions = static_cast<int>(tacCode.size());
    animatePhaseProgress(4);
    setPhaseState(3, "done", phaseTimer.elapsed());
    if (pipelineDiagram) {
        pipelineDiagram->setPhaseStatus(3, "done", phaseTimer.elapsed(), metrics.tacInstructions);
    }
    logToConsole("  Generated " + std::to_string(tacCode.size()) + " TAC instructions");

    if (lifetimeDiagram) {
        lifetimeDiagram->setInstructions(tacCode);
    }
    
    // PHASE 5: Optimization
    setPhaseState(4, "active", -1);
    phaseTimer.restart();
    logToConsole("\n[PHASE 5] Code Optimization...");
    optimizer->setInstructions(tacCode);
    auto optimizedTAC = optimizer->optimize(
        optimizationPanel->isConstantFoldingEnabled(),
        optimizationPanel->isDeadCodeEliminationEnabled(),
        optimizationPanel->isCseEnabled(),
        optimizationPanel->isAlgebraicSimplificationEnabled(),
        optimizationPanel->isCopyPropagationEnabled(),
        optimizationPanel->isStrengthReductionEnabled()
    );
    metrics.optimizationTime = std::chrono::milliseconds(phaseTimer.elapsed());
    metrics.optimizedTacInstructions = static_cast<int>(optimizedTAC.size());
    metrics.optimizationRatio = tacCode.size() > 0 ? 
        ((static_cast<double>(tacCode.size() - optimizedTAC.size()) / tacCode.size()) * 100.0) : 0.0;
    animatePhaseProgress(5);
    setPhaseState(4, "done", phaseTimer.elapsed());
    if (pipelineDiagram) {
        pipelineDiagram->setPhaseStatus(4, "done", phaseTimer.elapsed(), metrics.optimizedTacInstructions);
    }
    logToConsole("  Optimized from " + std::to_string(tacCode.size()) + 
                " to " + std::to_string(optimizedTAC.size()) + " instructions");
    
    optimizationPanel->setOptimizationResults(optimizer->getOptimizationResults());
    
    // PHASE 6: Code Generation
    setPhaseState(5, "active", -1);
    phaseTimer.restart();
    logToConsole("\n[PHASE 6] Target Code Generation...");
    auto assembly = codeGenerator->generate(optimizedTAC);
    metrics.codegenTime = std::chrono::milliseconds(phaseTimer.elapsed());
    metrics.assemblyInstructions = static_cast<int>(assembly.size());
    animatePhaseProgress(6);
    setPhaseState(5, "done", phaseTimer.elapsed());
    if (pipelineDiagram) {
        pipelineDiagram->setPhaseStatus(5, "done", phaseTimer.elapsed(), metrics.assemblyInstructions);
    }
    logToConsole("  Generated " + std::to_string(assembly.size()) + " assembly instructions");
    
    // PHASE 7: Target Optimization
    setPhaseState(6, "active", -1);
    phaseTimer.restart();
    logToConsole("\n[PHASE 7] Target Code Optimization...");
    targetOptimizer->setAssembly(assembly);
    auto optimizedAssembly = targetOptimizer->optimize();
    metrics.targetOptTime = std::chrono::milliseconds(phaseTimer.elapsed());
    metrics.optimizedAssemblyInstructions = static_cast<int>(optimizedAssembly.size());
    metrics.compressionRatio = assembly.size() > 0 ? 
        ((static_cast<double>(assembly.size() - optimizedAssembly.size()) / assembly.size()) * 100.0) : 0.0;
    animatePhaseProgress(7);
    setPhaseState(6, "done", phaseTimer.elapsed());
    if (pipelineDiagram) {
        pipelineDiagram->setPhaseStatus(6, "done", phaseTimer.elapsed(), metrics.optimizedAssemblyInstructions);
    }
    logToConsole("  Optimized from " + std::to_string(assembly.size()) + 
                " to " + std::to_string(optimizedAssembly.size()) + " instructions");
    
    // Update three-column view
    threeColumnView->setData(sourceCode, optimizedTAC, optimizedAssembly);
    
    // Update performance metrics
    performanceMetrics->updateMetrics(metrics);

    // Show the compile report card dialog after successful compilation.
    CompileStats reportStats;
    reportStats.tokenCount = metrics.tokenCount;
    reportStats.derivationSteps = metrics.derivationSteps;
    reportStats.astNodes = metrics.astNodes;
    reportStats.tacBefore = metrics.tacInstructions;
    reportStats.tacAfter = metrics.optimizedTacInstructions;
    reportStats.assemblyLines = metrics.optimizedAssemblyInstructions;
    reportStats.symbolCount = metrics.symbolTableEntries;
    reportStats.sourceLines = sourceEditor->document()->blockCount();
    reportStats.success = true;

    if (auto* program = parser->getAST(); program && program->switchStmt) {
        reportStats.caseCount = static_cast<int>(program->switchStmt->cases.size());
        if (program->switchStmt->defaultCase) {
            reportStats.caseCount += 1;
        }
    }

    reportStats.phaseMs = {
        metrics.lexerTime.count(),
        metrics.parserTime.count(),
        metrics.semanticTime.count(),
        metrics.tacTime.count(),
        metrics.optimizationTime.count(),
        metrics.codegenTime.count(),
        metrics.targetOptTime.count()
    };
    reportStats.phaseNames = {
        "Lexical",
        "Parser",
        "Semantic",
        "TAC",
        "Optimize",
        "CodeGen",
        "TargetOpt"
    };
    reportStats.totalMs = 0;
    for (const auto phaseMs : reportStats.phaseMs) {
        reportStats.totalMs += phaseMs;
    }

    auto* reportDialog = new CompileReport(this);
    reportDialog->setStats(reportStats);
    reportDialog->animateIn();
    
    logToConsole("\n=== COMPILATION COMPLETED SUCCESSFULLY ===");
    statusLabel->setText("Compilation successful!");
    statusLabel->setStyleSheet("QLabel { padding: 5px; background-color: #ccffcc; color: #006600; }");
}

void MainWindow::updateTokenTable() {
    tokenTable->setRowCount(0);

    const auto& tokens = lexer->getTokens();
    int eofTokenCount = 0;
    for (const auto& token : tokens) {
        const bool isEofToken = token.type == TokenType::END_OF_FILE;
        if (isEofToken) {
            ++eofTokenCount;
        }

        int row = tokenTable->rowCount();
        tokenTable->insertRow(row);
        
        const QString lexeme = isEofToken && token.lexeme.empty()
            ? QString("<EOF>")
            : QString::fromStdString(token.lexeme);
        QTableWidgetItem* lexItem = new QTableWidgetItem(lexeme);
        QTableWidgetItem* typeItem = new QTableWidgetItem(QString::fromStdString(token.typeToString()));
        QTableWidgetItem* lineItem = new QTableWidgetItem(QString::number(token.line));
        QTableWidgetItem* colItem = new QTableWidgetItem(QString::number(token.column));

        // Let's add brilliant colors to token types
        QString typeStr = QString::fromStdString(token.typeToString());
        if (typeStr == "Keyword") {
            typeItem->setForeground(QColor("#A78BFA")); // Purple
            typeItem->setFont(QFont("Consolas", 13, QFont::Bold));
        } else if (typeStr == "Identifier") {
            typeItem->setForeground(QColor("#38BDF8")); // Cyan
        } else if (typeStr == "Number") {
            typeItem->setForeground(QColor("#FBBF24")); // Amber
        } else if (typeStr == "Operator") {
            typeItem->setForeground(QColor("#F87171")); // Red
        } else if (typeStr.contains("Symbol")) {
            typeItem->setForeground(QColor("#94A3B8")); // Grey
        }

        if (isEofToken) {
            lexItem->setForeground(QColor("#94A3B8"));
            typeItem->setForeground(QColor("#94A3B8"));
            lexItem->setFont(QFont("Consolas", 12, QFont::StyleItalic));
            typeItem->setFont(QFont("Consolas", 12, QFont::StyleItalic));
        }

        lexItem->setTextAlignment(Qt::AlignCenter);
        typeItem->setTextAlignment(Qt::AlignCenter);
        lineItem->setTextAlignment(Qt::AlignCenter);
        colItem->setTextAlignment(Qt::AlignCenter);

        tokenTable->setItem(row, 0, lexItem);
        tokenTable->setItem(row, 1, typeItem);
        tokenTable->setItem(row, 2, lineItem);
        tokenTable->setItem(row, 3, colItem);
    }

    const int lexicalTokenCount = static_cast<int>(tokens.size()) - eofTokenCount;
    tokenCountLabel->setText(QString("Lexical Tokens: %1 | EOF Markers: %2 | Total Rows: %3")
        .arg(lexicalTokenCount)
        .arg(eofTokenCount)
        .arg(tokens.size()));
}

void MainWindow::updateSymbolTable() {
    symbolTable->setRowCount(0);

    static const std::set<std::string> blockedSymbols = {
        "include", "iostream", "using", "namespace", "std", "main",
        "cin", "cout", "endl",
        "switch", "case", "default", "break", "return",
        "int", "float", "string", "void",
        "true", "false", "NULL", "nullptr"
    };

    const auto& symbols = semanticAnalyzer->getSymbolTable().getAllDeclaredSymbols();
    for (const auto& sym : symbols) {
        if (sym.name.empty() || sym.type.empty()) {
            continue;
        }
        if (blockedSymbols.find(sym.name) != blockedSymbols.end()) {
            continue;
        }

        int row = symbolTable->rowCount();
        symbolTable->insertRow(row);
        
        QTableWidgetItem* nameItem = new QTableWidgetItem(QString::fromStdString(sym.name));
        QTableWidgetItem* typeItem = new QTableWidgetItem(QString::fromStdString(sym.type));
        QTableWidgetItem* valItem = new QTableWidgetItem(QString::fromStdString(sym.value));
        QTableWidgetItem* scopeItem = new QTableWidgetItem(QString::number(sym.scope));
        QTableWidgetItem* lineItem = new QTableWidgetItem(QString::number(sym.line));
        QTableWidgetItem* colItem = new QTableWidgetItem(QString::number(sym.column));

        // Let's add brilliant colors
        nameItem->setForeground(QColor("#38BDF8")); // Cyan
        nameItem->setFont(QFont("Consolas", 14, QFont::Bold));

        if (sym.type == "int") {
            typeItem->setForeground(QColor("#34D399")); // Green
        } else if (sym.type == "string") {
            typeItem->setForeground(QColor("#FBBF24")); // Amber
        }

        if (sym.value == "uninitialized") {
            valItem->setForeground(QColor("#F87171")); // Red
            valItem->setFont(QFont("Consolas", 12, QFont::StyleItalic));
        } else {
            valItem->setForeground(QColor("#A78BFA")); // Purple
        }

        nameItem->setTextAlignment(Qt::AlignCenter);
        typeItem->setTextAlignment(Qt::AlignCenter);
        valItem->setTextAlignment(Qt::AlignCenter);
        scopeItem->setTextAlignment(Qt::AlignCenter);
        lineItem->setTextAlignment(Qt::AlignCenter);
        colItem->setTextAlignment(Qt::AlignCenter);

        symbolTable->setItem(row, 0, nameItem);
        symbolTable->setItem(row, 1, typeItem);
        symbolTable->setItem(row, 2, valItem);
        symbolTable->setItem(row, 3, scopeItem);
        symbolTable->setItem(row, 4, lineItem);
        symbolTable->setItem(row, 5, colItem);
    }
}

void MainWindow::updateErrorTable() {
    // Errors are added during compilation phases
}

void MainWindow::logToConsole(const std::string& message) {
    consoleOutput->append(QString::fromStdString(message));
}

void MainWindow::onErrorClicked(int row, int column) {
    // Navigate to error location in source editor
    int line = errorTable->item(row, 2)->text().toInt();
    int col = errorTable->item(row, 3)->text().toInt();
    
    highlightError(line, col);
    tabWidget->setCurrentIndex(0);  // Switch to editor tab
}

void MainWindow::highlightError(int line, int column) {
    QTextDocument* doc = sourceEditor->document();
    const int totalLines = doc->blockCount();
    if (totalLines <= 0) {
        return;
    }

    line = std::max(1, std::min(line, totalLines));
    QTextBlock block = doc->findBlockByNumber(line - 1);
    
    if (!block.isValid()) return;
    
    const int lineLen = block.text().length();
    const int clampedColumn = std::max(1, column);
    const int caretCol0 = (lineLen > 0)
        ? std::min(clampedColumn - 1, lineLen - 1)
        : 0;

    QTextCursor cursor(block);
    cursor.setPosition(block.position() + caretCol0);
    sourceEditor->setTextCursor(cursor);
    sourceEditor->ensureCursorVisible();
    sourceEditor->setFocus();

    QList<QTextEdit::ExtraSelection> selections;
    QTextEdit::ExtraSelection lineSelection;
    lineSelection.format.setBackground(QColor(224, 92, 106, 56));
    lineSelection.format.setProperty(QTextFormat::FullWidthSelection, true);
    lineSelection.cursor = QTextCursor(block);
    selections.append(lineSelection);

    if (lineLen > 0) {
        QTextEdit::ExtraSelection tokenSelection;
        tokenSelection.format.setUnderlineColor(QColor("#E05C6A"));
        tokenSelection.format.setUnderlineStyle(QTextCharFormat::WaveUnderline);
        QTextCursor tokenCursor(block);
        tokenCursor.setPosition(block.position() + caretCol0);
        tokenCursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, 1);
        tokenSelection.cursor = tokenCursor;
        selections.append(tokenSelection);
    }

    sourceEditor->setExtraSelectionsMerged(selections);
    
    // Clear highlight after 300ms
    QTimer::singleShot(300, sourceEditor, [this]() {
        sourceEditor->setExtraSelectionsMerged({});
    });
}

void MainWindow::onSourceCodeChanged() {
    // Restart the debounce timer whenever user types
    parseTimer->stop();
    statusLabel->setText("Typing...");
    parseTimer->start();  // Restart 500ms countdown
}

void MainWindow::performRealTimeParse() {
    parseTimer->stop();
    
    std::string sourceCode = sourceEditor->toPlainText().toStdString();
    
    if (sourceCode.empty()) {
        editorErrorTable->setRowCount(0);
        sourceEditor->clearErrors();
        statusLabel->setText("Ready");
        return;
    }
    
    statusLabel->setText("Parsing...");
    
    const QString editorText = sourceEditor->toPlainText();
    const QStringList lines = editorText.split('\n');
    auto safeLineLen = [&lines](int ln) -> int {
        if (ln <= 0 || ln > lines.size()) return 1;
        return std::max(1, static_cast<int>(lines[ln - 1].length()));
    };

    // Tokenize
    lexer->setSource(sourceCode);
    lexer->tokenize();
    const auto& lexerErrors = lexer->getErrors();
    
    if (!lexerErrors.empty()) {
        // Show lexer errors in editor error panel
        editorErrorTable->setRowCount(0);
        std::vector<EditorError> errors;
        for (const auto& err : lexerErrors) {
            const int safeLine = std::max(1, err.line);
            const int safeCol = std::max(1, err.column);
            int row = editorErrorTable->rowCount();
            editorErrorTable->insertRow(row);
            editorErrorTable->setItem(row, 0, new QTableWidgetItem(QString::number(safeLine)));
            editorErrorTable->setItem(row, 1, new QTableWidgetItem(QString::number(safeCol)));
            editorErrorTable->setItem(row, 2, new QTableWidgetItem("Lexer"));
            QString msg = QString("Error in line %1: %2")
                .arg(safeLine)
                .arg(QString::fromStdString(err.message));
            editorErrorTable->setItem(row, 3, new QTableWidgetItem(msg));
            
            const int lineLen = safeLineLen(safeLine);
            const int startCol = std::max(1, std::min(safeCol, lineLen));
            errors.emplace_back(safeLine, startCol, startCol, msg.toStdString());
        }
        sourceEditor->setErrors(errors);
        statusLabel->setText(QString("Ready (Errors: %1)").arg(lexerErrors.size()));
        return;
    }
    
    // Parse
    parser->setTokens(lexer->getTokens());
    parser->parse();
    const auto& parserErrors = parser->getErrors();
    
    editorErrorTable->setRowCount(0);
    std::vector<EditorError> errors;
    
    for (const auto& err : parserErrors) {
        const int safeLine = std::max(1, err.line);
        const int safeCol = std::max(1, err.column);
        int row = editorErrorTable->rowCount();
        editorErrorTable->insertRow(row);
        editorErrorTable->setItem(row, 0, new QTableWidgetItem(QString::number(safeLine)));
        editorErrorTable->setItem(row, 1, new QTableWidgetItem(QString::number(safeCol)));
        const std::string& msg = err.message;
        QString category = "Syntax";
        if (msg.find("main") != std::string::npos || msg.find("#include") != std::string::npos) {
            category = "Compliance";
        } else if (msg.find("loop") != std::string::npos || msg.find("switch") != std::string::npos) {
            category = "Grammar";
        }
        editorErrorTable->setItem(row, 2, new QTableWidgetItem(category));
        QString uiMessage = QString::fromStdString(err.message);
        if (uiMessage.contains("main", Qt::CaseInsensitive)) {
            uiMessage += " | Suggestion: add `int main() { ... return 0; }`.";
        } else if (uiMessage.contains("#include", Qt::CaseInsensitive)) {
            uiMessage += " | Suggestion: add `#include <iostream>`.";
        } else if (uiMessage.contains("break", Qt::CaseInsensitive)) {
            uiMessage += " | Suggestion: each `case` should end with `break;`.";
        } else if (uiMessage.contains("switch", Qt::CaseInsensitive)) {
            uiMessage += " | Suggestion: ensure one valid `switch (expr) { ... }` exists.";
        }
        uiMessage = QString("Error in line %1: %2").arg(safeLine).arg(uiMessage);
        editorErrorTable->setItem(row, 3, new QTableWidgetItem(uiMessage));
        
        const int lineLen = safeLineLen(safeLine);
        const int startCol = std::max(1, std::min(safeCol, lineLen));
        const int tokenLen = std::max(1, static_cast<int>(err.found.size()));
        const int endCol = std::min(lineLen, startCol + tokenLen - 1);
        errors.emplace_back(safeLine, startCol, std::max(startCol, endCol), uiMessage.toStdString());
    }
    
    sourceEditor->setErrors(errors);
    
    if (parserErrors.empty() && lexerErrors.empty()) {
        statusLabel->setText("✓ Ready (No Errors)");
        validityLabel->show();
    } else {
        statusLabel->setText(QString("⚠ Ready (Errors: %1)").arg(parserErrors.size() + lexerErrors.size()));
        validityLabel->hide();
    }
}

void MainWindow::onParseTreeNodeClicked(int derivationStep)
{
    if (derivationStep < 0) {
        return;
    }
    derivationViewer->goToStep(derivationStep);
    tabWidget->setCurrentWidget(derivationTab);
}

void MainWindow::onParseTreeNodeHighlighted(int line, int column, int endLine, int endColumn, const QString& tooltip)
{
    // Highlight the corresponding code range in the editor
    QTextCursor cursor = sourceEditor->textCursor();
    QTextDocument* doc = sourceEditor->document();
    
    // Convert line/column to absolute position
    QTextBlock block = doc->findBlockByNumber(line - 1);
    if (!block.isValid()) return;
    
    const int clampedStart = std::max(1, column);
    const int clampedEnd = std::max(clampedStart, endColumn);
    int startPos = block.position() + (clampedStart - 1);
    int endPos = doc->findBlockByNumber(endLine - 1).position() + (clampedEnd - 1);
    
    if (endPos < startPos) endPos = startPos + 1;
    
    // Select the range
    cursor.setPosition(startPos);
    cursor.setPosition(endPos, QTextCursor::KeepAnchor);
    sourceEditor->setTextCursor(cursor);
    
    // Ensure visible
    sourceEditor->ensureCursorVisible();
    
    // Show tooltip
    if (!tooltip.isEmpty()) {
        QToolTip::showText(QCursor::pos(), tooltip, sourceEditor);
    }
}

void MainWindow::onParseTreeLexemeRequested(const QString& lexeme, const QString& tooltip)
{
    QString text = sourceEditor->toPlainText();
    if (lexeme.isEmpty() || text.isEmpty()) {
        return;
    }

    const int start = text.indexOf(lexeme);
    if (start < 0) {
        return;
    }

    QTextCursor cursor = sourceEditor->textCursor();
    cursor.setPosition(start);
    cursor.setPosition(start + lexeme.length(), QTextCursor::KeepAnchor);
    sourceEditor->setTextCursor(cursor);
    sourceEditor->ensureCursorVisible();
    sourceEditor->setFocus();

    QList<QTextEdit::ExtraSelection> selections;
    QTextEdit::ExtraSelection sel;
    sel.format.setBackground(QColor(124, 158, 245, 115));
    sel.format.setForeground(QColor("#FFFFFF"));
    sel.cursor = cursor;
    selections.append(sel);
    sourceEditor->setExtraSelectionsMerged(selections);
    QTimer::singleShot(500, sourceEditor, [this]() { sourceEditor->setExtraSelectionsMerged({}); });

    if (!tooltip.isEmpty()) {
        QToolTip::showText(QCursor::pos(), tooltip, sourceEditor);
    }
}


