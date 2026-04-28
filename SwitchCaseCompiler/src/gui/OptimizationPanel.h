#ifndef OPTIMIZATION_PANEL_H
#define OPTIMIZATION_PANEL_H

#include <QWidget>
#include <QCheckBox>
#include <QTableWidget>
#include <QPushButton>
#include <QLabel>
#include <QRadioButton>
#include <QComboBox>
#include <QScrollArea>
#include "../core/Optimizer.h"

class OptimizationPanel : public QWidget {
    Q_OBJECT

private:
    // Optimization toggle checkboxes
    QCheckBox* constantFoldingCheckbox;
    QCheckBox* deadCodeCheckbox;
    QCheckBox* cseCheckbox;
    QCheckBox* algebraicCheckbox;
    QCheckBox* copyPropCheckbox;
    QCheckBox* strengthReductionCheckbox;

    // View mode: all passes vs single pass
    QRadioButton* showAllRadio;
    QRadioButton* showPassRadio;
    QComboBox* passSelector;

    // Before/After tables
    QTableWidget* beforeTable;
    QTableWidget* afterTable;

    // Statistics
    QTableWidget* statsTable;
    QLabel* summaryLabel;

    // Impact visualization
    QWidget* impactBar;

    std::vector<OptimizationResult> results;
    
    void setupUI();
    void populateBeforeAfter(const std::vector<TACInstruction>& before,
                             const std::vector<TACInstruction>& after,
                             const std::string& passName);
    void populateAllPasses();
    void populateSinglePass(int index);
    void updateStatsTable();
    void updateImpactBar();

private slots:
    void onViewModeChanged();
    void onPassSelected(int index);

public:
    explicit OptimizationPanel(QWidget* parent = nullptr);

    bool isConstantFoldingEnabled() const;
    bool isDeadCodeEliminationEnabled() const;
    bool isCseEnabled() const;
    bool isAlgebraicSimplificationEnabled() const;
    bool isCopyPropagationEnabled() const;
    bool isStrengthReductionEnabled() const;

    void setOptimizationResults(const std::vector<OptimizationResult>& optimizationResults);
    void displayResults();
};

#endif // OPTIMIZATION_PANEL_H
