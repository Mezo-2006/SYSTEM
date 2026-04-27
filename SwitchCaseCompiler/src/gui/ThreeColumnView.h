#ifndef THREE_COLUMN_VIEW_H
#define THREE_COLUMN_VIEW_H

#include <QWidget>
#include <QTextEdit>
#include <QTableWidget>
#include <QSplitter>
#include <QPushButton>
#include <QTabWidget>
#include "../core/TACGenerator.h"
#include "../core/CodeGenerator.h"
#include "IRFlowDiagram.h"

class ThreeColumnView : public QWidget {
    Q_OBJECT

private:
    QTextEdit* sourceColumn;
    QTableWidget* originalTacTable;
    QTableWidget* tacTable;
    QTextEdit* assemblyColumn;
    QTabWidget* tacTabWidget;
    IRFlowDiagram* irFlowDiagram;
    QPushButton* animPlayBtn;
    
    std::vector<TACInstruction> tacInstructions;
    std::vector<AssemblyInstruction> assemblyInstructions;
    
    void setupUI();
    void updateDisplay();

private slots:
    void onExportAsm();
    void onExportReport();

public:
    explicit ThreeColumnView(QWidget* parent = nullptr);

    void setData(const std::string& source,
                const std::vector<TACInstruction>& originalTac,
                const std::vector<TACInstruction>& optimizedTac,
                const std::vector<AssemblyInstruction>& assembly);
};

#endif // THREE_COLUMN_VIEW_H
