#pragma once

#include <QWidget>
#include <QCheckBox>
#include <QTableWidget>
#include <QListWidget>
#include <QPushButton>
#include <QLabel>
#include <vector>
#include "Optimizer.h"

class OptimizationPanel : public QWidget {
    Q_OBJECT

public:
    explicit OptimizationPanel(QWidget* parent = nullptr);

    // Accessors
    bool isConstantFoldingEnabled() const;
    bool isDeadCodeEliminationEnabled() const;
    bool isCseEnabled() const;
    bool isAlgebraicSimplificationEnabled() const;
    bool isCopyPropagationEnabled() const;
    bool isStrengthReductionEnabled() const;

    // Data
    void setOptimizationResults(const std::vector<OptimizationResult>& optimizationResults);

private slots:
    void onShowAllClicked();
    void onPassClicked(QListWidgetItem* item);
    void onStatsCellClicked(int row, int column);

private:
    // UI components
    QCheckBox* constantFoldingCheckbox;
    QCheckBox* deadCodeCheckbox;
    QCheckBox* cseCheckbox;
    QCheckBox* algebraicCheckbox;
    QCheckBox* copyPropCheckbox;
    QCheckBox* strengthReductionCheckbox;

    // New sidebar for passes
    QListWidget* passList;               // clickable cards for each pass
    QPushButton* showAllButton;          // show full before/after view

    QLabel* summaryLabel;
    QTableWidget* statsTable;
    QWidget* impactBar;
    QTableWidget* beforeTable;
    QTableWidget* afterTable;

    // Data
    std::vector<OptimizationResult> results;

    // Helpers
    void setupUI();
    void displayResults();
    void updateStatsTable();
    void updateImpactBar();
    void populateAllPasses();
    void populateSinglePass(int index);
    void populateBeforeAfter(const std::vector<TACInstruction>& before,
                             const std::vector<TACInstruction>& after,
                             const std::string& passName);
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
