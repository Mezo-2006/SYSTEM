#include "DebuggerPanel.h"
#include <QGroupBox>
#include <QHeaderView>
#include <QTimer>
#include <QSplitter>
#include <QProgressBar>

DebuggerPanel::DebuggerPanel(QWidget* parent)
    : QWidget(parent), currentStepIndex(0), isPlaying(false), playbackSpeed(500),
      treeDepth(0), totalNodes(0), terminalCount(0), nonTerminalCount(0) {
    
    playbackTimer = new QTimer(this);
    connect(playbackTimer, &QTimer::timeout, this, &DebuggerPanel::onStepForward);
    
    setupUI();
}

DebuggerPanel::~DebuggerPanel() = default;

void DebuggerPanel::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(8, 8, 8, 8);
    mainLayout->setSpacing(8);
    
    tabWidget = new QTabWidget();
    
    // Tab 1: Debugger Controls
    setupDebuggerPanel();
    
    // Tab 2: Statistics
    setupStatisticsPanel();
    
    // Tab 3: Variable Inspector
    setupVariableInspector();
    
    // Tab 4: Rule Explorer
    setupRuleExplorer();
    
    mainLayout->addWidget(tabWidget);
    setLayout(mainLayout);
}

void DebuggerPanel::setupDebuggerPanel() {
    QWidget* debugWidget = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(debugWidget);
    
    // Control Panel
    setupControlPanel();
    QGroupBox* controlGroup = new QGroupBox("Derivation Playback");
    QVBoxLayout* ctrlLayout = new QVBoxLayout(controlGroup);
    
    // Playback buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    reverseBtn = new QPushButton("⏮️ Reverse");
    stepBwdBtn = new QPushButton("⏪ Step Back");
    playPauseBtn = new QPushButton("▶️ Play");
    stepFwdBtn = new QPushButton("⏩ Step Fwd");
    
    buttonLayout->addWidget(reverseBtn);
    buttonLayout->addWidget(stepBwdBtn);
    buttonLayout->addWidget(playPauseBtn);
    buttonLayout->addWidget(stepFwdBtn);
    
    connect(playPauseBtn, &QPushButton::clicked, this, &DebuggerPanel::onPlayPause);
    connect(reverseBtn, &QPushButton::clicked, this, &DebuggerPanel::onReverse);
    connect(stepFwdBtn, &QPushButton::clicked, this, &DebuggerPanel::onStepForward);
    connect(stepBwdBtn, &QPushButton::clicked, this, &DebuggerPanel::onStepBackward);
    
    ctrlLayout->addLayout(buttonLayout);
    
    // Slider
    QHBoxLayout* sliderLayout = new QHBoxLayout();
    sliderLayout->addWidget(new QLabel("Step:"));
    stepSlider = new QSlider(Qt::Horizontal);
    stepSlider->setRange(0, 100);
    stepSlider->setTickPosition(QSlider::TicksBelow);
    stepSlider->setTickInterval(10);
    connect(stepSlider, &QSlider::sliderMoved, this, &DebuggerPanel::onSliderMoved);
    sliderLayout->addWidget(stepSlider);
    stepCountLabel = new QLabel("0 / 0");
    sliderLayout->addWidget(stepCountLabel);
    ctrlLayout->addLayout(sliderLayout);
    
    // Speed control
    QHBoxLayout* speedLayout = new QHBoxLayout();
    speedLayout->addWidget(new QLabel("Speed:"));
    speedSpinBox = new QSpinBox();
    speedSpinBox->setRange(100, 2000);
    speedSpinBox->setValue(500);
    speedSpinBox->setSuffix(" ms");
    connect(speedSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &DebuggerPanel::onSpeedChanged);
    speedLayout->addWidget(speedSpinBox);
    speedLayout->addStretch();
    ctrlLayout->addLayout(speedLayout);
    
    layout->addWidget(controlGroup);
    
    // Derivation display
    QGroupBox* derivGroup = new QGroupBox("Derivation Visualization");
    QVBoxLayout* derivLayout = new QVBoxLayout(derivGroup);
    
    currentRuleLabel = new QLabel("Applied Rule: (none)");
    currentRuleLabel->setStyleSheet("QLabel { background-color: #2A3F5F; color: #E0E1DD; padding: 8px; border-radius: 4px; font-weight: bold; }");
    derivLayout->addWidget(currentRuleLabel);
    
    expandedNodeLabel = new QLabel("Expanded Node: (none)");
    expandedNodeLabel->setStyleSheet("QLabel { background-color: #2A3F5F; color: #7c9ef5; padding: 6px; border-radius: 4px; }");
    derivLayout->addWidget(expandedNodeLabel);
    
    sententialFormDisplay = new QTextEdit();
    sententialFormDisplay->setReadOnly(true);
    sententialFormDisplay->setFont(QFont("Courier", 10));
    sententialFormDisplay->setStyleSheet(
        "QTextEdit { background-color: #0B132B; color: #56d4a0; border: 1px solid #3A506B; border-radius: 4px; padding: 8px; }"
    );
    sententialFormDisplay->setPlaceholderText("Sentential form at current step...");
    derivLayout->addWidget(new QLabel("Sentential Form:"), 0);
    derivLayout->addWidget(sententialFormDisplay, 1);
    
    layout->addWidget(derivGroup);
    tabWidget->addTab(debugWidget, "🐛 Debugger");
}

void DebuggerPanel::setupControlPanel() {
    // Already set up in setupDebuggerPanel
}

void DebuggerPanel::setupStatisticsPanel() {
    QWidget* statsWidget = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(statsWidget);
    
    QGroupBox* treeStatsGroup = new QGroupBox("Parse Tree Statistics");
    QVBoxLayout* statsLayout = new QVBoxLayout(treeStatsGroup);
    
    // Stats grid
    treeDepthLabel = new QLabel("🌲 Tree Depth: 0");
    treeDepthLabel->setStyleSheet("QLabel { font-size: 12px; padding: 6px; background-color: #1C2541; border-radius: 4px; }");
    statsLayout->addWidget(treeDepthLabel);
    
    nodeCountLabel = new QLabel("📊 Total Nodes: 0");
    nodeCountLabel->setStyleSheet("QLabel { font-size: 12px; padding: 6px; background-color: #1C2541; border-radius: 4px; }");
    statsLayout->addWidget(nodeCountLabel);
    
    terminalCountLabel = new QLabel("📄 Terminals: 0");
    terminalCountLabel->setStyleSheet("QLabel { font-size: 12px; padding: 6px; background-color: #1C2541; border-radius: 4px; }");
    statsLayout->addWidget(terminalCountLabel);
    
    nonTerminalCountLabel = new QLabel("📋 Non-Terminals: 0");
    nonTerminalCountLabel->setStyleSheet("QLabel { font-size: 12px; padding: 6px; background-color: #1C2541; border-radius: 4px; }");
    statsLayout->addWidget(nonTerminalCountLabel);
    
    depthPercentLabel = new QLabel("📈 Depth Ratio: 0%");
    depthPercentLabel->setStyleSheet("QLabel { font-size: 12px; padding: 6px; background-color: #1C2541; border-radius: 4px; }");
    statsLayout->addWidget(depthPercentLabel);
    
    layout->addWidget(treeStatsGroup);
    
    // Rule frequency analysis
    QGroupBox* freqGroup = new QGroupBox("Production Rule Analysis");
    QVBoxLayout* freqLayout = new QVBoxLayout(freqGroup);
    frequencyLabel = new QTextEdit();
    frequencyLabel->setReadOnly(true);
    frequencyLabel->setFont(QFont("Courier", 9));
    frequencyLabel->setStyleSheet("QTextEdit { background-color: #0B132B; color: #E0E1DD; }");
    frequencyLabel->setPlaceholderText("Rule frequency will be shown here...");
    freqLayout->addWidget(frequencyLabel);
    
    layout->addWidget(freqGroup);
    layout->addStretch();
    
    tabWidget->addTab(statsWidget, "📈 Statistics");
}

void DebuggerPanel::setupVariableInspector() {
    QWidget* varWidget = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(varWidget);
    
    QLabel* varLabel = new QLabel("Variables at current derivation step:");
    varLabel->setStyleSheet("QLabel { font-weight: bold; color: #7c9ef5; }");
    layout->addWidget(varLabel);
    
    variableTable = new QTableWidget();
    variableTable->setColumnCount(3);
    variableTable->setHorizontalHeaderLabels({"Variable", "Type", "Value"});
    variableTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    variableTable->setStyleSheet(
        "QTableWidget { background-color: #1A1D27; color: #E0E1DD; border: 1px solid #3A506B; }"
        "QHeaderView::section { background-color: #22263a; color: #9199b8; padding: 4px; }"
    );
    layout->addWidget(variableTable);
    
    tabWidget->addTab(varWidget, "📦 Variables");
}

void DebuggerPanel::setupRuleExplorer() {
    QWidget* ruleWidget = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(ruleWidget);
    
    QLabel* ruleLabel = new QLabel("Grammar Production Rules (double-click to see FIRST/FOLLOW):");
    ruleLabel->setStyleSheet("QLabel { font-weight: bold; color: #7c9ef5; }");
    layout->addWidget(ruleLabel);
    
    QHBoxLayout* ruleLayout = new QHBoxLayout();
    
    // Rule tree on left
    ruleTree = new QTreeWidget();
    ruleTree->setHeaderLabel("Rules");
    ruleTree->setStyleSheet(
        "QTreeWidget { background-color: #0B132B; color: #E0E1DD; border: 1px solid #3A506B; }"
    );
    
    // Add rule categories
    QStringList categories = {"Program Structure", "Statements", "Expressions", "Lexical"};
    for (const auto& cat : categories) {
        auto item = new QTreeWidgetItem();
        item->setText(0, cat);
        ruleTree->addTopLevelItem(item);
    }
    
    connect(ruleTree, &QTreeWidget::itemDoubleClicked, this, &DebuggerPanel::onRuleDoubleClicked);
    ruleLayout->addWidget(ruleTree, 1);
    
    // Rule details on right
    ruleDetailsDisplay = new QTextEdit();
    ruleDetailsDisplay->setReadOnly(true);
    ruleDetailsDisplay->setFont(QFont("Courier", 9));
    ruleDetailsDisplay->setStyleSheet("QTextEdit { background-color: #0B132B; color: #56d4a0; }");
    ruleDetailsDisplay->setPlaceholderText("FIRST/FOLLOW sets will appear here...");
    ruleLayout->addWidget(ruleDetailsDisplay, 1);
    
    layout->addLayout(ruleLayout);
    tabWidget->addTab(ruleWidget, "📚 Rules");
}

void DebuggerPanel::setDerivationSteps(const std::vector<std::pair<std::string, std::string>>& steps) {
    derivationSteps = steps;
    stepSlider->setMaximum(derivationSteps.size() > 0 ? derivationSteps.size() - 1 : 0);
    currentStepIndex = 0;
    updateStepDisplay();
}

void DebuggerPanel::updateStatistics(int depth, int nodes, int terms, int nonterms) {
    treeDepth = depth;
    totalNodes = nodes;
    terminalCount = terms;
    nonTerminalCount = nonterms;
    
    treeDepthLabel->setText(QString("🌲 Tree Depth: %1").arg(depth));
    nodeCountLabel->setText(QString("📊 Total Nodes: %1").arg(nodes));
    terminalCountLabel->setText(QString("📄 Terminals: %1").arg(terms));
    nonTerminalCountLabel->setText(QString("📋 Non-Terminals: %1").arg(nonterms));
    
    if (totalNodes > 0) {
        int depthPercent = (treeDepth * 100) / totalNodes;
        depthPercentLabel->setText(QString("📈 Depth Ratio: %1%").arg(depthPercent));
    }
}

void DebuggerPanel::updateVariables(const std::map<std::string, std::pair<std::string, std::string>>& vars) {
    variables = vars;
    
    variableTable->setRowCount(0);
    int row = 0;
    for (const auto& [name, typeVal] : vars) {
        variableTable->insertRow(row);
        variableTable->setItem(row, 0, new QTableWidgetItem(QString::fromStdString(name)));
        variableTable->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(typeVal.first)));
        variableTable->setItem(row, 2, new QTableWidgetItem(QString::fromStdString(typeVal.second)));
        row++;
    }
}

QString DebuggerPanel::getCurrentSententialForm() const {
    if (currentStepIndex < derivationSteps.size()) {
        return QString::fromStdString(derivationSteps[currentStepIndex].first);
    }
    return "";
}

void DebuggerPanel::updateStepDisplay() {
    if (currentStepIndex < derivationSteps.size()) {
        const auto& [form, rule] = derivationSteps[currentStepIndex];
        sententialFormDisplay->setText(QString::fromStdString(form));
        currentRuleLabel->setText(QString("Applied Rule: %1").arg(QString::fromStdString(rule)));
        stepCountLabel->setText(QString("%1 / %2").arg(currentStepIndex + 1).arg(derivationSteps.size()));
        stepSlider->blockSignals(true);
        stepSlider->setValue(currentStepIndex);
        stepSlider->blockSignals(false);
        emit stepChanged(currentStepIndex);
    }
}

void DebuggerPanel::onPlayPause() {
    isPlaying = !isPlaying;
    if (isPlaying) {
        playPauseBtn->setText("⏸️ Pause");
        playbackTimer->start(playbackSpeed);
    } else {
        playPauseBtn->setText("▶️ Play");
        playbackTimer->stop();
    }
}

void DebuggerPanel::onReverse() {
    currentStepIndex = 0;
    updateStepDisplay();
    isPlaying = false;
    playPauseBtn->setText("▶️ Play");
    playbackTimer->stop();
}

void DebuggerPanel::onStepForward() {
    if (currentStepIndex < derivationSteps.size() - 1) {
        currentStepIndex++;
        updateStepDisplay();
    } else if (isPlaying) {
        isPlaying = false;
        playPauseBtn->setText("▶️ Play");
        playbackTimer->stop();
    }
}

void DebuggerPanel::onStepBackward() {
    if (currentStepIndex > 0) {
        currentStepIndex--;
        updateStepDisplay();
    }
}

void DebuggerPanel::onSliderMoved(int value) {
    currentStepIndex = value;
    updateStepDisplay();
    isPlaying = false;
    playPauseBtn->setText("▶️ Play");
    playbackTimer->stop();
}

void DebuggerPanel::updateCurrentStep() {
    if (isPlaying && currentStepIndex < static_cast<int>(derivationSteps.size())) {
        onStepForward();
    }
}

void DebuggerPanel::onSpeedChanged(int value) {
    playbackSpeed = value;
    if (isPlaying) {
        playbackTimer->stop();
        playbackTimer->start(playbackSpeed);
    }
}

void DebuggerPanel::onRuleDoubleClicked(QTreeWidgetItem* item, int column) {
    Q_UNUSED(column);
    if (item) {
        QString ruleName = item->text(0);
        ruleDetailsDisplay->setText(
            "Rule: " + ruleName + "\n\n"
            "FIRST() = { ... }\n"
            "FOLLOW() = { ... }\n\n"
            "Production: non-terminal → symbols"
        );
    }
}
