#ifndef OPTIMIZATION_PANEL_H
#define OPTIMIZATION_PANEL_H

#include <QWidget>
#include <QCheckBox>
#include <QTableWidget>
#include <QPushButton>
#include "../core/Optimizer.h"

class OptimizationPanel : public QWidget {
    Q_OBJECT

private:
    QCheckBox* constantFoldingCheckbox;
    QCheckBox* deadCodeCheckbox;
    QCheckBox* cseCheckbox;
    QCheckBox* algebraicCheckbox;
    QCheckBox* copyPropCheckbox;
    QCheckBox* strengthReductionCheckbox;

    QTableWidget* beforeTable;
    QTableWidget* afterTable;
    
    std::vector<OptimizationResult> results;
    
    void setupUI();

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
