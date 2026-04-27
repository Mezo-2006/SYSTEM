#ifndef THREE_COLUMN_VIEW_H
#define THREE_COLUMN_VIEW_H

#include <QWidget>
#include <QTextEdit>
#include <QTableWidget>
#include <QSplitter>
#include <QPushButton>
#include "../core/TACGenerator.h"
#include "../core/CodeGenerator.h"

class ThreeColumnView : public QWidget {
    Q_OBJECT

private:
    QTextEdit* sourceColumn;
    QTableWidget* tacTable;
    QTextEdit* assemblyColumn;
    
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
                const std::vector<TACInstruction>& tac,
                const std::vector<AssemblyInstruction>& assembly);
};

#endif // THREE_COLUMN_VIEW_H
