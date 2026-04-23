#pragma once
#include <QWidget>
#include <QTableWidget>
#include <QLabel>
#include <QComboBox>
#include <vector>
#include "../core/Optimizer.h"

// Side-by-side diff view for optimization before/after
class OptimizationDiff : public QWidget {
    Q_OBJECT
public:
    explicit OptimizationDiff(QWidget* parent = nullptr);
    void setResults(const std::vector<OptimizationResult>& results);

private slots:
    void onPassSelected(int idx);

private:
    void buildDiff(const OptimizationResult& result);
    void addRow(QTableWidget* tbl, const QString& text, const QColor& bg, const QColor& fg);

    std::vector<OptimizationResult> m_results;
    QComboBox*    passCombo;
    QLabel*       statsLabel;
    QTableWidget* beforeTable;
    QTableWidget* afterTable;
};
