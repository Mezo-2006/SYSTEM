#pragma once
#include <QWidget>
#include <QScrollArea>
#include <vector>
#include <string>
#include <map>
#include "../core/TACGenerator.h"

class LifetimeDiagram : public QWidget {
    Q_OBJECT
public:
    explicit LifetimeDiagram(QWidget* parent = nullptr);
    void setInstructions(const std::vector<TACInstruction>& instructions);
    void reset();

protected:
    void paintEvent(QPaintEvent*) override;
    void mouseMoveEvent(QMouseEvent*) override;
    void leaveEvent(QEvent*) override;

private:
    struct VarLife {
        std::string name;
        int         firstDef = -1;
        int         lastUse  = -1;
        bool        isTemp   = false;
        QColor      color;
    };

    std::vector<VarLife>       vars;
    std::vector<TACInstruction> instrs;
    int hoveredRow = -1;

    void buildLifetimes();
    static QColor colorForVar(const std::string& name, int idx);
};
