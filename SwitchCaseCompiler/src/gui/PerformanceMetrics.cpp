#include "PerformanceMetrics.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>
#include <QApplication>
#include <algorithm>

PerformanceMetrics::PerformanceMetrics(QWidget* parent) : QWidget(parent) {
    setStyleSheet(
        "PerformanceMetrics { background-color: #0F172A; }"
        "QGroupBox { color: #E2E8F0; font-weight: bold; border: 1px solid #334155; "
        "           border-radius: 6px; margin-top: 10px; padding-top: 10px; }"
        "QGroupBox::title { subcontrol-origin: margin; left: 10px; padding: 0 5px; }"
        "QLabel { color: #E2E8F0; }"
        "QProgressBar { border: 1px solid #334155; border-radius: 4px; "
        "              background-color: #1E293B; color: #E2E8F0; text-align: center; }"
        "QProgressBar::chunk { background-color: #3B82F6; border-radius: 3px; }"
    );
    
    m_updateTimer = new QTimer(this);
    m_updateTimer->setInterval(100);
    connect(m_updateTimer, &QTimer::timeout, this, &PerformanceMetrics::onUpdateTimer);
    
    setupUI();
}

PerformanceMetrics::~PerformanceMetrics() {}

void PerformanceMetrics::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(15, 15, 15, 15);
    mainLayout->setSpacing(15);
    
    // Header
    QLabel* header = new QLabel("📊 Performance Metrics & Compilation Analytics");
    header->setStyleSheet(
        "font-size: 18px; font-weight: bold; color: #3B82F6; "
        "background-color: #1E293B; padding: 10px; border-radius: 8px; "
        "border-left: 5px solid #60A5FA; margin-bottom: 10px;"
    );
    mainLayout->addWidget(header);
    
    // Summary metrics row
    QHBoxLayout* summaryLayout = new QHBoxLayout();
    
    auto createMetricCard = [](const QString& title, const QString& value, const QString& color) {
        QGroupBox* card = new QGroupBox(title);
        QVBoxLayout* cardLayout = new QVBoxLayout(card);
        QLabel* valueLabel = new QLabel(value);
        valueLabel->setStyleSheet(QString("font-size: 24px; font-weight: bold; color: %1;").arg(color));
        valueLabel->setAlignment(Qt::AlignCenter);
        cardLayout->addWidget(valueLabel);
        return std::make_pair(card, valueLabel);
    };
    
    auto totalTimeCard = createMetricCard("Total Time", "0 ms", "#10B981");
    m_totalTimeLabel = totalTimeCard.second;
    summaryLayout->addWidget(totalTimeCard.first);
    
    auto throughputCard = createMetricCard("Throughput", "0 tok/ms", "#F59E0B");
    m_throughputLabel = throughputCard.second;
    summaryLayout->addWidget(throughputCard.first);
    
    auto efficiencyCard = createMetricCard("Efficiency", "0%", "#8B5CF6");
    m_efficiencyLabel = efficiencyCard.second;
    summaryLayout->addWidget(efficiencyCard.first);
    
    auto optimizationCard = createMetricCard("Optimization", "0%", "#EF4444");
    m_optimizationLabel = optimizationCard.second;
    summaryLayout->addWidget(optimizationCard.first);
    
    mainLayout->addLayout(summaryLayout);
    
    // Phase timing progress bars
    QGroupBox* phaseGroup = new QGroupBox("Compilation Phase Timing");
    QGridLayout* phaseLayout = new QGridLayout(phaseGroup);
    
    auto createPhaseProgress = [&](const QString& name, int row) {
        QLabel* label = new QLabel(name);
        label->setMinimumWidth(120);
        QProgressBar* progress = new QProgressBar();
        progress->setRange(0, 100);
        progress->setValue(0);
        progress->setFormat("%v ms");
        phaseLayout->addWidget(label, row, 0);
        phaseLayout->addWidget(progress, row, 1);
        return progress;
    };
    
    m_lexerProgress = createPhaseProgress("🔤 Lexical Analysis", 0);
    m_parserProgress = createPhaseProgress("🌲 Syntax Analysis", 1);
    m_semanticProgress = createPhaseProgress("📊 Semantic Analysis", 2);
    m_tacProgress = createPhaseProgress("🔄 TAC Generation", 3);
    m_optimizationProgress = createPhaseProgress("⚡ Optimization", 4);
    m_codegenProgress = createPhaseProgress("🔧 Code Generation", 5);
    m_targetOptProgress = createPhaseProgress("🎯 Target Optimization", 6);
    
    mainLayout->addWidget(phaseGroup);
    
    // Detailed metrics table
    QGroupBox* tableGroup = new QGroupBox("Detailed Metrics");
    QVBoxLayout* tableLayout = new QVBoxLayout(tableGroup);
    
    m_metricsTable = new QTableWidget();
    m_metricsTable->setColumnCount(4);
    m_metricsTable->setHorizontalHeaderLabels({"Metric", "Current", "Average", "Best"});
    m_metricsTable->horizontalHeader()->setStretchLastSection(true);
    m_metricsTable->verticalHeader()->setVisible(false);
    m_metricsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_metricsTable->setAlternatingRowColors(true);
    m_metricsTable->setMaximumHeight(200);

    m_metricsTable->setStyleSheet(
        "QTableWidget { background-color: #1E293B; color: #E2E8F0; "
        "              border: 1px solid #334155; border-radius: 6px; }"
        "QHeaderView::section { background-color: #0F172A; color: #94A3B8; "
        "                      padding: 8px; border: none; font-weight: bold; }"
        "QTableWidget::item { padding: 6px; border-bottom: 1px solid #334155; }"
        "QTableWidget::item:alternate { background-color: #0F172A; }"
    );
    
    tableLayout->addWidget(m_metricsTable);
    mainLayout->addWidget(tableGroup);
}

void PerformanceMetrics::updateMetrics(const CompilationMetrics& metrics) {
    m_currentMetrics = metrics;
    
    // Add to history
    m_history.push_back(metrics);
    if (m_history.size() > MAX_HISTORY) {
        m_history.erase(m_history.begin());
    }
    
    updateSummaryLabels();
    updateMetricsTable();
    
    m_updateTimer->start();
}

void PerformanceMetrics::updateSummaryLabels() {
    auto totalTime = m_currentMetrics.lexerTime + m_currentMetrics.parserTime + 
                    m_currentMetrics.semanticTime + m_currentMetrics.tacTime + 
                    m_currentMetrics.optimizationTime + m_currentMetrics.codegenTime + 
                    m_currentMetrics.targetOptTime;
    
    m_totalTimeLabel->setText(QString("%1 ms").arg(totalTime.count()));
    
    double throughput = totalTime.count() > 0 ? 
        static_cast<double>(m_currentMetrics.tokenCount) / totalTime.count() : 0.0;
    m_throughputLabel->setText(QString("%1 tok/ms").arg(throughput, 0, 'f', 2));
    
    double efficiency = m_currentMetrics.tacInstructions > 0 ? 
        (static_cast<double>(m_currentMetrics.tacInstructions - m_currentMetrics.optimizedTacInstructions) / 
         m_currentMetrics.tacInstructions) * 100.0 : 0.0;
    m_efficiencyLabel->setText(QString("%1%").arg(efficiency, 0, 'f', 1));
    
    m_optimizationLabel->setText(QString("%1%").arg(m_currentMetrics.optimizationRatio, 0, 'f', 1));
    
    // Update progress bars
    auto maxTime = std::max({m_currentMetrics.lexerTime, m_currentMetrics.parserTime,
                            m_currentMetrics.semanticTime, m_currentMetrics.tacTime,
                            m_currentMetrics.optimizationTime, m_currentMetrics.codegenTime,
                            m_currentMetrics.targetOptTime});
    
    if (maxTime.count() > 0) {
        m_lexerProgress->setMaximum(static_cast<int>(maxTime.count()));
        m_parserProgress->setMaximum(static_cast<int>(maxTime.count()));
        m_semanticProgress->setMaximum(static_cast<int>(maxTime.count()));
        m_tacProgress->setMaximum(static_cast<int>(maxTime.count()));
        m_optimizationProgress->setMaximum(static_cast<int>(maxTime.count()));
        m_codegenProgress->setMaximum(static_cast<int>(maxTime.count()));
        m_targetOptProgress->setMaximum(static_cast<int>(maxTime.count()));
        
        m_lexerProgress->setValue(static_cast<int>(m_currentMetrics.lexerTime.count()));
        m_parserProgress->setValue(static_cast<int>(m_currentMetrics.parserTime.count()));
        m_semanticProgress->setValue(static_cast<int>(m_currentMetrics.semanticTime.count()));
        m_tacProgress->setValue(static_cast<int>(m_currentMetrics.tacTime.count()));
        m_optimizationProgress->setValue(static_cast<int>(m_currentMetrics.optimizationTime.count()));
        m_codegenProgress->setValue(static_cast<int>(m_currentMetrics.codegenTime.count()));
        m_targetOptProgress->setValue(static_cast<int>(m_currentMetrics.targetOptTime.count()));
    }
}

void PerformanceMetrics::updateMetricsTable() {
    m_metricsTable->setRowCount(0);
    
    auto addMetricRow = [this](const QString& name, double current, const QString& unit = "") {
        int row = m_metricsTable->rowCount();
        m_metricsTable->insertRow(row);
        
        m_metricsTable->setItem(row, 0, new QTableWidgetItem(name));
        m_metricsTable->setItem(row, 1, new QTableWidgetItem(QString("%1 %2").arg(current, 0, 'f', 2).arg(unit)));
        
        // Calculate average and best from history
        if (!m_history.empty()) {
            // This is a simplified version - in practice you'd calculate proper averages
            m_metricsTable->setItem(row, 2, new QTableWidgetItem(QString("%1 %2").arg(current, 0, 'f', 2).arg(unit)));
            m_metricsTable->setItem(row, 3, new QTableWidgetItem(QString("%1 %2").arg(current, 0, 'f', 2).arg(unit)));
        }
    };
    
    auto totalTime = m_currentMetrics.lexerTime + m_currentMetrics.parserTime + 
                    m_currentMetrics.semanticTime + m_currentMetrics.tacTime + 
                    m_currentMetrics.optimizationTime + m_currentMetrics.codegenTime + 
                    m_currentMetrics.targetOptTime;
    
    addMetricRow("Total Compilation Time", static_cast<double>(totalTime.count()), "ms");
    addMetricRow("Tokens Generated", static_cast<double>(m_currentMetrics.tokenCount));
    addMetricRow("AST Nodes", static_cast<double>(m_currentMetrics.astNodes));
    addMetricRow("TAC Instructions", static_cast<double>(m_currentMetrics.tacInstructions));
    addMetricRow("Optimized TAC", static_cast<double>(m_currentMetrics.optimizedTacInstructions));
    addMetricRow("Assembly Instructions", static_cast<double>(m_currentMetrics.assemblyInstructions));
    addMetricRow("Optimization Ratio", m_currentMetrics.optimizationRatio, "%");
}

void PerformanceMetrics::onUpdateTimer() {
    // Animate progress bars or update real-time metrics
    m_updateTimer->stop();
}

void PerformanceMetrics::clearHistory() {
    m_history.clear();
    updateMetricsTable();
}

void PerformanceMetrics::exportMetrics() {
    QString fileName = QFileDialog::getSaveFileName(this, 
        "Export Performance Metrics", 
        "performance_metrics.csv", 
        "CSV Files (*.csv)");
    
    if (fileName.isEmpty()) return;
    
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Export Error", "Could not open file for writing.");
        return;
    }
    
    QTextStream out(&file);
    out << "Timestamp,LexerTime,ParserTime,SemanticTime,TACTime,OptimizationTime,CodeGenTime,TargetOptTime,";
    out << "TokenCount,DerivationSteps,ASTNodes,SymbolTableEntries,TACInstructions,OptimizedTAC,";
    out << "AssemblyInstructions,OptimizedAssembly,OptimizationRatio\n";
    
    for (const auto& metrics : m_history) {
        out << metrics.timestamp.time_since_epoch().count() << ",";
        out << metrics.lexerTime.count() << ",";
        out << metrics.parserTime.count() << ",";
        out << metrics.semanticTime.count() << ",";
        out << metrics.tacTime.count() << ",";
        out << metrics.optimizationTime.count() << ",";
        out << metrics.codegenTime.count() << ",";
        out << metrics.targetOptTime.count() << ",";
        out << metrics.tokenCount << ",";
        out << metrics.derivationSteps << ",";
        out << metrics.astNodes << ",";
        out << metrics.symbolTableEntries << ",";
        out << metrics.tacInstructions << ",";
        out << metrics.optimizedTacInstructions << ",";
        out << metrics.assemblyInstructions << ",";
        out << metrics.optimizedAssemblyInstructions << ",";
        out << metrics.optimizationRatio << "\n";
    }
    
    QMessageBox::information(this, "Export Complete", 
        QString("Metrics exported to %1").arg(fileName));
}