#ifndef PERFORMANCE_METRICS_H
#define PERFORMANCE_METRICS_H

#include <QWidget>
#include <QLabel>
#include <QProgressBar>
#include <QTableWidget>
#include <QTimer>
#include <vector>
#include <chrono>

struct CompilationMetrics {
    std::chrono::milliseconds lexerTime{0};
    std::chrono::milliseconds parserTime{0};
    std::chrono::milliseconds semanticTime{0};
    std::chrono::milliseconds tacTime{0};
    std::chrono::milliseconds optimizationTime{0};
    std::chrono::milliseconds codegenTime{0};
    std::chrono::milliseconds targetOptTime{0};
    
    int tokenCount = 0;
    int derivationSteps = 0;
    int astNodes = 0;
    int symbolTableEntries = 0;
    int tacInstructions = 0;
    int optimizedTacInstructions = 0;
    int assemblyInstructions = 0;
    int optimizedAssemblyInstructions = 0;
    
    int lexicalErrors = 0;
    int syntaxErrors = 0;
    int semanticErrors = 0;
    
    double optimizationRatio = 0.0;
    double compressionRatio = 0.0;
    
    std::chrono::system_clock::time_point timestamp;
};

class PerformanceMetrics : public QWidget {
    Q_OBJECT

public:
    explicit PerformanceMetrics(QWidget* parent = nullptr);
    ~PerformanceMetrics() override;

    void updateMetrics(const CompilationMetrics& metrics);
    void clearHistory();
    void exportMetrics();

private slots:
    void onUpdateTimer();

private:
    void setupUI();
    void updateMetricsTable();
    void updateSummaryLabels();
    
    // UI Components
    QLabel* m_totalTimeLabel;
    QLabel* m_throughputLabel;
    QLabel* m_efficiencyLabel;
    QLabel* m_optimizationLabel;
    
    QProgressBar* m_lexerProgress;
    QProgressBar* m_parserProgress;
    QProgressBar* m_semanticProgress;
    QProgressBar* m_tacProgress;
    QProgressBar* m_optimizationProgress;
    QProgressBar* m_codegenProgress;
    QProgressBar* m_targetOptProgress;
    
    QTableWidget* m_metricsTable;
    
    // Data
    std::vector<CompilationMetrics> m_history;
    CompilationMetrics m_currentMetrics;
    
    QTimer* m_updateTimer;
    
    static constexpr int MAX_HISTORY = 50;
};

#endif // PERFORMANCE_METRICS_H